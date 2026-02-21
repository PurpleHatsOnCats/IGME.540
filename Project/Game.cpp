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

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

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

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// Create constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = (sizeof(VertexShaderData)+15)/ 16 * 16; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	Graphics::Device->CreateBuffer(&cbDesc, 0, constBuffer.GetAddressOf());
	Graphics::Context->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

	// Constant buffer data
	vertexShaderData.colorTint = DirectX::XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	vertexShaderData.world = DirectX::XMFLOAT4X4();

	// Create Cameras
	cameras = std::vector<std::shared_ptr<Camera>>();
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0,0,-1), XMConvertToRadians(90.0f), 0.01f, 100.0f, "Main Camera"));
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(-2,2,-3), XMConvertToRadians(45.0f), 0.01f, 100.0f, "Second Camera"));
	cameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0,0,-70), XMConvertToRadians(70.0f), 0.01f, 100.0f, "Other Camera"));
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


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Triangle
	Vertex vertices1[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};
	unsigned int indices1[] = { 0, 1, 2 };

	// Rectangle
	Vertex vertices2[] =
	{
		{ XMFLOAT3(+0.5f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), green },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), red },
		{ XMFLOAT3(-0.5f, +0.5f, +0.0f), green },
	};
	unsigned int indices2[] = { 0, 1, 2, 2, 3, 0 };

	// Pentagon
	Vertex vertices3[] =
	{
		{ XMFLOAT3(-0.3f, +0.0f, +0.0f), red },
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), green },
		{ XMFLOAT3(+0.3f, +0.0f, +0.0f), blue },
		{ XMFLOAT3(+0.15f, -0.5f, +0.0f), red },
		{ XMFLOAT3(-0.15f, -0.5f, +0.0f), blue },
	};
	unsigned int indices3[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4};
	
	shapes.push_back(std::make_shared<Mesh>("Triangle", vertices1, (unsigned int)ARRAYSIZE(vertices1), indices1, (unsigned int)ARRAYSIZE(indices1)));
	shapes.push_back(std::make_shared<Mesh>("Rectangle", vertices2, (unsigned int)ARRAYSIZE(vertices2), indices2, (unsigned int)ARRAYSIZE(indices2)));
	shapes.push_back(std::make_shared<Mesh>("Pentagon", vertices3, (unsigned int)ARRAYSIZE(vertices3), indices3, (unsigned int)ARRAYSIZE(indices3)));

	gameEntities.push_back(std::make_shared<GameEntity>(shapes.at(0), "Entity 0"));
	gameEntities.push_back(std::make_shared<GameEntity>(shapes.at(0), "Entity 1"));
	gameEntities.push_back(std::make_shared<GameEntity>(shapes.at(0), "Entity 2"));
	gameEntities.push_back(std::make_shared<GameEntity>(shapes.at(1), "Entity 3"));
	gameEntities.push_back(std::make_shared<GameEntity>(shapes.at(2), "Entity 4"));
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

	BuildUI(deltaTime);
	
	// Move game entities
	for (unsigned int i = 0; i < gameEntities.size(); i++) {
		switch (i) {
		case 0: gameEntities.at(i)->GetTransform()->SetPosition((float)cos(totalTime), (float)sin(totalTime), 0);
			gameEntities.at(i)->GetTransform()->Rotate(0, 0, deltaTime * 2.0f);
			break;
		case 1: gameEntities.at(i)->GetTransform()->Rotate(0, 0, deltaTime * 2.0f);
			break;
		case 2: gameEntities.at(i)->GetTransform()->Scale((1.0f - (0.1f*deltaTime)), 1, 1);
			gameEntities.at(i)->GetTransform()->SetPosition((float)cos(totalTime)/2.0f, 0, 0);
			break;
		case 3: gameEntities.at(i)->GetTransform()->MoveAbsolute(0.1f * deltaTime, 0.1f * deltaTime, 0.0f);
			gameEntities.at(i)->GetTransform()->Scale((1.0f + (0.2f * deltaTime)), (1.0f - (0.2f * deltaTime)), 1.0f);
			gameEntities.at(i)->GetTransform()->Rotate(0.1f * deltaTime, 0.1f * deltaTime, 0.0f);
			break;
		case 4: gameEntities.at(i)->GetTransform()->SetPosition((float)cos(totalTime * 5.0f), (float)sin(totalTime), 0);
			break;
		default:break;
		}
	}

	// Update Camera
	cameras[selectedCamera]->Update(deltaTime);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		vertexShaderData.view = cameras[selectedCamera]->GetViewMatrix();
		vertexShaderData.projection = cameras[selectedCamera]->GetProjectionMatrix();
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		for (unsigned int i = 0; i < gameEntities.size(); i++) {

			// Copy CPU constBuffer to GPU constBuffer
			D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
			Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer); // Locks GPU buffer and gets GPU address
			vertexShaderData.world = gameEntities.at(i)->GetTransform()->GetWorldMatrix();
			memcpy(mappedBuffer.pData, &vertexShaderData, sizeof(vertexShaderData)); // Copy data from CPU to GPU

			Graphics::Context->Unmap(constBuffer.Get(), 0); // Unlocks GPU buffer

			gameEntities.at(i)->Draw();
		}

		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
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

void Game::BuildUI(float deltaTime) {
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
		ImGui::Text("Window dimensions: %i x %i", Window::Width(), Window::Height());
		ImGui::ColorEdit4("Background color", backgroundColor);
		ImGui::Checkbox("Show demo window", &showDemo);

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
	if (ImGui::TreeNode("Game Entities")) {
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
	ImGui::End();
}


