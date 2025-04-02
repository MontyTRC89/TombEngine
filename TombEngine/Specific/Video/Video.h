#pragma once
#include <framework.h>

#include <vlc/vlc.h>
#include <d3d11.h>
#include <string>

namespace TEN::Video
{
	constexpr auto MIN_VIDEO_WIDTH  = 800;
	constexpr auto MIN_VIDEO_HEIGHT = 600;
	constexpr auto MAX_VIDEO_WIDTH  = 2048;
	constexpr auto MAX_VIDEO_HEIGHT = 2048;

	class VideoHandler
	{
	public:
		VideoHandler() = default;
		~VideoHandler();

		void Initialize(const std::string& gameDir, ID3D11Device* device, ID3D11DeviceContext* context);
		bool Play(const std::string& filename);
		void Pause();
		void Resume();
		void Stop();
		bool Sync();
		bool Update();
		void SetVolume(int volume);

	private:
		// VLC core components
		libvlc_instance_t* _vlcInstance = nullptr;
		libvlc_media_player_t* _player = nullptr;

		// Video properties
		int _volume = 100;
		unsigned int _videoWidth = MIN_VIDEO_WIDTH;
		unsigned int _videoHeight = MIN_VIDEO_HEIGHT;
		std::string _videoDirectory = {};
		std::string _currentFilename = {};

		// Render synchronization
		bool _needRender = true;

		// D3D Resources
		unsigned char* _frameBuffer = nullptr;
		ID3D11Device* _d3dDevice = nullptr;
		ID3D11DeviceContext* _d3dContext = nullptr;
		ID3D11Texture2D* _videoTexture = nullptr;
		ID3D11ShaderResourceView* _textureView = nullptr;

		// VLC frame callbacks
		static void* LockFrame(void* data, void** pixels);
		static void  UnlockFrame(void* data, void* picture, void* const* pixels);
		static void  DisplayFrame(void* data, void* picture);

		// Helpers
		bool HandleError();
		bool InitD3DTexture();
		void DeInitD3DTexture();
		void DeInitPlayer();
	};

	extern VideoHandler g_VideoPlayer;
}