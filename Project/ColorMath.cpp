#include "ColorMath.h"

XMFLOAT3 ColorMath::HSLtoRGB(float hue, float saturation, float light)
{
	hue *= 360.0f;
	// Code taken from https://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion
	float a = saturation * (float)fmin(light, 1 - light);
	auto f = [hue, light, a](float n) -> float {
		float k = (float)fmod(n + hue / 30, 12);
		return light - a * (float)fmax(fmin(k - 3, fmin(9 - k, 1)), -1);};
	return XMFLOAT3(f(0), f(8), f(4));
}

XMFLOAT3 ColorMath::HSLtoRGB(XMFLOAT3 color)
{
	return HSLtoRGB(color.x,color.y,color.z);
}
