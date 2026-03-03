#pragma once
#include <DirectXMath.h>

struct VertexShaderData {
public:
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};