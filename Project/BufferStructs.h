#pragma once
#include <DirectXMath.h>

struct VertexShaderExternalData {
public:
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInvTranspose;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
struct PixelShaderExternalData {
public:
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 UVScale;
	DirectX::XMFLOAT2 UVOffset;
	DirectX::XMFLOAT3 cameraPosition;	
	float time;
	DirectX::XMFLOAT3 ambientColor;
};