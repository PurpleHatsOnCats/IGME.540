#pragma once
#include <DirectXMath.h>
using namespace DirectX;
class ColorMath
{
public:
	static XMFLOAT3 HSLtoRGB(float hue, float saturation, float light);
	static XMFLOAT3 HSLtoRGB(XMFLOAT3 color);
};

