#pragma once
#include "Mesh.h"
#include <wrl/client.h>
#include <memory>
#include <WICTextureLoader.h>
class Sky
{
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> stencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizeState;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
	std::shared_ptr<Mesh> mesh;
public:
	Sky(std::shared_ptr<Mesh> mesh, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vs,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> ps);

	void Draw();

	// Helper for creating a cubemap from 6 individual textures
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

};

