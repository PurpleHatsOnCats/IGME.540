#pragma once
#include <DirectXMath.h>
#include "Lights.h"
#define MAX_LIGHTS 15

struct VertexShaderExternalData {
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInvTranspose;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 lightView;
	DirectX::XMFLOAT4X4 lightProjection;
	
};
struct PixelShaderExternalData {
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
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
struct BlurData {
	int radius;
	float pixelWidth;
	float pixelHeight;
};
struct AberrationData {
	XMFLOAT2 redOffset;
	XMFLOAT2 greenOffset;
	XMFLOAT2 blueOffset;
};