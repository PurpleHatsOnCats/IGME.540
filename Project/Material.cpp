#include "Material.h"

Material::Material(XMFLOAT4 color, ComPtr<ID3D11VertexShader> vertexShader, ComPtr<ID3D11PixelShader> pixelShader)
{
	Material::color = color;
	Material::vertexShader = vertexShader;
	Material::pixelShader = pixelShader;
}

XMFLOAT4 Material::GetColor()
{
	return Material::color;
}

ComPtr<ID3D11VertexShader> Material::GetVertexShader()
{
	return Material::vertexShader;
}

ComPtr<ID3D11PixelShader> Material::GetPixelShader()
{
	return Material::pixelShader;
}

void Material::SetColor(XMFLOAT4 color)
{
	Material::color = color;
}

void Material::SetVertexShader(ComPtr<ID3D11VertexShader> shader)
{
	Material::vertexShader = shader;
}

void Material::SetPixelShader(ComPtr<ID3D11PixelShader> shader)
{
	Material::pixelShader = shader;
}
