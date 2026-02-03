#pragma once

#include "Vertex.h"
#include "Graphics.h"
#include <d3d11_1.h>
#include <wrl/client.h>

class Mesh
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int vertCount;
	unsigned int indexCount;
	const char* name;
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetVertexCount();
	unsigned int GetIndexCount();
	const char* GetName();
	void Draw();

	Mesh(const char* name, Vertex* vertices, unsigned int vertCount, unsigned int* indices, unsigned int indexCount);
};

