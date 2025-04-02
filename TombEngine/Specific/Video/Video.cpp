#include <framework.h>
#include "Video.h"

#include <vlc/vlc.h>
#include <iostream>
#include "Sound/sound.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;

namespace TEN::Video
{
	VideoHandler* g_VideoPlayer = nullptr;

	void LogCallback(void* userdata, int level, const libvlc_log_t* ctx, const char* fmt, va_list args)
	{
		// This is where the log messages are captured and output to your console
		std::string levelStr;
		LogLevel logLevel = LogLevel::Info;

		switch (level)
		{
		case LIBVLC_ERROR:
			levelStr = "ERROR";
			logLevel = LogLevel::Error;
			break;
		case LIBVLC_WARNING:
			levelStr = "WARNING";
			logLevel = LogLevel::Warning;
			break;
		case LIBVLC_NOTICE:
			levelStr = "INFO";
			logLevel = LogLevel::Info;
			break;
		case LIBVLC_DEBUG:
			levelStr = "DEBUG";
			logLevel = LogLevel::Info;
			break;
		default:
			levelStr = "UNKNOWN";
			logLevel = LogLevel::Info;
			break;
		}

		char logMessage[1024];
		vsnprintf(logMessage, sizeof(logMessage), fmt, args);

		TENLog("[" + levelStr + "] " + std::string(logMessage), logLevel);
	}

	VideoHandler::VideoHandler(ID3D11Device* device, ID3D11DeviceContext* context) : 
		vlcInstance(nullptr), player(nullptr), frameBuffer(nullptr),
		d3dDevice(device), d3dContext(context), videoTexture(nullptr), textureView(nullptr), videoWidth(VIDEO_WIDTH), videoHeight(VIDEO_HEIGHT),
		needRender(false), volume(100)
	{
		TENLog("Initializing video player...", LogLevel::Info);

		SetEnvironmentVariable("VLC_PLUGIN_PATH", "./vlc/plugins"); // TODO

		// Set VLC arguments, including the audio output module
		const char* args[] = { "--verbose", "--vout=none", "--no-video-title" }; // Example arguments
		vlcInstance = libvlc_new(3, args);  // Use the appropriate arguments

		// DEBUG: Uncomment to do a verbose logging.
		// libvlc_log_set(vlcInstance, LogCallback, nullptr);
		
		HandleError();
	}

	VideoHandler::~VideoHandler()
	{
		if (player)
		{
			libvlc_media_player_stop_async(player);
			libvlc_media_player_release(player);
		}

		if (vlcInstance)
			libvlc_release(vlcInstance);

		if (frameBuffer)
			delete[] frameBuffer;

		if (videoTexture)
			videoTexture->Release();

		if (textureView)
			textureView->Release();
	}

	bool VideoHandler::Play(const std::string& filename)
	{
		if (!std::filesystem::is_regular_file(filename))
		{
			TENLog("Video file not found: " + filename, LogLevel::Warning);
			return false;
		}


		if (player)
		{
			libvlc_media_player_stop_async(player);
			libvlc_media_player_release(player);
			player = nullptr;
		}

		currentFilename = filename;

		libvlc_media_t* media = libvlc_media_new_path(filename.c_str());
		if (!media)
		{
			TENLog("Failed to create media from path: " + filename, LogLevel::Error);
			return false;
		}

		player = libvlc_media_player_new_from_media(vlcInstance, media);
		libvlc_media_release(media);

		if (!player)
		{
			TENLog("Failed to create media player", LogLevel::Error);
			return false;
		}

		if (!InitD3DTexture())
			return false;

		libvlc_video_set_callbacks(player, LockFrame, UnlockFrame, DisplayFrame, this);
		libvlc_media_player_play(player);
		SetVolume(volume);

		if (HandleError())
			PauseAllSounds(SoundPauseMode::Global);

		return true;
	}

	bool VideoHandler::InitD3DTexture()
	{
		if (videoTexture)
			videoTexture->Release();

		if (textureView)
			textureView->Release();

		unsigned int width = 0, height = 0;
		int result = libvlc_video_get_size(player, 0, &width, &height);

		if (width == 0 || height == 0)
		{
			TENLog("Failed to retrieve video dimensions, using default (" + std::to_string(VIDEO_WIDTH) + "x" + std::to_string(VIDEO_HEIGHT) + ")", LogLevel::Error);
			width = VIDEO_WIDTH;
			height = VIDEO_HEIGHT;
		}

		videoWidth = width;
		videoHeight = height;

		libvlc_video_set_format(player, "BGRA", videoWidth, videoHeight, videoWidth * 4);

		if (frameBuffer)
		{
			delete[] frameBuffer;
			frameBuffer = nullptr;
		}

		frameBuffer = new unsigned char[videoWidth * videoHeight * 4];
		if (!frameBuffer)
		{
			TENLog("Failed to allocate frame buffer", LogLevel::Error);
			return false;
		}

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texDesc.MiscFlags = 0;

		if (FAILED(d3dDevice->CreateTexture2D(&texDesc, nullptr, &videoTexture)))
		{
			TENLog("Failed to create video texture", LogLevel::Error);
			return false;
		}

		if (videoTexture != nullptr && FAILED(d3dDevice->CreateShaderResourceView(videoTexture, nullptr, &textureView)))
		{
			TENLog("Failed to create shader resource view", LogLevel::Error);
			return false;
		}

		return true;
	}

	void VideoHandler::Pause()
	{
		if (!player)
			return;

		if (libvlc_media_player_get_state(player) != libvlc_Paused)
			libvlc_media_player_pause(player);

		HandleError();
	}

	void VideoHandler::Resume()
	{
		if (!player)
			return;

		if (libvlc_media_player_get_state(player) == libvlc_Paused)
			libvlc_media_player_play(player);

		HandleError();
	}


	void VideoHandler::Stop()
	{
		if (player)
		{
			libvlc_media_player_stop_async(player);
			libvlc_media_player_release(player);
			player = nullptr;
			needRender = false;
		}

		HandleError();
	}

	bool VideoHandler::Sync()
	{
		if (!player)
			return false;

		bool renderResult = needRender;
		needRender = false;

		return renderResult;
	}

	bool VideoHandler::Update()
	{
		if (player == nullptr)
			return false;

		UpdateInputActions(true);

		if (IsHeld(In::Deselect) || IsHeld(In::Look))
			Stop();

		auto state = (player == nullptr) ? libvlc_Stopped : libvlc_media_player_get_state(player);
		if (state == libvlc_Stopping || state == libvlc_Error || state == libvlc_Stopped)
			ResumeAllSounds(SoundPauseMode::Global);

		HandleError();

		return (state == libvlc_Playing);
	}

	float VideoHandler::GetPosition()
	{
		float pos = player ? libvlc_media_player_get_position(player) : 0.0f;
		HandleError();
		return pos;
	}

	void VideoHandler::SetPosition(float position)
	{
		if (player)
			libvlc_media_player_set_position(player, position, true);

		HandleError();
	}

	void VideoHandler::SetVolume(int volume)
	{
		this->volume = std::clamp(volume, 0, 100);

		if (player)
			libvlc_audio_set_volume(player, this->volume);

		HandleError();
	}

	ID3D11ShaderResourceView* VideoHandler::GetTextureView() const
	{
		return textureView;
	}

	bool VideoHandler::HandleError()
	{
		const char* vlcMessage = libvlc_errmsg();
		if (vlcMessage && strlen(vlcMessage) > 0)
		{
			TENLog("Failed to initialize VLC instance: " + std::string(vlcMessage), LogLevel::Error);
			return false;
		}

		return true;
	}

	void* VideoHandler::LockFrame(void* opaque, void** pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(opaque);
		*pixels = player->frameBuffer;
		return nullptr;
	}

	void VideoHandler::UnlockFrame(void* opaque, void* picture, void* const* pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(opaque);

		if (!player->videoTexture)
		{
			TENLog("Video texture does not exist", LogLevel::Error);
			return;
		}

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		if (SUCCEEDED(player->d3dContext->Map(player->videoTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			memcpy(mappedResource.pData, player->frameBuffer, player->videoWidth * player->videoHeight * 4);
			player->d3dContext->Unmap(player->videoTexture, 0);
			player->needRender = true;
		}
		else
		{
			TENLog("Failed to map video texture", LogLevel::Error);
		}
	}

	void VideoHandler::DisplayFrame(void* opaque, void* picture)
	{
	}
}