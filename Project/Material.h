#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <unordered_map>
#include "Graphics.h"
using namespace DirectX;
using namespace Microsoft::WRL;
class Material
{
private:
	const char* name;
	XMFLOAT4 color;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
	XMFLOAT2 UVScale;
	XMFLOAT2 UVOffset;

public:
	Material(XMFLOAT4 color, ComPtr<ID3D11VertexShader> vertexShader, ComPtr<ID3D11PixelShader> pixelShader);
	Material(XMFLOAT4 color, ComPtr<ID3D11VertexShader> vertexShader, ComPtr<ID3D11PixelShader> pixelShader, const char* name);

	// Getters
	const char* GetName();
	XMFLOAT4 GetColor();
	ComPtr<ID3D11VertexShader> GetVertexShader();
	ComPtr<ID3D11PixelShader> GetPixelShader();
	XMFLOAT2 GetUVScale();
	XMFLOAT2 GetUVOffset();
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetSRVs();

	// Setters
	void SetColor(XMFLOAT4 color);
	void SetVertexShader(ComPtr<ID3D11VertexShader> shader);
	void SetPixelShader(ComPtr<ID3D11PixelShader> shader);
	void SetUVScale(XMFLOAT2 scale);
	void SetUVOffset(XMFLOAT2 offset);

	// Adders
	void AddTextureSRV(unsigned int index, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(unsigned int index, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void BindTexturesAndSamplers();
};

