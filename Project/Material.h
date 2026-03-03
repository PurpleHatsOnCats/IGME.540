#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
using namespace DirectX;
using namespace Microsoft::WRL;
class Material
{
private:
	XMFLOAT4 color;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
public:
	Material(XMFLOAT4 color, ComPtr<ID3D11VertexShader> vertexShader, ComPtr<ID3D11PixelShader> pixelShader);

	// Getters
	XMFLOAT4 GetColor();
	ComPtr<ID3D11VertexShader> GetVertexShader();
	ComPtr<ID3D11PixelShader> GetPixelShader();

	// Setters
	void SetColor(XMFLOAT4 color);
	void SetVertexShader(ComPtr<ID3D11VertexShader> shader);
	void SetPixelShader(ComPtr<ID3D11PixelShader> shader);
};

