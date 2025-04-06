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

	VideoHandler::~VideoHandler()
	{
		Stop();

		if (_vlcInstance)
			libvlc_release(_vlcInstance);
	}

	void VideoHandler::Initialize(const std::string& gameDir, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		TENLog("Initializing video player...", LogLevel::Info);

		// Disable video output and title, because we are rendering to D3D texture.
#ifdef _DEBUG
		const char* args[] = { "--vout=none", "--no-video-title", "--no-media-library"};
		_vlcInstance = libvlc_new(3, args);
		//libvlc_log_set(_vlcInstance, OnLog, nullptr);
#else
		const char* args[] = { "--vout=none", "--no-video-title", "--no-media-library", "--quiet" };
		_vlcInstance = libvlc_new(4, args);
#endif

		HandleError();

		_d3dDevice = device;
		_d3dContext = context;

		_videoDirectory = gameDir + VIDEO_PATH;
		_size = Vector2i::Zero;
		_fileName = {};
		_playbackMode = VideoPlaybackMode::Exclusive;
		_looped = false;
		_silent = false;
	}

	bool VideoHandler::Play(const std::string& filename, VideoPlaybackMode mode, bool silent, bool loop)
	{
		auto fullVideoName = filename;

		// At first, attempt to load video file with original filename. Then proceed with asset directory.
		// Then, if not found, try all common video file extensions, and only quit if none are found.
		if (!std::filesystem::is_regular_file(fullVideoName))
		{
			fullVideoName = _videoDirectory + filename;

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
		}

		// Don't start playback if the same video is already playing.
		if (_player != nullptr)
		{
			if (libvlc_media_player_get_state(_player) == libvlc_Playing &&
				_playbackMode == mode && _fileName == fullVideoName)
			{
				TENLog("Video file " + fullVideoName + " is already playing.", LogLevel::Warning);
				return false;
			}
		}

		// Stop previous player instance, if it exists.
		Stop();

		_looped = loop;
		_silent = silent;
		_playbackMode = mode;
		_fileName = fullVideoName;

		auto* media = libvlc_media_new_path(_fileName.c_str());
		if (media == nullptr)
		{
			TENLog("Failed to create media from path: " + _fileName, LogLevel::Error);
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

		libvlc_video_set_callbacks(_player, OnLockFrame, OnUnlockFrame, nullptr, this);
		libvlc_video_set_format_callbacks(_player, OnSetup, nullptr);
		libvlc_media_player_play(_player);

		SetVolume(_volume);

		if (!HandleError())
			return false;

		auto filePath = std::filesystem::path(_fileName);
		TENLog("Playing video file: " + filePath.filename().string() + " (" + (mode == VideoPlaybackMode::Exclusive ? "Exclusive" : "Background") + " mode)", LogLevel::Info);

		if (_playbackMode == VideoPlaybackMode::Exclusive)
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

			return (_playbackMode == VideoPlaybackMode::Exclusive);
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

			return (_playbackMode == VideoPlaybackMode::Exclusive);
		}

		HandleError();
		return false;
	}

	void VideoHandler::Stop()
	{
		DeInitPlayer();
		DeInitD3DTexture();

		// Don't unset this flag until D3D texture is released, otherwise it may cause a crash
		// when trying to render the texture.
		_needRender = false;

		HandleError();
	}

	void VideoHandler::UpdateBackground()
	{
		if (_player == nullptr)
			return;

		auto state = libvlc_media_player_get_state(_player);

		// If video has finished playback, stop and delete it.
		if (!_looped && (state == libvlc_Error || state == libvlc_Stopped))
		{
			Stop();
			return;
		}

		// Reset playback to the start, if video is looped.
		if (_looped && (state == libvlc_Stopping || state == libvlc_Stopped))
			libvlc_media_player_play(_player);

		HandleError();
	}

	void VideoHandler::RenderBackground()
	{
		_texture.Width = _size.x;
		_texture.Height = _size.y;
		_texture.Texture = _videoTexture;
		_texture.ShaderResourceView = _textureView;

		g_Renderer.UpdateVideoTexture(&_texture);
	}

	void VideoHandler::UpdateExclusive()
	{
		if (_player == nullptr)
			return;

		// Because VLC plays media asynchronously with internal clock, we may update game loop 
		// as quickly as possible.
		App.ResetClock = true;
		UpdateInputActions(true);

		bool interruptPlayback = IsHeld(In::Deselect) || IsHeld(In::Look);
		auto state = libvlc_media_player_get_state(_player);

		// If player is just opening, buffering or stopping, always return early and wait for the process to end.
		if (state == libvlc_Opening || state == libvlc_Buffering)
			return;

		// Reset playback to the start, if video is looped.
		if (_looped && !interruptPlayback && (state == libvlc_Stopping || state == libvlc_Stopped))
			libvlc_media_player_play(_player);

		// If user pressed a key to break out from the video, or video has finished playback or in an error, stop and delete it.
		if (interruptPlayback || state == libvlc_Error || state == libvlc_Stopped)
		{
			Stop();
			ClearAction(In::Pause); // HACK: Otherwise pause key won't work after video ends.
			ResumeAllSounds(SoundPauseMode::Global);
		}

		HandleError();
	}

	void VideoHandler::RenderExclusive()
	{
		g_Renderer.RenderFullScreenTexture(_textureView, (float)_size.x / (float)_size.y);
	}

	bool VideoHandler::Update()
	{
		if (_player == nullptr)
			return false;

		auto state = libvlc_media_player_get_state(_player);

		// Attempt to map and render texture only if callback has set the frame to be rendered.
		if (_needRender)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (_videoTexture && SUCCEEDED(_d3dContext->Map(_videoTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				// Copy framebuffer row by row, otherwise skewing may occur.
				unsigned char* pData = reinterpret_cast<unsigned char*>(mappedResource.pData);
				for (int row = 0; row < _size.y; row++)
					memcpy(pData + row * mappedResource.RowPitch, _frameBuffer.data() + row * _size.x * 4, _size.x * 4);
				_d3dContext->Unmap(_videoTexture, 0);

				if (_playbackMode == VideoPlaybackMode::Exclusive)
					RenderExclusive();
				else if (_playbackMode == VideoPlaybackMode::Background)
					RenderBackground();
			}
			else
			{
				TENLog("Failed to render video texture", LogLevel::Error);
			}

			_needRender = false;
		}

		if (_playbackMode == VideoPlaybackMode::Exclusive)
			UpdateExclusive();
		else if (_playbackMode == VideoPlaybackMode::Background)
			UpdateBackground();

		return (_playbackMode == VideoPlaybackMode::Exclusive);
	}

	float VideoHandler::GetNormalizedPosition() const
	{
		if (_player == nullptr)
			return 0.0f;

		return (float)libvlc_media_player_get_position(_player);
	}

	int VideoHandler::GetPosition() const
	{
		if (_player == nullptr)
			return 0;

		libvlc_media_t* media = libvlc_media_player_get_media(_player);
		long long duration = libvlc_media_get_duration(media);
		float posNormalized = GetNormalizedPosition();

		// Convert and return percentage in frames.
		long long posMsec = posNormalized * duration;
		return (int)((posMsec / 1000.0f) * FPS);
	}

	void VideoHandler::SetNormalizedPosition(float position)
	{
		if (_player == nullptr)
			return;

		libvlc_media_player_set_position(_player, std::clamp(position, 0.0f, 1.0f), false);
		HandleError();
	}

	void VideoHandler::SetPosition(int frameCount)
	{
		if (_player == nullptr)
			return;

		libvlc_media_t* media = libvlc_media_player_get_media(_player);
		long long duration = libvlc_media_get_duration(media);

		// Convert frames to time in milliseconds (assuming 30 FPS).
		float posMsec = (frameCount / FPS) * 1000.0f;
		float posNormalized = posMsec / (float)(duration);

		if (posNormalized > 1.0f)
		{
			TENLog("Video position is out of bounds.", LogLevel::Warning);
			return;
		}

		// Set position in percentage.
		SetNormalizedPosition(posNormalized);
		HandleError();
	}

	std::string VideoHandler::GetFileName() const
	{
		if (_player == nullptr)
			return std::string();

		return _fileName;
	}

	bool VideoHandler::GetSilent() const
	{
		return _silent;
	}

	bool VideoHandler::GetLooped() const
	{
		return _looped;
	}

	bool VideoHandler::IsPlaying() const
	{
		if (_player == nullptr)
			return false;

		auto state = libvlc_media_player_get_state(_player);
		return (state == libvlc_Playing);
	}

	void VideoHandler::SetVolume(int volume)
	{
		// Set volume even if player is not available, because volume may be externally changed from settings.
		this->_volume = std::clamp(volume, 0, 100);

		if (_player != nullptr)
			libvlc_audio_set_volume(_player, _silent ? 0.0f : this->_volume);

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
		if (_videoTexture != nullptr || _textureView != nullptr)
		{
			TENLog("Video texture already exists", LogLevel::Error);
			return false;
		}

		_frameBuffer.resize(_size.x * _size.y * 4);

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = _size.x;
		texDesc.Height = _size.y;
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
		if (_videoTexture != nullptr)
		{
			_videoTexture->Release();
			_videoTexture = nullptr;
		}

		if (_textureView != nullptr)
		{
			_textureView->Release();
			_textureView = nullptr;
		}

		_texture = {};
		_frameBuffer.clear();

		_size = Vector2i::Zero;
	}

	void VideoHandler::DeInitPlayer()
	{
		if (_player == nullptr)
			return;

		libvlc_media_player_stop_async(_player);
		libvlc_media_player_release(_player);

		_player = nullptr;
		_fileName = {};

		HandleError();
	}

	void* VideoHandler::OnLockFrame(void* data, void** pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(data);
		*pixels = player->_frameBuffer.data();
		return nullptr;
	}

	void VideoHandler::OnUnlockFrame(void* data, void* picture, void* const* pixels)
	{
		VideoHandler* player = static_cast<VideoHandler*>(data);
		player->_needRender = true;
	}

	unsigned int VideoHandler::OnSetup(void** data, char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines)
	{
		strncpy(chroma, "BGRA", 4);

		*pitches = *width * 4;
		*lines = *height;

		VideoHandler* player = static_cast<VideoHandler*>(*data);

		// Fetch video size only once, when playback is just started.
		if (player->_size == Vector2i::Zero)
		{
			player->_size = Vector2i(*width, *height);
			player->InitD3DTexture();
		}

		return 1;
	}

	void VideoHandler::OnLog(void* data, int level, const libvlc_log_t* ctx, const char* fmt, va_list args)
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

		TENLog("VLC: " + std::string(logMessage), logLevel);
	}
}