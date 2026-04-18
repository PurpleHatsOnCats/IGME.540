#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include "Mesh.h"
#include "BufferStructs.h"
#include "GameEntity.h"
#include "Camera.h"
#include "ColorMath.h"
#include <WICTextureLoader.h>
#include "Lights.h"
#include "Sky.h"

using namespace Microsoft::WRL;
class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	ComPtr<ID3D11VertexShader> LoadVertexShader(std::wstring);
	ComPtr<ID3D11PixelShader> LoadPixelShader(std::wstring);
	void CreateElements();
	void BuildUI(float deltaTime, float totalTime);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Custom fields
	float backgroundColor[4];
	bool showDemo;
	int sliderValue = 0;
	VertexShaderExternalData vertexShaderData;
	PixelShaderExternalData pixelShaderData;
	VertexShaderExternalDataSky vertexShaderDataSky;

	std::vector<std::shared_ptr<Mesh>> shapes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<GameEntity>> gameEntities;
	std::vector<std::shared_ptr<Camera>> cameras;
	int selectedCamera = 0;

	XMFLOAT3 colorGradient;

	std::vector<Light> lights;
	int numLights;
	std::shared_ptr<Sky> sky;
};

