#include "Material.h"

Material::Material(XMFLOAT4 color, ComPtr<ID3D11VertexShader> vertexShader, ComPtr<ID3D11PixelShader> pixelShader) : 
	Material( color, vertexShader, pixelShader, "DefaultName") {}
Material::Material(XMFLOAT4 color, ComPtr<ID3D11VertexShader> vertexShader, ComPtr<ID3D11PixelShader> pixelShader, const char* name)
{
	Material::name = name;
	Material::color = color;
	Material::vertexShader = vertexShader;
	Material::pixelShader = pixelShader;
	UVScale = XMFLOAT2(1, 1);
	UVOffset = XMFLOAT2(0, 0);
}

const char* Material::GetName() 
{
	return Material::name;
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

XMFLOAT2 Material::GetUVScale()
{
	return UVScale;
}

XMFLOAT2 Material::GetUVOffset()
{
	return UVOffset;
}

std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Material::GetSRVs()
{
	return textureSRVs;
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

void Material::SetUVScale(XMFLOAT2 scale)
{
	UVScale = scale;
}

void Material::SetUVOffset(XMFLOAT2 offset)
{
	UVOffset = offset;
}

void Material::AddTextureSRV(unsigned int index, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ index, srv });
}

void Material::AddSampler(unsigned int index, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ index, sampler });
}

void Material::BindTexturesAndSamplers()
{
	for (auto i = textureSRVs.begin(); i != textureSRVs.end(); i++) {
		Graphics::Context->PSSetShaderResources(i->first, 1, i->second.GetAddressOf());
	}
	for (auto i = samplers.begin(); i != samplers.end(); i++) {
		Graphics::Context->PSSetSamplers(i->first, 1, i->second.GetAddressOf());
	}
}
