#pragma once
#include <framework.h>

#include <vlc/vlc.h>
#include <d3d11.h>
#include <string>

namespace TEN::Video
{
	constexpr auto VIDEO_WIDTH = 1920;
	constexpr auto VIDEO_HEIGHT = 1080;

	class VideoHandler
	{
	public:
		VideoHandler(ID3D11Device* device, ID3D11DeviceContext* context);
		~VideoHandler();

		bool Play(const std::string& filename);
		void Pause();
		void Resume();
		void Stop();
		bool Sync();
		bool Update();
		float GetPosition();
		void SetPosition(float position);
		void SetVolume(int volume);

		ID3D11ShaderResourceView* GetTextureView() const;

	private:
		// VLC core components
		libvlc_instance_t* vlcInstance = nullptr;
		libvlc_media_player_t* player = nullptr;

		// Video properties
		int volume = 100;
		unsigned int videoWidth = VIDEO_WIDTH;
		unsigned int videoHeight = VIDEO_HEIGHT;
		std::string currentFilename = {};

		// Render synchronization
		bool needRender = true;

		// D3D Resources
		unsigned char* frameBuffer = nullptr;
		ID3D11Device* d3dDevice = nullptr;
		ID3D11DeviceContext* d3dContext = nullptr;
		ID3D11Texture2D* videoTexture = nullptr;
		ID3D11ShaderResourceView* textureView = nullptr;

		// VLC frame callbacks
		static void* LockFrame(void* opaque, void** pixels);
		static void  UnlockFrame(void* opaque, void* picture, void* const* pixels);
		static void  DisplayFrame(void* opaque, void* picture);

		// Helpers
		bool HandleError();
		bool InitD3DTexture();
	};

	extern VideoHandler* g_VideoPlayer;
}