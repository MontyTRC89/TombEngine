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

		if (_vlcInstance)
			libvlc_release(_vlcInstance);

		DeInitD3DTexture();
	}

	void VideoHandler::Initialize(const std::string& gameDir, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		TENLog("Initializing video player...", LogLevel::Info);

		// Disable video output and title, because we are rendering to D3D texture.
		const char* args[] = { "--vout=none", "--no-video-title" };
		_vlcInstance = libvlc_new(2, args);

#ifdef _DEBUG
		// libvlc_log_set(_vlcInstance, LogCallback, nullptr);
#endif

		HandleError();

		_videoDirectory = gameDir + VIDEO_PATH;
		_d3dDevice = device;
		_d3dContext = context;
	}

	bool VideoHandler::Play(const std::string& filename)
	{
		auto fullVideoName = _videoDirectory + filename;

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

		_currentFilename = fullVideoName;

		auto* media = libvlc_media_new_path(_currentFilename.c_str());
		if (media == nullptr)
		{
			TENLog("Failed to create media from path: " + _currentFilename, LogLevel::Error);
			return false;
		}

		// VLC requires to initialize media, load it into player and release it right away.
		_player = libvlc_media_player_new_from_media(_vlcInstance, media);
		libvlc_media_release(media);

		if (_player == nullptr)
		{
			TENLog("Failed to create media player", LogLevel::Error);
			return false;
		}

		if (!InitD3DTexture())
			return false;

		libvlc_video_set_callbacks(_player, LockFrame, UnlockFrame, DisplayFrame, this);
		libvlc_media_player_play(_player);
		SetVolume(_volume);

		if (!HandleError())
			return false;

		PauseAllSounds(SoundPauseMode::Global);
		return true;
	}

	bool VideoHandler::Pause()
	{
		if (_player == nullptr)
			return false;

		if (libvlc_media_player_get_state(_player) != libvlc_Paused)
		{
			libvlc_media_player_pause(_player);
			HandleError();
			return true;
		}

		HandleError();
		return false;
	}

	bool VideoHandler::Resume()
	{
		if (_player == nullptr)
			return false;

		if (libvlc_media_player_get_state(_player) == libvlc_Paused)
		{
			libvlc_media_player_play(_player);
			HandleError();
			return true;
		}

		HandleError();
		return false;
	}

	void VideoHandler::Stop()
	{
		DeInitPlayer();
		DeInitD3DTexture();
		_needRender = false;

		HandleError();
	}

	bool VideoHandler::Sync()
	{
		if (_player == nullptr)
			return false;

		bool renderResult = _needRender;
		_needRender = false;

		return renderResult;
	}

	bool VideoHandler::Update()
	{
		if (_player == nullptr)
			return false;

		// Because VLC plays media asynchronously with internal clock, we may update game loop 
		// as quickly as possible.
		App.ResetClock = true;
		UpdateInputActions(true);

		bool interruptPlayback = IsHeld(In::Deselect) || IsHeld(In::Look);
		auto state = libvlc_media_player_get_state(_player);

		if (!interruptPlayback && state == libvlc_Playing)
		{
			if (g_VideoPlayer.Sync())
			{
				unsigned int videoWidth, videoHeight;
				libvlc_video_get_size(_player, 0, &videoWidth, &videoHeight);
				g_Renderer.RenderFullScreenTexture(_textureView, (float)videoWidth / videoHeight);
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
		this->_volume = std::clamp(volume, 0, 100);

		if (_player != nullptr)
			libvlc_audio_set_volume(_player, this->_volume);

		HandleError();
	}

	bool VideoHandler::HandleError()
	{
		if (_vlcInstance == nullptr)
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
		if (_videoTexture || _textureView)
		{
			TENLog("Video texture already exists", LogLevel::Error);
			return false;
		}

		auto resolution = g_Renderer.GetScreenResolution();
		
		// Limit maximum resolution, because some GPUs may not handle textures larger than 2048x2048.
		_videoWidth = std::clamp(resolution.x, MIN_VIDEO_WIDTH, MAX_VIDEO_WIDTH);
		_videoHeight = std::clamp(resolution.y, MIN_VIDEO_HEIGHT, MAX_VIDEO_HEIGHT);

		libvlc_video_set_format(_player, "BGRA", _videoWidth, _videoHeight, _videoWidth * 4);

		_frameBuffer = new unsigned char[_videoWidth * _videoHeight * 4];
		if (!_frameBuffer)
		{
			TENLog("Failed to allocate frame buffer", LogLevel::Error);
			return false;
		}

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = _videoWidth;
		texDesc.Height = _videoHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texDesc.MiscFlags = 0;

		if (FAILED(_d3dDevice->CreateTexture2D(&texDesc, nullptr, &_videoTexture)))
		{
			TENLog("Failed to create video texture", LogLevel::Error);
			return false;
		}

		if (_videoTexture != nullptr && FAILED(_d3dDevice->CreateShaderResourceView(_videoTexture, nullptr, &_textureView)))
		{
			TENLog("Failed to create shader resource view", LogLevel::Error);
			return false;
		}

		return true;
	}

	void VideoHandler::DeInitD3DTexture()
	{
		if (_frameBuffer)
			delete[] _frameBuffer;

		if (_videoTexture)
		{
			_videoTexture->Release();
			_videoTexture = nullptr;
		}

		if (_textureView)
		{
			_textureView->Release();
			_textureView = nullptr;
		}
	}

	void VideoHandler::DeInitPlayer()
	{
		if (_player == nullptr)
			return;

		libvlc_media_player_stop_async(_player);
		libvlc_media_player_release(_player);
		_player = nullptr;

		HandleError();
	}

	void* VideoHandler::LockFrame(void* data, void** pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(data);
		*pixels = player->_frameBuffer;
		return nullptr;
	}

	void VideoHandler::UnlockFrame(void* data, void* picture, void* const* pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(data);

		if (!player->_videoTexture)
		{
			TENLog("Video texture does not exist", LogLevel::Error);
			return;
		}

		// In case of consequential unlock frame callbacks, do not attempt to map texture again.
		if (player->_needRender)
			return;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		if (SUCCEEDED(player->_d3dContext->Map(player->_videoTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			memcpy(mappedResource.pData, player->_frameBuffer, player->_videoWidth * player->_videoHeight * 4);
			player->_d3dContext->Unmap(player->_videoTexture, 0);
			player->_needRender = true;
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