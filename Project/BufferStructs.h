#pragma once
#include <DirectXMath.h>
#include "Lights.h"
#define MAX_LIGHTS 30

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
	int numLights;
	Light lights[MAX_LIGHTS];
};
struct VertexShaderExternalDataSky {
public:
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};