#include <framework.h>
#include "Video.h"

#include <vlc/vlc.h>
#include <iostream>

namespace TEN::Video
{
	VideoHandler* g_VideoPlayer = nullptr;

	VideoHandler::VideoHandler(ID3D11Device* device, ID3D11DeviceContext* context) : 
		vlcInstance(nullptr), player(nullptr), frameBuffer(nullptr),
		d3dDevice(device), d3dContext(context), videoTexture(nullptr), textureView(nullptr), videoWidth(VIDEO_WIDTH), videoHeight(VIDEO_HEIGHT),
		currentTime(0.0f), lastRenderTime(0.0f), volume(100)
	{
		TENLog("Initializing video player...", LogLevel::Info);

		SetEnvironmentVariable("VLC_PLUGIN_PATH", "./vlc/plugins"); // TODO

		// Set VLC arguments, including the audio output module
		const char* args[] = { "--aout=directsound", "--no-video-title" }; // Example arguments
		vlcInstance = libvlc_new(2, args);  // Use the appropriate arguments

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

	bool VideoHandler::LoadVideo(const std::string& filename)
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

		if (frameBuffer)
		{
			delete[] frameBuffer;
			frameBuffer = nullptr;
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

		unsigned int width = 0, height = 0;
		libvlc_video_get_size(player, 0, &width, &height);

		if (width == 0 || height == 0)
		{
			TENLog("Failed to retrieve video dimensions, using default 1920x1080", LogLevel::Error);
			width = VIDEO_WIDTH;
			height = VIDEO_HEIGHT;
		}

		videoWidth = width;
		videoHeight = height;

		libvlc_video_set_format(player, "RGBA", videoWidth, videoHeight, videoWidth * 4);

		frameBuffer = new unsigned char[videoWidth * videoHeight * 4];
		if (!frameBuffer)
		{
			TENLog("Failed to allocate frame buffer", LogLevel::Error);
			return false;
		}

		libvlc_video_set_callbacks(player, LockFrame, UnlockFrame, DisplayFrame, this);

		SetVolume(volume);
		InitD3DTexture();

		return HandleError();
	}

	void VideoHandler::InitD3DTexture()
	{
		if (videoTexture)
			videoTexture->Release();

		if (textureView)
			textureView->Release();

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = videoWidth;
		texDesc.Height = videoHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DYNAMIC;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(d3dDevice->CreateTexture2D(&texDesc, nullptr, &videoTexture)))
			TENLog("Failed to create video texture", LogLevel::Error);

		if (videoTexture != nullptr && FAILED(d3dDevice->CreateShaderResourceView(videoTexture, nullptr, &textureView)))
			TENLog("Failed to create shader resource view", LogLevel::Error);
	}

	void VideoHandler::Play(const std::string& filename)
	{
		if (!LoadVideo(filename))
			return;

		Play();
	}

	void VideoHandler::Play()
	{
		if (CheckPlayerExistence())
			libvlc_media_player_play(player);

		HandleError();
	}

	void VideoHandler::Pause()
	{
		if (CheckPlayerExistence())
			libvlc_media_player_pause(player);

		HandleError();
	}

	void VideoHandler::Stop()
	{
		if (CheckPlayerExistence())
			libvlc_media_player_stop_async(player);

		HandleError();
	}

	bool VideoHandler::Update()
	{
		// Check the current position in the video and compare it to the last rendered time.
		float currentPosition = GetPosition(); // Get current video position (0.0 to 1.0)

		// If the position has changed sufficiently since the last render, render the frame.
		if (currentPosition - lastRenderTime >= 1.0f / 60.0f) // Example: Update 60 FPS
		{
			lastRenderTime = currentPosition;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool VideoHandler::IsPlaying()
	{
		if (!CheckPlayerExistence())
			return false;

		auto state = libvlc_media_player_get_state(player);
		return (state == libvlc_Playing);
	}

	float VideoHandler::GetPosition()
	{
		return (CheckPlayerExistence()) ? libvlc_media_player_get_position(player) : 0.0f;
	}

	void VideoHandler::SetPosition(float position)
	{
		if (CheckPlayerExistence())
			libvlc_media_player_set_position(player, position, true);

		HandleError();
	}

	void VideoHandler::SetVolume(int volume)
	{
		this->volume = std::clamp(volume, 0, 100);

		if (CheckPlayerExistence())
			libvlc_audio_set_volume(player, this->volume);

		HandleError();
	}

	ID3D11ShaderResourceView* VideoHandler::GetTextureView() const
	{
		return textureView;
	}

	bool VideoHandler::CheckPlayerExistence()
	{
		if (!player)
		{
			// TENLog("No video player initialized", LogLevel::Error);
			return false;
		}

		return true;
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