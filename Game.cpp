#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <string>
#include <DirectXMath.h>
#include "BufferStructs.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
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

	//Creating the CONSTANT BUFFER
	{
		unsigned int size = sizeof(VertexShaderData);
		size = (size + 15) / 16 * 16;

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.ByteWidth = size;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;

		Graphics::Device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

		vsData.colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
		//vsData.offset = XMFLOAT3(0.25f, 0.0f, 0.0f);
	}

	cameraList.push_back(std::make_shared<Camera>((float)Window::Width() / Window::Height(), 
		XMFLOAT3(0.0f, 0.0f, -5.0f), 
		XMFLOAT3(0.0f, 0.0f, 0.0f), 
		5.0f, 1.0f, 
		XM_PIDIV4, 
		0.001f, 
		1000.0f, 
		false));

	cameraList.push_back(std::make_shared<Camera>((float)Window::Width() / Window::Height(),
		XMFLOAT3(3.0f, 6.0f, -5.0f),
		XMFLOAT3(9.0f, 1.0f, 0.0f),
		5.0f, 1.0f,
		XM_PIDIV2,
		0.001f,
		1000.0f,
		false));

	cameraList.push_back(std::make_shared<Camera>((float)Window::Width() / Window::Height(),
		XMFLOAT3(1.0f, 2.0f, -5.0f),
		XMFLOAT3(5.0f, -2.0f, 0.0f),
		5.0f, 1.0f,
		XM_PIDIV4,
		0.001f,
		1000.0f,
		false));

	cameraList.push_back(std::make_shared<Camera>((float)Window::Width() / Window::Height(),
		XMFLOAT3(-1.0f, 4.0f, -5.0f),
		XMFLOAT3(-2.0f, -1.0f, 0.0f),
		5.0f, 1.0f,
		XM_PIDIV2,
		0.001f,
		1000.0f,
		false));

	camera = cameraList[activeCamera];

	// Initializing ImGui itself and platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	// ImGui Style
	ImGui::StyleColorsDark();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean ups
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
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 purple = XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices1[] =
	{
		{ XMFLOAT3(+0.0f, +0.3f, +0.0f), red },
		{ XMFLOAT3(+0.3f, -0.3f, +0.0f), blue },
		{ XMFLOAT3(-0.3f, -0.3f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices1[] = { 0, 1, 2 };

	Vertex vertices2[] = {
		{ XMFLOAT3(-0.80f, +0.80f, +0.0f), purple },
		{ XMFLOAT3(-0.80f, +0.40f, +0.0f), purple },
		{ XMFLOAT3(-0.40f, +0.40f, +0.0f), green },
		{ XMFLOAT3(-0.40f, +0.80f, +0.0f), green },
	};

	unsigned int indices2[] = { 
		0,3,2,
		0,2,1 
	};

	Vertex vertices3[] = {
		//right side
		{ XMFLOAT3(+0.70f, -0.40f, +0.0f), blue},
		{ XMFLOAT3(+0.60f, -0.60f, +0.0f), blue}, //top left
		{ XMFLOAT3(+0.60f, -0.40f, +0.0f), black }, //bl
		//Middle
		{ XMFLOAT3(+0.40f, -0.40f, +0.0f), black }, //br
		{ XMFLOAT3(+0.40f, -0.60f, +0.0f), red }, //tr
		//right side
		{ XMFLOAT3(+0.30f, -0.40f, +0.0f), red},
	};

	//boat
	unsigned int indices3[] = {
		0, 1, 2,
		1, 3, 2,
		1, 4, 3,
		4, 5, 3
	};

	//Creating Meshes
	//puting the mesh data into list so data can be displayed
	std::shared_ptr<Mesh> triangle = std::make_shared<Mesh>("Triangle", vertices1, ARRAYSIZE(vertices1), indices1, ARRAYSIZE(indices1));
	meshList.push_back(triangle);

	std::shared_ptr<Mesh> quad = std::make_shared<Mesh>("Quad", vertices2, ARRAYSIZE(vertices2), indices2, ARRAYSIZE(indices2));
	meshList.push_back(quad);

	std::shared_ptr<Mesh> boat = std::make_shared<Mesh>("Boat", vertices3, ARRAYSIZE(vertices3), indices3, ARRAYSIZE(indices3));
	meshList.push_back(boat);

	//Creating Game Entity
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(triangle);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(quad);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(boat);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(boat);
	std::shared_ptr<Entity> entity5 = std::make_shared<Entity>(boat);

	entity1->GetTransform()->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	entity2->GetTransform()->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	entity3->GetTransform()->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	entity4->GetTransform()->SetPosition(XMFLOAT3(-0.2f, 0.6f, 0.0f));
	entity5->GetTransform()->SetPosition(XMFLOAT3(-0.09f, 0.9f, 0.0f));

	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
	entities.push_back(entity5);
	
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (camera) 
	{
		camera->UpdateProjectionMatrix((float)Window::Width() / Window::Height());
	};
}

// Helper methods
void Game::ImGuiUpdate(float deltaTime) {
	//Fresh data
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	//Resetting frames
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);

	//Show demo window
	if(showDemo) ImGui::ShowDemoWindow();
}

void Game::BuildUI()
{
	ImGui::Begin("Inspector"); // Inspector

	if (ImGui::TreeNode("App Details")) {
		ImGui::Text("Frame rate: %f fps", ImGui::GetIO().Framerate); //displaying frame rate
		ImGui::Text("Window Client Size: %dx%d", Window::Width(), Window::Height()); //displaying window size

		//color og background
		ImGui::ColorEdit4("Background Color", color);

		//Button to show ImGui Demo
		if (showDemo == false) {
			if (ImGui::Button("Show ImGui Demo Window")) showDemo = true;
		}
		else {
			if (ImGui::Button("Close ImGui Demo Window")) showDemo = false;
		}

		ImGui::TreePop(); // popped app details
	}

	if (ImGui::TreeNode("Meshes")) {

		for (auto& m : meshList) {
			if (ImGui::TreeNode(m->GetName())) {
				ImGui::Text("Triangles: %d", m->GetIndexCount() / 3);
				ImGui::Text("Vertices: %d", m->GetVertexCount());
				ImGui::Text("Indices: %d", m->GetIndexCount());
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	//float offset[3] = {vsData.offset.x, vsData.offset.y, vsData.offset.z };
	float color[4] = { vsData.colorTint.x, vsData.colorTint.y, vsData.colorTint.z, vsData.colorTint.w };

	if (ImGui::TreeNode("Scene Entities")) {
		for (int i = 0; i < entities.size(); i++) {
			std::string label = "Entity #" + std::to_string(i + 1);
			ImGui::PushID(i);
			XMFLOAT3 position = entities[i]->GetTransform()->GetPosition();
			XMFLOAT3 rotation = entities[i]->GetTransform()->GetPitchYawRoll();
			XMFLOAT3 scale = entities[i]->GetTransform()->GetScale();

			if (ImGui::TreeNode(label.c_str())) {

				ImGui::SliderFloat3("Position", &position.x, -1.0f, 1.0f);
				entities[i]->GetTransform()->SetPosition(position);

				ImGui::SliderFloat3("Rotation (Radians)", &rotation.x, -180.0f, 180.0f);
				entities[i]->GetTransform()->SetRotation(rotation.x, rotation.y, rotation.z);

				ImGui::SliderFloat3("Scale", &scale.x, 0.1f, 2.0f);
				entities[i]->GetTransform()->SetScale(scale);

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		
		ImGui::TreePop();
	}

	//Camera Switching
	std::string cameraLabel = "Camera in use: " + std::to_string(activeCamera + 1);
	if (ImGui::TreeNode(cameraLabel.c_str())) {

		if (ImGui::TreeNode("Cameras")) {
			if (ImGui::Button("Camera 1 ([0])")) { activeCamera = 0; camera = cameraList[activeCamera]; }
			if (ImGui::Button("Camera 2 ([1])")) { activeCamera = 1; camera = cameraList[activeCamera]; }
			if (ImGui::Button("Camera 3 ([3])")) { activeCamera = 2; camera = cameraList[activeCamera]; }
			if (ImGui::Button("Camera 4 ([4])")) { activeCamera = 3; camera = cameraList[activeCamera]; }
			ImGui::TreePop();
		}


		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Random Things")) {
		ImGui::InputInt("size", &num);
		ImGui::DragFloat("float drag", &fnum);
		ImGui::Checkbox("check", &check);
	
		ImGui::TreePop();

	}

	ImGui::End(); //Inspector
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	ImGuiUpdate(deltaTime);
	BuildUI();

	entities[0]->GetTransform()->Rotate(XMFLOAT3(0, 0, deltaTime));

	camera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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
		/*const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };*/
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//ColorTint and Offset
	{

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
		Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

		memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

		Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);

		//Binding Constant Buffer
		Graphics::Context->VSSetConstantBuffers(0, 1, vsConstantBuffer.GetAddressOf());
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		for (int i = 0; i < entities.size(); i++) {
			entities[i]->Draw(vsConstantBuffer, camera);
		}
	}
	//ImGui
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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



