#include <framework.h>
#include "Video.h"

#include <vlc/vlc.h>
#include <iostream>
#include "Renderer/Renderer.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/winmain.h"

using namespace TEN::Input;

namespace TEN::Video
{
	VideoHandler g_VideoPlayer = {};

	const std::string VIDEO_PATH = "FMV/";
	const std::vector<std::string> VIDEO_EXTENSIONS = { ".mp4", ".avi", ".mkv", ".mov" };

	void LogCallback(void* userdata, int level, const libvlc_log_t* ctx, const char* fmt, va_list args)
	{
		LogLevel logLevel = LogLevel::Info;

		switch (level)
		{
		case LIBVLC_ERROR:
			logLevel = LogLevel::Error;
			break;
		case LIBVLC_WARNING:
			logLevel = LogLevel::Warning;
			break;
		case LIBVLC_NOTICE:
			logLevel = LogLevel::Info;
			break;
		case LIBVLC_DEBUG:
			logLevel = LogLevel::Info;
			break;
		default:
			logLevel = LogLevel::Info;
			break;
		}

		char logMessage[1024];
		vsnprintf(logMessage, sizeof(logMessage), fmt, args);

		TENLog(std::string(logMessage), logLevel);
	}

	VideoHandler::~VideoHandler()
	{
		DeInitPlayer();

		if (vlcInstance)
			libvlc_release(vlcInstance);

		DeInitD3DTexture();
	}

	void VideoHandler::Initialize(const std::string& gameDir, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		TENLog("Initializing video player...", LogLevel::Info);

		// Disable video output and title, because we are rendering to D3D texture.
		const char* args[] = { "--vout=none", "--no-video-title" };
		vlcInstance = libvlc_new(2, args);

#ifdef _DEBUG
		// libvlc_log_set(vlcInstance, LogCallback, nullptr);
#endif

		HandleError();

		videoDirectory = gameDir + VIDEO_PATH;
		d3dDevice = device;
		d3dContext = context;
	}

	bool VideoHandler::Play(const std::string& filename)
	{
		auto fullVideoName = videoDirectory + filename;

		// At first, attempt to load video file with full filename.
		// Then, if not found, try all common video file extensions, and only quit if none are found.
		if (!std::filesystem::is_regular_file(fullVideoName))
		{
			for (const auto& ext : VIDEO_EXTENSIONS)
			{
				if (std::filesystem::is_regular_file(fullVideoName + ext))
				{
					fullVideoName += ext;
					break;
				}
			}

			if (!std::filesystem::is_regular_file(fullVideoName))
			{
				TENLog("Video file not found: " + fullVideoName, LogLevel::Warning);
				return false;
			}
		}

		// Delete previous player instance.
		DeInitPlayer();

		currentFilename = fullVideoName;

		auto* media = libvlc_media_new_path(currentFilename.c_str());
		if (media == nullptr)
		{
			TENLog("Failed to create media from path: " + currentFilename, LogLevel::Error);
			return false;
		}

		// VLC requires to initialize media, load it into player and release it right away.
		player = libvlc_media_player_new_from_media(vlcInstance, media);
		libvlc_media_release(media);

		if (player == nullptr)
		{
			TENLog("Failed to create media player", LogLevel::Error);
			return false;
		}

		if (!InitD3DTexture())
			return false;

		libvlc_video_set_callbacks(player, LockFrame, UnlockFrame, DisplayFrame, this);
		libvlc_media_player_play(player);
		SetVolume(volume);

		if (!HandleError())
			return false;

		PauseAllSounds(SoundPauseMode::Global);
		return true;
	}

	void VideoHandler::Pause()
	{
		if (player == nullptr)
			return;

		if (libvlc_media_player_get_state(player) != libvlc_Paused)
			libvlc_media_player_pause(player);

		HandleError();
	}

	void VideoHandler::Resume()
	{
		if (player == nullptr)
			return;

		if (libvlc_media_player_get_state(player) == libvlc_Paused)
			libvlc_media_player_play(player);

		HandleError();
	}

	void VideoHandler::Stop()
	{
		DeInitPlayer();
		DeInitD3DTexture();
		needRender = false;

		HandleError();
	}

	bool VideoHandler::Sync()
	{
		if (player == nullptr)
			return false;

		bool renderResult = needRender;
		needRender = false;

		return renderResult;
	}

	bool VideoHandler::Update()
	{
		if (player == nullptr)
			return false;

		// Because VLC plays media asynchronously with internal clock, we may update game loop 
		// as quickly as possible.
		App.ResetClock = true;
		UpdateInputActions(true);

		bool interruptPlayback = IsHeld(In::Deselect) || IsHeld(In::Look);
		auto state = libvlc_media_player_get_state(player);

		if (!interruptPlayback && state == libvlc_Playing)
		{
			if (g_VideoPlayer.Sync())
			{
				unsigned int videoWidth, videoHeight;
				libvlc_video_get_size(player, 0, &videoWidth, &videoHeight);
				g_Renderer.RenderFullScreenTexture(textureView, (float)videoWidth / videoHeight);
			}
		}

		if (interruptPlayback || state == libvlc_Stopping || state == libvlc_Error || state == libvlc_Stopped)
		{
			Stop();
			ClearAction(In::Pause); // HACK: Otherwise pause key won't work after video ends.
			ResumeAllSounds(SoundPauseMode::Global);
		}

		HandleError();

		return (state == libvlc_Playing);
	}

	void VideoHandler::SetVolume(int volume)
	{
		// Set volume even if player is not available, because volume may be externally changed from settings.
		this->volume = std::clamp(volume, 0, 100);

		if (player != nullptr)
			libvlc_audio_set_volume(player, this->volume);

		HandleError();
	}

	bool VideoHandler::HandleError()
	{
		if (vlcInstance == nullptr)
			return false;

		const char* vlcMessage = libvlc_errmsg();
		if (vlcMessage && strlen(vlcMessage) > 0)
		{
			TENLog("Video player error: " + std::string(vlcMessage), LogLevel::Error);
			return false;
		}

		return true;
	}

	bool VideoHandler::InitD3DTexture()
	{
		if (videoTexture || textureView)
		{
			TENLog("Video texture already exists", LogLevel::Error);
			return false;
		}

		auto resolution = g_Renderer.GetScreenResolution();
		
		// Limit maximum resolution, because some GPUs may not handle textures larger than 2048x2048.
		videoWidth = std::clamp(resolution.x, MIN_VIDEO_WIDTH, MAX_VIDEO_WIDTH);
		videoHeight = std::clamp(resolution.y, MIN_VIDEO_HEIGHT, MAX_VIDEO_HEIGHT);

		libvlc_video_set_format(player, "BGRA", videoWidth, videoHeight, videoWidth * 4);

		frameBuffer = new unsigned char[videoWidth * videoHeight * 4];
		if (!frameBuffer)
		{
			TENLog("Failed to allocate frame buffer", LogLevel::Error);
			return false;
		}

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = videoWidth;
		texDesc.Height = videoHeight;
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

	void VideoHandler::DeInitD3DTexture()
	{
		if (frameBuffer)
			delete[] frameBuffer;

		if (videoTexture)
		{
			videoTexture->Release();
			videoTexture = nullptr;
		}

		if (textureView)
		{
			textureView->Release();
			textureView = nullptr;
		}
	}

	void VideoHandler::DeInitPlayer()
	{
		if (player == nullptr)
			return;

		libvlc_media_player_stop_async(player);
		libvlc_media_player_release(player);
		player = nullptr;

		HandleError();
	}

	void* VideoHandler::LockFrame(void* data, void** pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(data);
		*pixels = player->frameBuffer;
		return nullptr;
	}

	void VideoHandler::UnlockFrame(void* data, void* picture, void* const* pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(data);

		if (!player->videoTexture)
		{
			TENLog("Video texture does not exist", LogLevel::Error);
			return;
		}

		// In case of consequential unlock frame callbacks, do not attempt to map texture again.
		if (player->needRender)
			return;

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

	void VideoHandler::DisplayFrame(void* data, void* picture)
	{
		// Empty event.
	}
}