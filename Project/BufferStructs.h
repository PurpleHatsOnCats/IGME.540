#pragma once
#include <DirectXMath.h>

struct VertexShaderExternalData {
public:
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
struct PixelShaderExternalData {
public:
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 UVScale;
	DirectX::XMFLOAT2 UVOffset;
	float time;
};