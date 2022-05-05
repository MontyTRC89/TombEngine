#include "framework.h"
#include <d3d11.h>
#include <SimpleMath.h>
#include "RenderQuad.h"
#include "Renderer/Renderer11.h"
#include "../Utils.h"

namespace TEN::Renderer
{
	ComPtr<ID3D11Buffer> quadVertexBuffer;

	void TEN::Renderer::initQuad(ID3D11Device* device)
	{
		std::array<RendererVertex, 4> quadVertices;
		//Bottom Left
		quadVertices[0].Position = Vector3(-0.5, -0.5, 0);
		quadVertices[0].Normal = Vector3(-1, -1, 1);
		quadVertices[0].Normal.Normalize();
		quadVertices[0].UV = Vector2(0, 1);
		quadVertices[0].Color = Vector4(1, 1, 1, 1);
		//Top Left
		quadVertices[1].Position = Vector3(-0.5, 0.5, 0);
		quadVertices[1].Normal = Vector3(-1, 1, 1);
		quadVertices[1].Normal.Normalize();
		quadVertices[1].UV = Vector2(0, 0);
		quadVertices[1].Color = Vector4(1, 1, 1, 1);
		//Top Right
		quadVertices[3].Position = Vector3(0.5, 0.5, 0);
		quadVertices[3].Normal = Vector3(1, 1, 1);
		quadVertices[3].Normal.Normalize();
		quadVertices[3].UV = Vector2(1, 0);
		quadVertices[3].Color = Vector4(1, 1, 1, 1);
		//Bottom Right
		quadVertices[2].Position = Vector3(0.5, -0.5, 0);
		quadVertices[2].Normal = Vector3(1, -1, 1);
		quadVertices[2].Normal.Normalize();
		quadVertices[2].UV = Vector2(1, 1);
		quadVertices[2].Color = Vector4(1, 1, 1, 1);
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(RendererVertex) * 4;
		bufferDesc.MiscFlags = 0x0;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0x0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.StructureByteStride = sizeof(RendererVertex);

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = quadVertices.data();
		initData.SysMemPitch = sizeof(RendererVertex) * quadVertices.size();
		Utils::throwIfFailed(device->CreateBuffer(&bufferDesc, &initData, quadVertexBuffer.GetAddressOf()));
	}
}
