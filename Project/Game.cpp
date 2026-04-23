#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Helper method for creating some basic
	// geometry to draw and some simple camera matrices.
	CreateElements();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());
	}

	// Constant buffer data
	pixelShaderData.colorTint = DirectX::XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	vertexShaderData.world = DirectX::XMFLOAT4X4();

	// Create Cameras
	cameras = std::vector<std::shared_ptr<Camera>>();
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0,3,-6), XMConvertToRadians(90.0f), 0.01f, 100.0f, "Main Camera"));
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(-2,2,-3), XMConvertToRadians(45.0f), 0.01f, 100.0f, "Second Camera"));
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0,0,-70), XMConvertToRadians(70.0f), 0.01f, 100.0f, "Other Camera"));
	
	pixelShaderData.ambientColor = XMFLOAT3(0.5f, 0.6f,0.5f);

	CreateShadowMap();
}

void Game::CreateShadowMap()
{
	// Create the actual texture that will be the shadow map
	shadowMapResolution = 1024;
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // using srv and dsv
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Reserve 32 bits and the type is determined by the srv(?)
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT; // 32 bits for one value; depth stencil version
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // 2D texture
	shadowDSDesc.Texture2D.MipSlice = 0; // Which mip to use; index 0
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());
	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT; //  32 bits for one value; shader resource version
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; // 2D texture
	srvDesc.Texture2D.MipLevels = 1; // How many mips
	srvDesc.Texture2D.MostDetailedMip = 0; // index 0; we only have 1 mip
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());
	
	// Create rasterizer
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = false; // Keep out-of-frustum objects!
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
}

// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

ComPtr<ID3D11VertexShader> Game::LoadVertexShader(std::wstring filename)
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* vertexShaderBlob;
	ComPtr<ID3D11VertexShader> vertexShader;
	D3DReadFileToBlob(FixPath(filename).c_str(), &vertexShaderBlob);
	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
		vertexShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer


	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a UV, which is 2 float values
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;					// 2x 32-bit floats
		inputElements[1].SemanticName = "TEXCOORD";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Set up the third element - a Normal, which is 3 float values
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// 3x 32-bit floats
		inputElements[2].SemanticName = "NORMAL";							// Match our vertex shader input!
		inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Set up the fourth element - a Tangent, which is 3 float values
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// 3x 32-bit floats
		inputElements[3].SemanticName = "TANGENT";							// Match our vertex shader input!
		inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		inputLayout.Reset();
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			4,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
	return vertexShader;
}

ComPtr<ID3D11PixelShader> Game::LoadPixelShader(std::wstring filename)
{
	ID3DBlob* pixelShaderBlob;
	ComPtr<ID3D11PixelShader> pixelShader;
	D3DReadFileToBlob(FixPath(filename).c_str(), &pixelShaderBlob);
	Graphics::Device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		pixelShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer
	return pixelShader;
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateElements()
{
	// Load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_cobble_albedo;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str(),
		nullptr,
		srv_cobble_albedo.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_cobble_roughness;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str(),
		nullptr,
		srv_cobble_roughness.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_cobble_metal;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_metal.png").c_str(),
		nullptr,
		srv_cobble_metal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_cobble_normals;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(),
		nullptr,
		srv_cobble_normals.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_paint_albedo;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/paint_albedo.png").c_str(),
		nullptr,
		srv_paint_albedo.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_paint_roughness;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/paint_roughness.png").c_str(),
		nullptr,
		srv_paint_roughness.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_paint_metal;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/paint_metal.png").c_str(),
		nullptr,
		srv_paint_metal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_paint_normals;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/paint_normals.png").c_str(),
		nullptr,
		srv_paint_normals.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_scratched_albedo;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(),
		nullptr,
		srv_scratched_albedo.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_scratched_roughness;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(),
		nullptr,
		srv_scratched_roughness.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_scratched_metal;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_metal.png").c_str(),
		nullptr,
		srv_scratched_metal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_scratched_normals;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_normals.png").c_str(),
		nullptr,
		srv_scratched_normals.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_bronze_albedo;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(),
		nullptr,
		srv_bronze_albedo.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_bronze_roughness;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(),
		nullptr,
		srv_bronze_roughness.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_bronze_metal;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_metal.png").c_str(),
		nullptr,
		srv_bronze_metal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_bronze_normals;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(),
		nullptr,
		srv_bronze_normals.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_wood_albedo;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_albedo.png").c_str(),
		nullptr,
		srv_wood_albedo.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_wood_roughness;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_roughness.png").c_str(),
		nullptr,
		srv_wood_roughness.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_wood_metal;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_metal.png").c_str(),
		nullptr,
		srv_wood_metal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_wood_normals;
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(),
		nullptr,
		srv_wood_normals.GetAddressOf());

	ComPtr<ID3D11SamplerState> samplerState;
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Each dimension can
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // have a different mode
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // but that is uncommon
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX; // Maximum mip level

	Graphics::Device->CreateSamplerState(
		&sampDesc,
		samplerState.GetAddressOf());

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	ComPtr<ID3D11VertexShader> vertexShader = LoadVertexShader(L"VertexShader.cso");
	ComPtr<ID3D11VertexShader> vertexShaderNormals = LoadVertexShader(L"NormalMappingVS.cso");
	ComPtr<ID3D11VertexShader> vsSky = LoadVertexShader(L"SkyVS.cso");
	shadowVS = LoadVertexShader(L"ShadowVS.cso");

	ComPtr<ID3D11PixelShader> pixelShader = LoadPixelShader(L"PixelShader.cso");
	ComPtr<ID3D11PixelShader> debugUVsPSShader = LoadPixelShader(L"DebugUVsPS.cso");
	ComPtr<ID3D11PixelShader> debugNormalsPSShader = LoadPixelShader(L"DebugNormalsPS.cso");
	ComPtr<ID3D11PixelShader> customPSShader = LoadPixelShader(L"CustomPS.cso");
	ComPtr<ID3D11PixelShader> twoTexturePSShader = LoadPixelShader(L"TwoTexturePS.cso");
	ComPtr<ID3D11PixelShader> pixelShaderNormals = LoadPixelShader(L"NormalMappingPS.cso");
	ComPtr<ID3D11PixelShader> psSky = LoadPixelShader(L"SkyPS.cso");

	// Create Materials
	materials = std::vector<std::shared_ptr<Material>>();
	materials.push_back(std::make_shared<Material>(white, vertexShaderNormals, pixelShaderNormals, "Cobblestone"));
	materials.at(0)->AddSampler(0, samplerState);
	materials.at(0)->AddTextureSRV(0, srv_cobble_albedo);
	materials.at(0)->AddTextureSRV(1, srv_cobble_roughness);
	materials.at(0)->AddTextureSRV(2, srv_cobble_metal);
	materials.at(0)->AddTextureSRV(3, srv_cobble_normals);
	materials.push_back(std::make_shared<Material>(white, vertexShaderNormals, pixelShaderNormals, "Paint"));
	materials.at(1)->AddSampler(0, samplerState);
	materials.at(1)->AddTextureSRV(0, srv_paint_albedo);
	materials.at(1)->AddTextureSRV(1, srv_paint_roughness);
	materials.at(1)->AddTextureSRV(2, srv_paint_metal);
	materials.at(1)->AddTextureSRV(3, srv_paint_normals);
	materials.push_back(std::make_shared<Material>(white, vertexShaderNormals, pixelShaderNormals, "Scratched"));
	materials.at(2)->AddSampler(0, samplerState);
	materials.at(2)->AddTextureSRV(0, srv_scratched_albedo);
	materials.at(2)->AddTextureSRV(1, srv_scratched_roughness);
	materials.at(2)->AddTextureSRV(2, srv_scratched_metal);
	materials.at(2)->AddTextureSRV(3, srv_scratched_normals);
	materials.push_back(std::make_shared<Material>(white, vertexShaderNormals, pixelShaderNormals, "Bronze"));
	materials.at(3)->AddSampler(0, samplerState);
	materials.at(3)->AddTextureSRV(0, srv_bronze_albedo);
	materials.at(3)->AddTextureSRV(1, srv_bronze_roughness);
	materials.at(3)->AddTextureSRV(2, srv_bronze_metal);
	materials.at(3)->AddTextureSRV(3, srv_bronze_normals);	
	materials.push_back(std::make_shared<Material>(white, vertexShaderNormals, pixelShaderNormals, "Wood"));
	materials.at(4)->AddSampler(0, samplerState);
	materials.at(4)->AddTextureSRV(0, srv_wood_albedo);
	materials.at(4)->AddTextureSRV(1, srv_wood_roughness);
	materials.at(4)->AddTextureSRV(2, srv_wood_metal);
	materials.at(4)->AddTextureSRV(3, srv_wood_normals);
	
	// Create meshes from obj files
	shapes.push_back(std::make_shared<Mesh>("Cube", FixPath("../../Assets/Meshes/cube.obj").c_str()));
	shapes.push_back(std::make_shared<Mesh>("Cylinder", FixPath("../../Assets/Meshes/cylinder.obj").c_str()));
	shapes.push_back(std::make_shared<Mesh>("Helix", FixPath("../../Assets/Meshes/helix.obj").c_str()));
	shapes.push_back(std::make_shared<Mesh>("Quad", FixPath("../../Assets/Meshes/quad.obj").c_str()));
	shapes.push_back(std::make_shared<Mesh>("Quad Double Sided", FixPath("../../Assets/Meshes/quad_double_sided.obj").c_str()));
	shapes.push_back(std::make_shared<Mesh>("Sphere", FixPath("../../Assets/Meshes/sphere.obj").c_str()));
	shapes.push_back(std::make_shared<Mesh>("Torus", FixPath("../../Assets/Meshes/torus.obj").c_str()));

	// Create sky
	sky = std::make_shared<Sky>(
		shapes[0],
		samplerState, 
		Sky::CreateCubemap(
			FixPath(L"../../Assets/Textures/Cold Sunset/right.png").c_str(),
			FixPath(L"../../Assets/Textures/Cold Sunset/left.png").c_str(),
			FixPath(L"../../Assets/Textures/Cold Sunset/up.png").c_str(),
			FixPath(L"../../Assets/Textures/Cold Sunset/down.png").c_str(),
			FixPath(L"../../Assets/Textures/Cold Sunset/front.png").c_str(),
			FixPath(L"../../Assets/Textures/Cold Sunset/back.png").c_str()),
		vsSky,
		psSky
		);

	// Create game entities
	//for (int i = 0; i < materials.size(); i++) {
	//	for (int j = 0; j < shapes.size(); j++) {

	//		// Do some deep memory copy in order to utilize string appending
	//		std::string str_name = (std::to_string(i) + std::string(" x ") + std::to_string(j));
	//		const std::string::size_type size = str_name.size();
	//		char* name = new char[size + 1];   //we need extra char for NUL
	//		memcpy(name, str_name.c_str(), size + 1);

	//		std::shared_ptr<GameEntity> gameEntity = std::make_shared<GameEntity>(shapes.at(j), materials.at(i), name);
	//		gameEntity->GetTransform()->SetPosition(j * 3.0f, i * 3.0f, 5.0f);
	//		gameEntities.push_back(gameEntity);
	//		delete[](name);
	//	}
	//}
	std::shared_ptr<GameEntity> gameEntity = std::make_shared<GameEntity>(shapes.at(0), materials.at(4), "Floor");
	gameEntity->GetTransform()->SetPosition(0, -2, 0);
	gameEntity->GetTransform()->SetScale(15, 1, 15);
	gameEntity->GetMaterial()->SetUVScale(XMFLOAT2(15,15));
	gameEntities.push_back(gameEntity);
	gameEntity = std::make_shared<GameEntity>(shapes.at(2), materials.at(2), "Helix");
	gameEntities.push_back(gameEntity);
	gameEntity = std::make_shared<GameEntity>(shapes.at(0), materials.at(0), "Cube");
	gameEntity->GetTransform()->SetPosition(3,0,0);
	gameEntities.push_back(gameEntity);
	gameEntity = std::make_shared<GameEntity>(shapes.at(1), materials.at(3), "Cylinder");
	gameEntities.push_back(gameEntity);

	// Create lights
	lights = std::vector<Light>();

	Light light = {};
	light.Type = LIGHT_TYPE_DIRECTIONAL;
	light.Color = XMFLOAT3(1,1,1);
	light.Intensity = 1.0f;
	light.Direction = XMFLOAT3(0, -1, 1);
	lights.push_back(light);

	for (int i = 1; i <= MAX_LIGHTS-1; i++) {
		// Create Lights
		light.Type = LIGHT_TYPE_POINT;
		light.Position = XMFLOAT3(
			(float)fmod(fmod(i * i * 57.7821f + i * 65.14f, 42.15f), shapes.size() + 6) - 3, 
			(float)fmod(fmod(i * 5783.9271f, 74.4f), materials.size() * 3 + 3),
			(float)fmod(fmod(i * 7547.536f, 92.8f), 6.0f) + 2);
		light.Color = ColorMath::HSLtoRGB(XMFLOAT3(fmod(fmod(i * i * 57.7821f + i * 65.14f, 42.15f), 1.0f), 1, 0.5f));
		light.Intensity = 10.0f;
		light.Range = 10.0f;
		lights.push_back(light);
	}
	numLights = 1;
}

XMFLOAT4X4 Game::LightView(Light light)
{
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	// See if the direction is directly down
	uint32_t CR;
	XMVectorEqualR(&CR, XMVector3Normalize(XMLoadFloat3(&light.Direction)), XMVectorSet(0, -1, 0, 0));
	if (XMComparisonAllTrue(CR)) {
		up = XMVectorSet(1, 0, 0, 0);
	}
	XMMATRIX lightView = XMMatrixLookToLH(
		XMLoadFloat3(&light.Direction) * -20.0f, // Position: "Backing up" 20 units from origin
		XMLoadFloat3(&light.Direction), // Direction: light's direction
		up); // Up: World up vector (Y axis)
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, lightView);
	return matrix;
}

XMFLOAT4X4 Game::LightProjection(Light light, float lightProjectionSize)
{
	XMMATRIX lightProjection = XMMatrixSet(	
		0,0,0,0,
		0,0,0,0,
		0,0,0,0,
		0,0,0,0);
	switch (light.Type) {
	case LIGHT_TYPE_DIRECTIONAL:
		lightProjection = XMMatrixOrthographicLH(
			lightProjectionSize,
			lightProjectionSize,
			1.0f, // nearclip
			100.0f); // farclip
		break;
	case LIGHT_TYPE_SPOT: break;
	case LIGHT_TYPE_POINT: break;
	}
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, lightProjection);
	return matrix;
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (unsigned int i = 0; i < cameras.size(); i++) {
		cameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	BuildUI(deltaTime, totalTime);
	
	colorGradient = ColorMath::HSLtoRGB(std::fmod(totalTime, 10.0f)/10.0f, 1.0f, 0.5f);

	//// Move game entities
	//for (unsigned int i = 0; i < gameEntities.size(); i++) {
	//	gameEntities.at(i)->GetTransform()->Rotate(0.0f, deltaTime * 1.0f, 0.0f);
	//	if (gameEntities.at(i)->GetMaterial()->GetName() == "Custom Shader") {
	//		gameEntities.at(i)->GetMaterial()->SetColor(XMFLOAT4(colorGradient.x, colorGradient.y, colorGradient.z, 1.0f));
	//	}
	//}
	if (!freezeEntities) {
		gameEntities[1]->GetTransform()->SetPosition(0, sinf(totalTime) * 3 + 2, 0);
		gameEntities[2]->GetTransform()->Rotate(0, deltaTime, 0);
		gameEntities[3]->GetTransform()->SetPosition(sinf(totalTime * 3) * 5 - 2, 3, 0);
	}
	
	// Update Camera
	cameras[selectedCamera]->Update(deltaTime);

	// Update light matrices
	lightViewMatrix = LightView(lights[0]);
	lightProjectionMatrix = LightProjection(lights[0], shadowMapSize);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Shadow map
	{
		// Clear depth stencil
		Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Change target depth stencil and unbind back buffer
		ID3D11RenderTargetView* nullRTV{};
		Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

		// Deactivate pixel shader
		Graphics::Context->PSSetShader(0, 0, 0);

		// Change the viewport
		D3D11_VIEWPORT viewport = {};
		viewport.Width = (float)shadowMapResolution; // matches shadow map
		viewport.Height = (float)shadowMapResolution;
		viewport.MaxDepth = 1.0f;
		Graphics::Context->RSSetViewports(1, &viewport);

		// Set Vertex Shader
		Graphics::Context->VSSetShader(shadowVS.Get(), 0, 0);

		// Render Enities
		struct ShadowVSData
		{
			XMFLOAT4X4 world;
			XMFLOAT4X4 view;
			XMFLOAT4X4 proj;
		};

		ShadowVSData vsData = {};
		vsData.view = lightViewMatrix;
		vsData.proj = lightProjectionMatrix;

		// Set rasterizer
		Graphics::Context->RSSetState(shadowRasterizer.Get());

		// Loop and draw all entities
		for (auto& e : gameEntities)
		{
			vsData.world = e->GetTransform()->GetWorldMatrix();
			Graphics::FillAndBindNextConstantBuffer(&vsData, sizeof(ShadowVSData), D3D11_VERTEX_SHADER, 0); // Shortened for space
			e->GetMesh()->Draw();
		}

		// Unset rasterizer
		Graphics::Context->RSSetState(0);

		// Reset viewport
		viewport.Width = Window::Width();
		viewport.Height = Window::Height();
		Graphics::Context->RSSetViewports(1, &viewport);

		// Rebind correct back and depth buffer
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}

	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		vertexShaderData.view = cameras[selectedCamera]->GetViewMatrix();
		vertexShaderData.projection = cameras[selectedCamera]->GetProjectionMatrix();
		vertexShaderDataSky.view = vertexShaderData.view;
		vertexShaderDataSky.projection = vertexShaderData.projection;

		pixelShaderData.time = totalTime;
		pixelShaderData.cameraPosition = cameras[selectedCamera]->GetTransform()->GetPosition();

		// Copy light data into ps buffer struct
		pixelShaderData.numLights = numLights;
		Light* lightsArray = lights.data();
		memcpy(&pixelShaderData.lights, lightsArray, sizeof(Light) * numLights);

		vertexShaderData.lightProjection = lightProjectionMatrix;
		vertexShaderData.lightView = lightViewMatrix;
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		// Bind shadow map
		Graphics::Context->PSSetShaderResources(4, 1, shadowSRV.GetAddressOf());
		Graphics::Context->PSSetSamplers(1, 1, shadowSampler.GetAddressOf());

		for (unsigned int i = 0; i < gameEntities.size(); i++) {
			// Update vertex shader data
			vertexShaderData.world = gameEntities.at(i)->GetTransform()->GetWorldMatrix();
			vertexShaderData.worldInvTranspose = gameEntities.at(i)->GetTransform()->GetWorldInverseTransposeMatrix();
			
			// Update pixel shader data
			pixelShaderData.colorTint = gameEntities.at(i)->GetMaterial()->GetColor();
			pixelShaderData.UVScale = gameEntities.at(i)->GetMaterial()->GetUVScale();
			pixelShaderData.UVOffset = gameEntities.at(i)->GetMaterial()->GetUVOffset();

			Graphics::FillAndBindNextConstantBuffer(
				&vertexShaderData,
				sizeof(VertexShaderExternalData),
				D3D11_VERTEX_SHADER,
				0
			);
			Graphics::FillAndBindNextConstantBuffer(
				&pixelShaderData,
				sizeof(PixelShaderExternalData),
				D3D11_PIXEL_SHADER,
				0
			);
			gameEntities.at(i)->GetMaterial()->BindTexturesAndSamplers();
			gameEntities.at(i)->Draw();
		}

		// Draw Sky
		Graphics::FillAndBindNextConstantBuffer(
			&vertexShaderDataSky,
			sizeof(VertexShaderExternalDataSky),
			D3D11_VERTEX_SHADER,
			0
		);
		sky->Draw();

		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Unbind all SRVs
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

void Game::BuildUI(float deltaTime, float totalTime) {
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	if (showDemo)
		ImGui::ShowDemoWindow();

	// Custom Window
	ImGui::Begin("My first window");
	if (ImGui::TreeNode("Window data")) {
		ImGui::Text("Fps: %f", ImGui::GetIO().Framerate);
		ImGui::Text("TotalTime: %f", totalTime);
		ImGui::Text("Window dimensions: %i x %i", Window::Width(), Window::Height());
		ImGui::ColorEdit4("Background color", backgroundColor);
		ImGui::Checkbox("Show demo window", &showDemo);
		ImGui::InputFloat3("Gradient Color", &colorGradient.x);

		const char* items[] = { "Apple","Banana","Orange","Tomatoe" };
		static int item_selected_idx = 0;
		if (ImGui::BeginCombo("Favorite fruit", items[item_selected_idx])) {
			for (int n = 0; n < IM_COUNTOF(items); n++)
			{
				const bool is_selected = (item_selected_idx == n);
				if (ImGui::Selectable(items[n], is_selected))
					item_selected_idx = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::TextColored(ImVec4(backgroundColor[0], backgroundColor[1], backgroundColor[2], 255), "This is the background color!");
		ImGui::VSliderInt("##1", ImVec2(18, 160), &sliderValue, 0, 5);
		ImGui::SameLine();
		ImGui::VSliderInt("##2", ImVec2(18, 160), &sliderValue, 0, 5);
		ImGui::SameLine();
		ImGui::VSliderInt("##3", ImVec2(18, 160), &sliderValue, 0, 5);
		ImGui::SameLine();
		ImGui::SliderInt("##4", &sliderValue, 0, 5);
		ImGui::SliderInt("##5", &sliderValue, 0, 5);
		ImGui::SliderInt("##6", &sliderValue, 0, 5);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Meshes")) {
		for (unsigned int i = 0; i < shapes.size(); i++) {
			if (ImGui::TreeNode(shapes.at(i)->GetName())) {
				ImGui::Text("Triangles: %i", (shapes.at(i)->GetIndexCount() / 3));
				ImGui::Text("Vertices: %i", shapes.at(i)->GetVertexCount());
				ImGui::Text("Indices: %i", shapes.at(i)->GetIndexCount());
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Materials")) {
		for (unsigned int i = 0; i < materials.size(); i++) {
			if (ImGui::TreeNode(materials.at(i)->GetName())) {
				XMFLOAT4 color = materials.at(i)->GetColor();
				ImGui::DragFloat4("Color", (float*)&color, 0.01f, 0.0f, 1.0f);
				materials.at(i)->SetColor(color);

				XMFLOAT2 uvscale = materials.at(i)->GetUVScale();
				ImGui::DragFloat2("UV Scale", (float*)&uvscale, 0.01f);
				materials.at(i)->SetUVScale(uvscale);

				XMFLOAT2 uvoffset = materials.at(i)->GetUVOffset();
				ImGui::DragFloat2("UV Offset", (float*)&uvoffset, 0.01f);
				materials.at(i)->SetUVOffset(uvoffset);

				std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs = materials.at(i)->GetSRVs();
				for (auto i = textureSRVs.begin(); i != textureSRVs.end(); i++) {
					ImGui::Image((void*)i->second.Get(),ImVec2(100,100));
				}

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Game Entities")) {
		ImGui::Checkbox("Freeze entities", &freezeEntities);
		for (unsigned int i = 0; i < gameEntities.size(); i++) {
			if (ImGui::TreeNode(gameEntities.at(i)->GetName())) {
				XMFLOAT3 position = gameEntities.at(i)->GetTransform()->GetPosition();
				ImGui::DragFloat3("Position", (float*)&position, 0.01f);
				gameEntities.at(i)->GetTransform()->SetPosition(position);

				XMFLOAT3 rotation = gameEntities.at(i)->GetTransform()->GetPitchYawRoll();
				ImGui::DragFloat3("Rotation", (float*)&rotation, 0.01f);
				gameEntities.at(i)->GetTransform()->SetRotation(rotation);

				XMFLOAT3 scale = gameEntities.at(i)->GetTransform()->GetScale();
				ImGui::DragFloat3("Scale", (float*)&scale, 0.01f);
				gameEntities.at(i)->GetTransform()->SetScale(scale);

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Cameras")) {
		if (ImGui::BeginCombo("Selected Camera", cameras[selectedCamera]->GetName())) {
			for (int i = 0; i < cameras.size(); i++)
			{
				const bool is_selected = (selectedCamera == i);
				if (ImGui::Selectable(cameras[i]->GetName(), is_selected))
					selectedCamera = i;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::TextColored(ImVec4(0.5f,0.5f,1.0f,1.0f), "Controls (?)");
		ImGui::SetItemTooltip("WASD - move directionally\nC - Move down\nSpace - Move up\nRight click + mouse move - look around\nCtrl + scoll - change move speed", ImGuiHoveredFlags_DelayNone);
		for (unsigned int i = 0; i < cameras.size(); i++) {
			if (ImGui::TreeNode(cameras[i]->GetName())) {

				XMFLOAT3 position = cameras.at(i)->GetTransform()->GetPosition();
				ImGui::DragFloat3("Position", (float*)&position, 0.01f);
				cameras.at(i)->GetTransform()->SetPosition(position);

				XMFLOAT3 rotation = cameras.at(i)->GetTransform()->GetPitchYawRoll();
				ImGui::DragFloat3("Rotation", (float*)&rotation, 0.01f);
				cameras.at(i)->GetTransform()->SetRotation(rotation);

				float speed = cameras[i]->GetMoveSpeed();
				ImGui::DragFloat("Camera Speed", &speed, 0.01f, 0.0f, 0.0f, "%.3f");
				cameras[i]->SetMoveSpeed(speed);

				float sensitivity = cameras[i]->GetSensitivity();
				ImGui::DragFloat("Look Sensitivity", &sensitivity, 0.01f, 0.0f, 0.0f, "%.3f");
				cameras[i]->SetSensitivity(sensitivity);

				float fov = cameras[i]->GetFOV();
				ImGui::DragFloat("Field of View", &fov, 0.01f, 0.0f, 0.0f, "%.3f");
				cameras[i]->SetFOV(fov);

				float nearplane = cameras[i]->GetNearplane();
				ImGui::DragFloat("Near Plane", &nearplane, 0.01f, 0.0f, 0.0f, "%.3f");
				cameras[i]->SetNearplane(nearplane);

				float farplane = cameras[i]->GetFarplane();
				ImGui::DragFloat("Far Plane", &farplane, 0.01f, 0.0f, 0.0f, "%.3f");
				cameras[i]->SetFarPlane(farplane);

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
		
	}
	if (ImGui::TreeNode("Lights")) {
		ImGui::SliderInt("Number of Lights", &numLights, 1, (int)lights.size());
		for (unsigned int i = 0; i < (unsigned int)numLights; i++) {
			char name[10] = "Light "; // can take up to 3 digits
			strcat_s(name, std::to_string(i).c_str());
			if (ImGui::TreeNode(name)) {

				// Dropdown for light types
				const char* lightTypes[] = { "Directional","Point","Spot" };
				if (ImGui::BeginCombo("Light Type", lightTypes[lights[i].Type])) {
					for (int n = 0; n < IM_COUNTOF(lightTypes); n++)
					{
						const bool is_selected = (lights[i].Type == n);
						if (ImGui::Selectable(lightTypes[n], is_selected))
							lights[i].Type = n;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::ColorEdit3("Color", (float*)&lights[i].Color, ImGuiColorEditFlags_Float);
				ImGui::DragFloat("Intensity", (float*)&lights[i].Intensity, 0.01f, 0.0f, FLT_MAX);
				if (lights[i].Type != LIGHT_TYPE_POINT) {
					ImGui::DragFloat3("Direction", (float*)&lights[i].Direction, 0.01f);
				}
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					ImGui::DragFloat3("Position", (float*)&lights[i].Position, 0.01f);
					ImGui::DragFloat("Range", (float*)&lights[i].Range, 0.01f, 0.0f, FLT_MAX);
				}
				if (lights[i].Type == LIGHT_TYPE_SPOT) {
					ImGui::DragFloat("Inner Angle", (float*)&lights[i].SpotInnerAngle, 0.01f, 0.0f, lights[i].SpotOuterAngle);
					ImGui::DragFloat("Outer Angle", (float*)&lights[i].SpotOuterAngle, 0.01f, lights[i].SpotInnerAngle, 360.0f);
					if (lights[i].SpotInnerAngle > lights[i].SpotOuterAngle) {
						lights[i].SpotOuterAngle = lights[i].SpotInnerAngle;
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Shadow Map")) {
		//ImGui::DragInt("Map Resolution", &shadowMapResolution,1, 64, 4048);
		ImGui::DragFloat("Map Size", &shadowMapSize, 1.0f, 1.0f, 128.0f);
		ImGui::Image(shadowSRV.Get(), ImVec2(256, 256));
		ImGui::TreePop();
	}
	ImGui::End();
}