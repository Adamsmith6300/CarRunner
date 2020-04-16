//***************************************************************************************
// App.cpp by Frank Luna (C) 2015 All Rights Reserved.
//
// Hold down '1' key to view scene in wireframe mode.
//***************************************************************************************


#include <thread> 
#include <map>
#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "../../Common/camera.h"
#include "../../Common/Entity.h"
#include "../../Physics/Physics.h"
#include "SkullAI.h"
#include "Server.h"
#include "Client.h"
#include "FrameResource.h"
#include "RenderItem.h"

#define ENTMAP map<string, Entity*>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources = 3;

//For Development only
const bool isTopDown = false;
XMFLOAT3 topPos = { 0.0f, 20.0f, 0.0f };

// Control the acceleration, smaller = longer acceleration
const float decel = 0.025f;

enum class RenderLayer : int
{
	Opaque = 0,
	Sky,
	Count
};


class App : public D3DApp
{
public:
	App(HINSTANCE hInstance);
	App(const App& rhs) = delete;
	App& operator=(const App& rhs) = delete;
	~App();

	virtual bool Initialize()override;
	virtual int Run()override;
	virtual void CalculateFrameStats();

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdatePlayer2(const GameTimer& gt);
	void MoveCars(const GameTimer& gt);
	void MoveSkulls(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialBuffer(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	bool collisionCheck(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax, XMMATRIX& firstboxworld, XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, XMMATRIX& secondboxworld);
	void calcAABB(std::vector<XMFLOAT3> boxVerts, XMFLOAT4X4& worldspace, XMVECTOR& boxmin, XMVECTOR& boxmax);
	void CreateBoundingVolumes(std::vector<GeometryGenerator::Vertex>& vertPosArray, std::vector<XMFLOAT3>& boundingBoxVerts, std::vector<DWORD>& boundingBoxIndex);
	void CreateBoundingVolumes(std::vector<Vertex>& vertPosArray, std::vector<XMFLOAT3>& boundingBoxVerts, std::vector<int32_t>& boundingBoxIndex);
	//void handleCollision(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax,XMFLOAT3& firsttranslation,XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, float speed,XMFLOAT3 velocity, float deltatime);
	XMFLOAT3 makeCeil(XMFLOAT3 first, XMFLOAT3 second);
	XMFLOAT3 makeFloor(XMFLOAT3 first, XMFLOAT3 second);

	void BuildEnt(string name, XMFLOAT3 pos, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 look);
	void BuildEnt(string name);
	Entity* FindEnt(string name);

	void collision(string ent, XMFLOAT3& pos, float dt);

	void LoadTextures();
	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildplatformGeometry();
	void BuildTruckGeometry();
	void BuildSkullGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	void SetupClientServer();
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	RenderItem* mBoxItemMovable;
	XMFLOAT3 pos = { -2.5f, 0.5f, 0.0f };
	XMFLOAT3 right = { pos.x + 1, pos.y, pos.z };
	XMFLOAT3 up = { pos.x, pos.y + 1, pos.z };
	XMFLOAT3 look = { pos.x, pos.y, pos.z + 1 };

	ENTMAP ents = {};

	//global variables for the bounding box
	RenderItem* firstbox = nullptr;
	RenderItem* secondbox = nullptr;
	std::vector<XMFLOAT3> boxBoundingVertPosArray;
	std::vector<DWORD> boxBoundingVertIndexArray;
	GeometryGenerator::MeshData box;

	std::vector<XMFLOAT3> carBoundingVertPosArray;
	std::vector<DWORD> carBoundingVertIndexArray;
	GeometryGenerator::MeshData car;

	std::vector<XMFLOAT3> platformBoundingVertPosArray;
	std::vector<DWORD> platformBoundingVertIndexArray;
	GeometryGenerator::MeshData platform;

	std::vector<XMFLOAT3> roadBoundingVertPosArray;
	std::vector<DWORD> roadBoundingVertIndexArray;
	GeometryGenerator::MeshData road;

	std::vector<XMFLOAT3> skullBoundingVertPosArray;
	std::vector<DWORD> skullBoundingVertIndexArray;
	GeometryGenerator::MeshData skull;

	UINT carsCBIndexStart = 0;
	UINT carCount = 39;

	// AI related
	std::vector<SkullAI> skullControllers;
	UINT skullCount = 10;
	UINT skullCbIndexStart = 0;
	UINT skullCbIndexMax = 0;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];
	UINT mSkyTexHeapIndex = 0;
	PassConstants mMainPassCB;

	UINT mPassCbvOffset = 0;

	bool mIsWireframe = false;

	Camera mCamera;

    POINT mLastMousePos;

    POINT deltaMousePos;
    POINT relLastMousePos;
	POINT relCurrMousePos;
	POINT jumpChange;

	Entity* winner = nullptr;

	//networking
	Server* gameServer = nullptr;
	Client* gameClient = nullptr;
	thread clientThread;
	thread serverThread;
	bool isHost = false;
	bool sPMode = false;
	float startTime = 0.0f;
	float endTime = 0.0f;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		App theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException & e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

App::App(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

App::~App()
{
	/*std::map<string, Entity*>::iterator it = ents.begin();
	while (it != ents.end()) {
		delete it->second;
		++it;
	}*/
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}
int App::Run()
{
	MSG msg = { 0 };

	mTimer.Reset();

	if (isHost) {
		serverThread = thread(&Server::start, gameServer);
	}
	clientThread = thread(&Client::start, gameClient);

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			mTimer.Tick();

			if (isHost && GetAsyncKeyState('3') & 0x8000 && !gameClient->gameActive && gameClient->winner == 0) {
				gameServer->sendStartGame();
				startTime = mTimer.TotalTime();
			}
			if (sPMode && GetAsyncKeyState('3') & 0x8000 && !gameClient->gameActive && gameClient->winner == 0) {
				gameClient->gameActive = true;
				startTime = mTimer.TotalTime();
			}

			CalculateFrameStats();
			Update(mTimer);
			Draw(mTimer);
		}
	}
	delete gameClient;
	delete gameServer;
	if (isHost) {
		serverThread.join();
	}
	clientThread.join();

	return (int)msg.wParam;
}

void App::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);
		wstring timeTotal = to_wstring(endTime - startTime);

		wstring playerMessage = L"Run!";
		if (!gameClient->gameActive) {
			if (isHost) {
				playerMessage = L"Press 3 to start the game...";
			}
			else {
				playerMessage = L"Waiting on host to start the game...";
			}
			if (sPMode)playerMessage = L"Press 3 to start the game...";
			if (gameClient->winner > 0)playerMessage = L"Congratulations! You won!  Time: " + timeTotal;
			if (gameClient->winner < 0)playerMessage = L"Uh oh! You lost!";
		}


		wstring windowText = mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr + L"            " + playerMessage;

		SetWindowText(mhMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}


bool App::Initialize()
{

	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (isTopDown) {
		mCamera.SetPosition(topPos);
		mCamera.Pitch(MathHelper::Pi / 2);
	}
	LoadTextures();

	//entities created
	BuildEnt("player", pos, right, up, look);
	BuildEnt("block");

	//to help with decision making for skull AI
	srand(time(NULL));

	SetupClientServer();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildTruckGeometry();
	BuildplatformGeometry();
	BuildSkullGeometry();
	BuildMaterials();
	//for creating the necessary vertices for bounding boxes
	CreateBoundingVolumes(box.Vertices, boxBoundingVertPosArray, boxBoundingVertIndexArray);
	CreateBoundingVolumes(platform.Vertices, platformBoundingVertPosArray, platformBoundingVertIndexArray);
	CreateBoundingVolumes(road.Vertices, roadBoundingVertPosArray, roadBoundingVertIndexArray);
	CreateBoundingVolumes(skull.Vertices, skullBoundingVertPosArray, skullBoundingVertIndexArray);
	//CreateBoundingVolumes();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	//hide mouse
	ShowCursor(false);
	relLastMousePos.x = relLastMousePos.y = relCurrMousePos.x = relCurrMousePos.y = 0;
	mLastMousePos.x = mLastMousePos.y = 0;
	deltaMousePos.x = deltaMousePos.y = 0;
	jumpChange.x = jumpChange.y = 0;

    return true;
}

void App::OnResize()
{
	D3DApp::OnResize();

	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

}

void App::Update(const GameTimer& gt)
{
	if (!mAppPaused)
	{
		OnKeyboardInput(gt);
	}
	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	/*std::wstring output = L"Delta:" + to_wstring(gt.DeltaTime()) + L"\n";
	OutputDebugString(output.c_str());*/

	UpdatePlayer2(gt);
	AnimateMaterials(gt);

	if (gameClient->gameActive)MoveCars(gt);
	if (sPMode) MoveSkulls(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialBuffer(gt);
	UpdateMainPassCB(gt);
}

void App::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	// Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
	// set as a root descriptor.
	auto matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

	// Bind the sky cube map.  For our demos, we just use one "world" cube map representing the environment
	// from far away, so all objects will use the same cube map and we only need to set it once per-frame.  
	// If we wanted to use "local" cube maps, we would have to change them per-object, or dynamically
	// index into an array of cube maps.

	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(mSkyTexHeapIndex, mCbvSrvDescriptorSize);
	mCommandList->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

	// Bind all the textures used in this scene.  Observe
	// that we only have to specify the first descriptor in the table.  
	// The root signature knows how many descriptors are expected in the table.
	mCommandList->SetGraphicsRootDescriptorTable(4, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["sky"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void App::OnMouseMove(WPARAM btnState, int x, int y)
{
	deltaMousePos.x = x - mLastMousePos.x;
	deltaMousePos.y = y - mLastMousePos.y;

	if (abs(deltaMousePos.x) < 100 && abs(deltaMousePos.y) < 100) {
		relCurrMousePos.x += deltaMousePos.x;
		int deltaX = relCurrMousePos.x - relLastMousePos.x;
		float dx = XMConvertToRadians(0.25f * static_cast<float>(deltaX));
		mCamera.RotateY(dx);
		relLastMousePos.x = relCurrMousePos.x;

		relCurrMousePos.y += deltaMousePos.y;
		int deltaY = relCurrMousePos.y - relLastMousePos.y;
		float dy = XMConvertToRadians(0.25f * static_cast<float>(deltaY));
		mCamera.Pitch(dy);
		relLastMousePos.y = relCurrMousePos.y;
		PhysicsEntity* entPhys = FindEnt("player")->GetPhysHolder();
		entPhys->setAngle(mCamera.getAngle());
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
	//SetCursorPos(mClientWidth/2, mClientHeight/2);
}

bool App::collisionCheck(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax, XMMATRIX& firstboxworld, XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, XMMATRIX& secondboxworld)
{

	//std::wostringstream ss;
	//ss << XMVectorGetX(firstboxmin) << " " << XMVectorGetX(secondboxmin)<< std::endl;
	//ss << "Firstbox "<< firstEntity.getCenter().x << " " << firstEntity.getCenter().y << " " << firstEntity.getCenter().z << std::endl;
	//ss << "Secondbox " << secondEntity.getCenter().x << " " << secondEntity.getCenter().y << " " << secondEntity.getCenter().z << std::endl;
	//ss << "Firstbox " << XMVectorGetX(firstboxmin) << " " << XMVectorGetY(firstboxmin) << " " << XMVectorGetZ(firstboxmin) << std::endl;
	//ss << "Secondbox " << XMVectorGetX(secondboxmin) << " " << XMVectorGetY(secondboxmin) << " " << XMVectorGetZ(secondboxmin) << std::endl;
	//ss << "normal " <<normal.x << " " << normal.y << " " << normal.z << std::endl;
	//ss << std::endl;
	//OutputDebugString(ss.str().c_str());

	//Is obj1's max X greater than obj2's min X? If not, obj1 is to the LEFT of obj2
	if (XMVectorGetX(firstboxmax) > XMVectorGetX(secondboxmin)) {
		//Is obj1's min X less than obj2's max X? If not, obj1 is to the RIGHT of obj2
		if (XMVectorGetX(firstboxmin) < XMVectorGetX(secondboxmax)) {
			//Is obj1's max Y greater than obj2's min Y? If not, obj1 is UNDER obj2
			if (XMVectorGetY(firstboxmax) > XMVectorGetY(secondboxmin)) {
				//Is obj1's min Y less than obj2's max Y? If not, obj1 is ABOVE obj2
				if (XMVectorGetY(firstboxmin) < XMVectorGetY(secondboxmax)) {
					//Is obj1's max Z greater than obj2's min Z? If not, obj1 is IN FRONT OF obj2
					if (XMVectorGetZ(firstboxmax) > XMVectorGetZ(secondboxmin)) {
						//Is obj1's min Z less than obj2's max Z? If not, obj1 is BEHIND obj2
						if (XMVectorGetZ(firstboxmin) < XMVectorGetZ(secondboxmax)) {
							//If we've made it this far, then the two bounding boxes are colliding
							return true;
						}
					}
				}
			}
		}
	}

	//If the two bounding boxes are not colliding, then return false
	//OutputDebugString(L"No collision \n");
	return false;
}

void App::calcAABB(std::vector<XMFLOAT3> boxVerts, XMFLOAT4X4& worldspace, XMVECTOR& boxmin, XMVECTOR& boxmax)
{
	//OutputDebugString(L"Calculating AABB");
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	XMMATRIX world = XMLoadFloat4x4(&worldspace);
	//Loop through the 8 vertices describing the bounding box
	for (UINT i = 0; i < 8; i++)
	{
		//Transform the bounding boxes vertices to the objects world space
		XMVECTOR Vert = XMVectorSet(boxVerts[i].x, boxVerts[i].y, boxVerts[i].z, 0.0f);
		Vert = XMVector3TransformCoord(Vert, world);

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x, XMVectorGetX(Vert));    // Find smallest x value in model
		minVertex.y = min(minVertex.y, XMVectorGetY(Vert));    // Find smallest y value in model
		minVertex.z = min(minVertex.z, XMVectorGetZ(Vert));    // Find smallest z value in model

		//Get the largest vertex 
		maxVertex.x = max(maxVertex.x, XMVectorGetX(Vert));    // Find largest x value in model
		maxVertex.y = max(maxVertex.y, XMVectorGetY(Vert));    // Find largest y value in model
		maxVertex.z = max(maxVertex.z, XMVectorGetZ(Vert));    // Find largest z value in model
	}

	//Store Bounding Box's min and max vertices
	boxmin = XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
	boxmax = XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);
}

void App::CreateBoundingVolumes(std::vector<GeometryGenerator::Vertex>& vertPosArray, std::vector<XMFLOAT3>& boundingBoxVerts, std::vector<DWORD>& boundingBoxIndex)
{
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (UINT i = 0; i < vertPosArray.size(); i++)
	{
		// The minVertex and maxVertex will most likely not be actual vertices in the model, but vertices
		// that use the smallest and largest x, y, and z values from the model to be sure ALL vertices are
		// covered by the bounding volume

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x, vertPosArray[i].Position.x);    // Find smallest x value in model
		minVertex.y = min(minVertex.y, vertPosArray[i].Position.y);    // Find smallest y value in model
		minVertex.z = min(minVertex.z, vertPosArray[i].Position.z);    // Find smallest z value in model

		//Get the largest vertex 
		maxVertex.x = max(maxVertex.x, vertPosArray[i].Position.x);    // Find largest x value in model
		maxVertex.y = max(maxVertex.y, vertPosArray[i].Position.y);    // Find largest y value in model
		maxVertex.z = max(maxVertex.z, vertPosArray[i].Position.z);    // Find largest z value in model
	}

	// Compute distance between maxVertex and minVertex
	float distX = (maxVertex.x - minVertex.x) / 2.0f;
	float distY = (maxVertex.y - minVertex.y) / 2.0f;
	float distZ = (maxVertex.z - minVertex.z) / 2.0f;

	// Create bounding box    
	// Front Vertices
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, minVertex.y, minVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z));

	// Back Vertices
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z));

	DWORD* i = new DWORD[36];

	// Front Face
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Back Face
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Top Face
	i[12] = 1; i[13] = 7; i[14] = 6;
	i[15] = 1; i[16] = 6; i[17] = 2;

	// Bottom Face
	i[18] = 0; i[19] = 4; i[20] = 5;
	i[21] = 0; i[22] = 5; i[23] = 3;

	// Left Face
	i[24] = 4; i[25] = 7; i[26] = 1;
	i[27] = 4; i[28] = 1; i[29] = 0;

	// Right Face
	i[30] = 3; i[31] = 2; i[32] = 6;
	i[33] = 3; i[34] = 6; i[35] = 5;

	for (int j = 0; j < 36; j++)
		boundingBoxIndex.push_back(i[j]);
}

void App::CreateBoundingVolumes(std::vector<Vertex>& vertPosArray, std::vector<XMFLOAT3>& boundingBoxVerts, std::vector<int32_t>& boundingBoxIndex)
{
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (UINT i = 0; i < vertPosArray.size(); i++)
	{
		// The minVertex and maxVertex will most likely not be actual vertices in the model, but vertices
		// that use the smallest and largest x, y, and z values from the model to be sure ALL vertices are
		// covered by the bounding volume

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x, vertPosArray[i].Pos.x);    // Find smallest x value in model
		minVertex.y = min(minVertex.y, vertPosArray[i].Pos.y);    // Find smallest y value in model
		minVertex.z = min(minVertex.z, vertPosArray[i].Pos.z);    // Find smallest z value in model

		//Get the largest vertex 
		maxVertex.x = max(maxVertex.x, vertPosArray[i].Pos.x);    // Find largest x value in model
		maxVertex.y = max(maxVertex.y, vertPosArray[i].Pos.y);    // Find largest y value in model
		maxVertex.z = max(maxVertex.z, vertPosArray[i].Pos.z);    // Find largest z value in model
	}

	// Compute distance between maxVertex and minVertex
	float distX = (maxVertex.x - minVertex.x) / 2.0f;
	float distY = (maxVertex.y - minVertex.y) / 2.0f;
	float distZ = (maxVertex.z - minVertex.z) / 2.0f;

	// Create bounding box    
	// Front Vertices
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, minVertex.y, minVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z));

	// Back Vertices
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z));
	boundingBoxVerts.push_back(XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z));

	DWORD* i = new DWORD[36];

	// Front Face
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Back Face
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Top Face
	i[12] = 1; i[13] = 7; i[14] = 6;
	i[15] = 1; i[16] = 6; i[17] = 2;

	// Bottom Face
	i[18] = 0; i[19] = 4; i[20] = 5;
	i[21] = 0; i[22] = 5; i[23] = 3;

	// Left Face
	i[24] = 4; i[25] = 7; i[26] = 1;
	i[27] = 4; i[28] = 1; i[29] = 0;

	// Right Face
	i[30] = 3; i[31] = 2; i[32] = 6;
	i[33] = 3; i[34] = 6; i[35] = 5;

	for (int j = 0; j < 36; j++)
		boundingBoxIndex.push_back(i[j]);
}

XMFLOAT3 App::makeCeil(XMFLOAT3 first, XMFLOAT3 second)
{
	//XMFLOAT3 result;
	if (second.x > first.x) first.x = second.x;
	if (second.y > first.y) first.y = second.y;
	if (second.z > first.z) first.z = second.z;
	return first;
}

XMFLOAT3 App::makeFloor(XMFLOAT3 first, XMFLOAT3 second)
{
	if (second.x < first.x) first.x = second.x;
	if (second.y < first.y) first.y = second.y;
	if (second.z < first.z) first.z = second.z;
	return first;
}

void App::BuildEnt(string name, XMFLOAT3 pos, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 look) {
	ents.insert(make_pair(name, new Entity{ pos, right, up, look }));
}

void App::BuildEnt(string name)
{
	ents.insert(make_pair(name, new Entity()));
}

Entity* App::FindEnt(string name) {
	return ents.find(name)->second;
}

void App::collision(string ent, XMFLOAT3& curPos, float dt) {

	std::map<string, Entity*>::iterator it = ents.begin();
	while (it != ents.end()) {

		//OutputDebugStringA(it->first.c_str());
		//OutputDebugString(L"\n");

		if (Physics::collisionCheck(FindEnt(ent), FindEnt(it->first)) && ent != it->first) {

			Physics::handleCollision(FindEnt(ent), FindEnt(it->first), curPos, dt);

			XMMATRIX boxRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
			XMMATRIX boxScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
			XMMATRIX boxOffset = XMMatrixTranslation(curPos.x, curPos.y, curPos.z);
			if (it->first.compare("road") == 0) {
				XMFLOAT3 newPos = FindEnt(ent)->GetStartPosition3f();
				boxOffset = XMMatrixTranslation(newPos.x - pos.x, newPos.y - pos.y, newPos.z - pos.z);
				FindEnt(ent)->returnToStart();
				mCamera.SetPosition(FindEnt(ent)->getHPos());
				mCamera.UpdateViewMatrix();
				pos = newPos;
			}

			XMMATRIX boxWorld = boxRotate * boxScale * boxOffset;

			XMStoreFloat4x4(&firstbox->World, boxWorld);
			XMStoreFloat4x4(&FindEnt(ent)->World, boxWorld);

			FindEnt(ent)->calcAABB(boxBoundingVertPosArray);
			calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);

			if (it->first.compare("endplatform") == 0 && gameClient->winner == 0) {
				if (!sPMode) {
					gameClient->sendToServerWin();
					endTime = mTimer.TotalTime();
				}
				else {
					gameClient->winner = 1;
					gameClient->gameActive = false;
					endTime = mTimer.TotalTime();
				}
			}

			// handling skull collisions
			for (int i = 0; i < skullCount; i++) { 
				string entname = "skull" + std::to_string(i);
				if (it->first.compare(entname) == 0) {
					XMFLOAT3 newPos = FindEnt(ent)->GetStartPosition3f();
					boxOffset = XMMatrixTranslation(newPos.x - pos.x, newPos.y - pos.y, newPos.z - pos.z);
					FindEnt(ent)->returnToStart();
					mCamera.SetPosition(FindEnt(ent)->getHPos());
					mCamera.UpdateViewMatrix();
					pos = newPos;

					XMMATRIX boxWorld = boxRotate * boxScale * boxOffset;
					XMStoreFloat4x4(&firstbox->World, boxWorld);
					XMStoreFloat4x4(&FindEnt(ent)->World, boxWorld);

					FindEnt(ent)->calcAABB(boxBoundingVertPosArray);
					calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);

				}
			
			}

		}
		else {
			//OutputDebugString(L"No collision \n");
		}
		it++;
	}
}

void App::OnKeyboardInput(const GameTimer& gt)
{
	if (gameClient->gameActive) {
		const float dt = gt.DeltaTime();
		PhysicsEntity* entPhys = FindEnt("player")->GetPhysHolder();

		float maxSpeed = 15.0f * dt;
		float boxSpeedX = maxSpeed,
			boxSpeedZ = maxSpeed;
		bool moveZ = false, moveX = false;

		//if (GetAsyncKeyState('Q') & 0x8000) {
		//	entPhys->setAngleNegative();
		//}
		//if (GetAsyncKeyState('E') & 0x8000) {
		//	entPhys->setAnglePositive();
		//	//keyboardInput.y -= boxSpeed;
		//}

		if (GetAsyncKeyState('W') & 0x8000) {
			entPhys->setZIntentPositive();
			moveZ = true;
			//keyboardInput.z += boxSpeed;
		}
		if (GetAsyncKeyState('S') & 0x8000) {
			entPhys->setZIntentNegative();
			moveZ = true;
			//keyboardInput.z -= boxSpeed;
		}
		if (GetAsyncKeyState('A') & 0x8000) {
			entPhys->setXIntentNegative();
			moveX = true;
			//keyboardInput.x -= boxSpeed;
		}
		if (GetAsyncKeyState('D') & 0x8000) {
			entPhys->setXIntentPositive();
			moveX = true;
			//keyboardInput.x += boxSpeed;
		}
		if (GetAsyncKeyState(' ') & 0x8000) {
			entPhys->decrementJump();
		}

		if (moveZ) {
			float tempT = FindEnt("player")->getCountDownZ() - decel;
			if (tempT > 0) {
				FindEnt("player")->setCountDownZ(tempT);
			}
			else {
				FindEnt("player")->resetCountDownZ(true);
			}
			boxSpeedZ *= cos(FindEnt("player")->getCountDownZ());
		}
		else { FindEnt("player")->resetCountDownZ(false); }

		if (moveX) {
			float tempT = FindEnt("player")->getCountDownX() - decel;
			if (tempT > 0) {
				FindEnt("player")->setCountDownX(tempT);
			}
			else {
				FindEnt("player")->resetCountDownX(true);
			}
			boxSpeedX *= cos(FindEnt("player")->getCountDownX());
		}
		else { FindEnt("player")->resetCountDownX(false); }

		//box translation//
		XMMATRIX boxRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
		XMMATRIX boxScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMMATRIX boxOffset = XMMatrixTranslation(pos.x, pos.y, pos.z);
		XMMATRIX boxWorld = boxRotate * boxScale * boxOffset;

		firstbox->Geo;

		XMStoreFloat4x4(&FindEnt("player")->World, boxWorld);
		XMStoreFloat4x4(&firstbox->World, boxWorld);

		FindEnt("player")->calcAABB(boxBoundingVertPosArray);

		//calculate new bounding box of first box
		calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);

		//Physics::XYZPhysics(pos, entPhys, boxSpeed);
		Physics::XYZPhysics(pos, entPhys, 5.0f * dt, boxSpeedX, boxSpeedZ);
		FindEnt("player")->SetPosition(pos);

		collision("player", pos, dt);
		//FindEnt("player")->SetPosition(pos);

		if (!sPMode) {
			gameClient->sendToServer(pos.x, pos.y, pos.z);
		}


		//formerly mboxritemmovable
		firstbox->NumFramesDirty++;

	}

	if (!isTopDown) {
		mCamera.SetPosition(FindEnt("player")->getHPos());
	}
	mCamera.UpdateViewMatrix();
}


void App::UpdatePlayer2(const GameTimer& gt) {
	FindEnt("block")->World = secondbox->World;
	FindEnt("block")->calcAABB(boxBoundingVertPosArray);
	calcAABB(boxBoundingVertPosArray, secondbox->World, secondbox->boundingboxminvertex, secondbox->boundingboxmaxvertex);
	secondbox->NumFramesDirty++;
}

void App::MoveSkulls(const GameTimer& gt) {
	const float dt = gt.DeltaTime();
	for (auto& e : mAllRitems) {
		if (e->ObjCBIndex >= skullCbIndexStart && e->ObjCBIndex < skullCbIndexMax) {
			UINT curIndex = e->ObjCBIndex - skullCbIndexStart;
			string entname = "skull" + std::to_string(curIndex);
			XMMATRIX world = XMLoadFloat4x4(&e->World);

			if (skullControllers[curIndex].isInRange(FindEnt("player"))) {
				Entity* target = skullControllers[curIndex].CalcClosest(FindEnt("player"), nullptr);
				XMFLOAT3 skullMov = skullControllers[curIndex].CalcMove(target);
				XMMATRIX skullOffset = XMMatrixTranslation(skullMov.x * dt, skullMov.y * dt, skullMov.z * dt);

				XMMATRIX skullWorld = world * skullOffset;

				XMStoreFloat4x4(&e->World, skullWorld);
				XMStoreFloat4x4(&FindEnt(entname)->World, skullWorld);
			}
			else {
				XMStoreFloat4x4(&e->World, world);
				XMStoreFloat4x4(&FindEnt(entname)->World, world);
			}

			FindEnt(entname)->calcAABB(skullBoundingVertPosArray);
			e->NumFramesDirty++;
		}
	}
}

void App::MoveCars(const GameTimer& gt) {
	const float dt = gt.DeltaTime();
	for (auto& e : mAllRitems) {
		XMFLOAT3 mov = { 0.0f, 0.0f, 3.0f * dt };
		if (e->ObjCBIndex >= carsCBIndexStart && e->ObjCBIndex < carsCBIndexStart + carCount) {
			string entname = "block" + std::to_string(e->ObjCBIndex);
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			if (e->ObjCBIndex < carsCBIndexStart + carCount / 3 || e->ObjCBIndex >= carsCBIndexStart + (2 * carCount / 3)) {
				mov = { 0.0f, 0.0f, -3.0f * dt };
			}
			XMMATRIX boxOffset = XMMatrixTranslation(mov.x, mov.y, mov.z);
			XMMATRIX boxWorld = world * boxOffset;
			if (e->World(3, 2) > (20 * 8.0f)) {
				XMMATRIX transl = XMMatrixTranslation(0.0f, -0.8f, 0.0f);
				XMMATRIX resetPos = XMMatrixScaling(0.5f, 0.5f, 0.5f) * transl * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f);
				XMStoreFloat4x4(&e->World, resetPos);
				XMStoreFloat4x4(&FindEnt(entname)->World, resetPos);
			}
			else if (e->World(3, 2) < 0) {
				XMMATRIX transl = XMMatrixTranslation(5.0f, -0.8f, 0.0f);
				if (e->ObjCBIndex < carsCBIndexStart + carCount / 3)transl = XMMatrixTranslation(5.0f, -0.8f, 20 * 8.0f);
				if (e->ObjCBIndex >= carsCBIndexStart + (2 * carCount / 3))transl = XMMatrixTranslation(-5.0f, -0.8f, 20 * 8.0f);
				XMMATRIX resetPos = XMMatrixScaling(0.5f, 0.5f, 0.5f) * transl;// *XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f);
				XMStoreFloat4x4(&e->World, resetPos);
				XMStoreFloat4x4(&FindEnt(entname)->World, resetPos);
			}
			else {
				XMStoreFloat4x4(&e->World, boxWorld);
				XMStoreFloat4x4(&FindEnt(entname)->World, boxWorld);
			}
			////OutputDebugStringA(entname.c_str());
			////OutputDebugString(L"\n");
			FindEnt(entname)->calcAABB(carBoundingVertPosArray);
			e->NumFramesDirty++;
		}
	}
}

void App::AnimateMaterials(const GameTimer& gt)
{
}

void App::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();

	/*auto& player = mAllRitems.at(0);
	XMMATRIX*/

	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
			objConstants.MaterialIndex = e->Mat->MatCBIndex;

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void App::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void App::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
	mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };
	if (!gameClient->gameActive || gameClient->winner < 0) {
		mMainPassCB.Lights[0].Strength = { 1.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[1].Strength = { 1.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[2].Strength = { 1.0f, 0.0f, 0.0f };
	}
	if (gameClient->winner > 0) {
		mMainPassCB.Lights[0].Strength = { 0.0f, 1.0f, 0.0f };
		mMainPassCB.Lights[1].Strength = { 0.0f, 1.0f, 0.0f };
		mMainPassCB.Lights[2].Strength = { 0.0f, 1.0f, 0.0f };
	}
	for (int i = 3; i < 6; ++i) {
		//mMainPassCB.Lights[i].Direction = { 0.0f, -5.0f, -10.0f };
		mMainPassCB.Lights[i].Strength = { 1.0f, 0.0f, 0.0f };
		mMainPassCB.Lights[i].Position = { ((i - 3) * 5.0f) - 5.0f, 1.0f, 2.6f * 59 };
		//mMainPassCB.Lights[i].FalloffStart = 0.5f;
		mMainPassCB.Lights[i].FalloffEnd = 20.0f;
		//mMainPassCB.Lights[i].SpotPower = 15.0f;
	}

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}


void App::LoadTextures()
{
	std::vector<std::string> texNames =
	{
		"bricksDiffuseMap",
		"tileDiffuseMap",
		"defaultDiffuseMap",
		"skyCubeMap"
	};

	std::vector<std::wstring> texFilenames =
	{
		L"../../Textures/purplegems.dds",
		L"../../Textures/tile.dds",
		L"../../Textures/concreteroad.dds",
		L"../../Textures/snowcube1024.dds"
	};

	for (int i = 0; i < (int)texNames.size(); ++i)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->Name = texNames[i];
		texMap->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));

		mTextures[texMap->Name] = std::move(texMap);
	}
}

void App::SetupClientServer() {
	while (true) {
		if (GetAsyncKeyState('1') & 0x8000) {
			gameServer = new Server();
			gameClient = new Client();
			isHost = true;
			break;
		}
		if (GetAsyncKeyState('2') & 0x8000) {
			gameClient = new Client();
			break;
		}
		if (GetAsyncKeyState('3') & 0x8000) {
			gameClient = new Client();
			sPMode = true;
			break;
		}
	}
}

void App::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 5;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto bricksTex = mTextures["bricksDiffuseMap"]->Resource;
	auto tileTex = mTextures["tileDiffuseMap"]->Resource;
	auto whiteTex = mTextures["defaultDiffuseMap"]->Resource;
	auto skyTex = mTextures["skyCubeMap"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = bricksTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = bricksTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = tileTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = tileTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = whiteTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = whiteTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(whiteTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyTex->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(skyTex.Get(), &srvDesc, hDescriptor);

	mSkyTexHeapIndex = 3;
}


void App::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsShaderResourceView(0, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);


	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void App::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void App::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	box = geoGen.CreateBox(0.5f, 0.5f, 0.5f, 3);
	GeometryGenerator::MeshData box2 = geoGen.CreateBox(0.5f, 0.5f, 0.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(15.0f, 5.0f, 30, 20);
	road = grid;
	GeometryGenerator::MeshData tunnel = geoGen.CreateTunnel(15.0f, 2.2f, 10.0f, 3);
	platform = tunnel;
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT box2VertexOffset = (UINT)box.Vertices.size();
	UINT gridVertexOffset = box2VertexOffset + (UINT)box2.Vertices.size();
	UINT tunnelVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT sphereVertexOffset = tunnelVertexOffset + (UINT)tunnel.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT box2IndexOffset = (UINT)box.Indices32.size();
	UINT gridIndexOffset = box2IndexOffset + (UINT)box2.Indices32.size();
	UINT tunnelIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT sphereIndexOffset = tunnelIndexOffset + (UINT)tunnel.Indices32.size();
	// Define the SubmeshGeometry that cover different 
	// regions of the vertex/index buffers.

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry box2Submesh;
	box2Submesh.IndexCount = (UINT)box2.Indices32.size();
	box2Submesh.StartIndexLocation = box2IndexOffset;
	box2Submesh.BaseVertexLocation = box2VertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry tunnelSubmesh;
	tunnelSubmesh.IndexCount = (UINT)tunnel.Indices32.size();
	tunnelSubmesh.StartIndexLocation = tunnelIndexOffset;
	tunnelSubmesh.BaseVertexLocation = tunnelVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	auto totalVertexCount =
		box.Vertices.size() + box2.Vertices.size() +
		grid.Vertices.size() + tunnel.Vertices.size() + sphere.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < box2.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box2.Vertices[i].Position;
		vertices[k].Normal = box2.Vertices[i].Normal;
		vertices[k].TexC = box2.Vertices[i].TexC;
	}
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
	}
	for (size_t i = 0; i < tunnel.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = tunnel.Vertices[i].Position;
		vertices[k].Normal = tunnel.Vertices[i].Normal;
		vertices[k].TexC = tunnel.Vertices[i].TexC;
	}
	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
	}


	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(box2.GetIndices16()), std::end(box2.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(tunnel.GetIndices16()), std::end(tunnel.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["box2"] = box2Submesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["tunnel"] = tunnelSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

//void App::BuildSkyBoxGeometry()
//{
//	GeometryGenerator geoGen;
//	GeometryGenerator::MeshData skyBox = geoGen.CreateBox(100.0f, 100.0f, 100.0f, 3);
//
//	UINT skyBoxVertexOffset = 0;
//
//	UINT skyBoxIndexOffset = 0;
//
//	SubmeshGeometry skyBoxSubmesh;
//	skyBoxSubmesh.IndexCount = (UINT)skyBox.Indices32.size();
//	skyBoxSubmesh.StartIndexLocation = skyBoxIndexOffset;
//	skyBoxSubmesh.BaseVertexLocation = skyBoxVertexOffset;
//
//	auto totalVertexCount =
//		skyBox.Vertices.size();
//
//	std::vector<Vertex> vertices(totalVertexCount);
//
//	UINT k = 0;
//	for (size_t i = 0; i < skyBox.Vertices.size(); ++i, ++k)
//	{
//		vertices[k].Pos = skyBox.Vertices[i].Position;
//		vertices[k].Color = XMFLOAT4(DirectX::Colors::SkyBlue);
//	}
//
//	std::vector<std::uint16_t> indices;
//	indices.insert(indices.end(), std::begin(skyBox.GetIndices16()), std::end(skyBox.GetIndices16()));
//
//	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
//	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
//
//	auto geo = std::make_unique<MeshGeometry>();
//	geo->Name = "skyBoxGeo";
//
//	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
//	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
//
//	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
//	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
//
//	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
//
//	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
//		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
//
//	geo->VertexByteStride = sizeof(Vertex);
//	geo->VertexBufferByteSize = vbByteSize;
//	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
//	geo->IndexBufferByteSize = ibByteSize;
//
//	geo->DrawArgs["skyBox"] = skyBoxSubmesh;
//
//	mGeometries[geo->Name] = std::move(geo);
//}


void App::BuildplatformGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData platform = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);

	UINT platformVertexOffset = 0;

	UINT platformIndexOffset = 0;

	SubmeshGeometry platformSubmesh;
	platformSubmesh.IndexCount = (UINT)platform.Indices32.size();
	platformSubmesh.StartIndexLocation = platformIndexOffset;
	platformSubmesh.BaseVertexLocation = platformVertexOffset;

	auto totalVertexCount =
		platform.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < platform.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = platform.Vertices[i].Position;
		vertices[k].Normal = platform.Vertices[i].Normal;
		vertices[k].TexC = platform.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(platform.GetIndices16()), std::end(platform.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "platformGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["platform"] = platformSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void App::BuildTruckGeometry()
{
	std::ifstream fin("Models/car.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/car.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		//fin >> vertices[i].TexC.x >> vertices[i].TexC.y;
		/*vertices[i].TexC.x = vertices[i].Pos.x;
		vertices[i].TexC.y = vertices[i].Pos.y;*/
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "semitruckGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["semitruck"] = submesh;


	mGeometries[geo->Name] = std::move(geo);

	CreateBoundingVolumes(vertices, carBoundingVertPosArray, indices);

}

void App::BuildSkullGeometry()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::int32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skullGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["skull"] = submesh;

	mGeometries[geo->Name] = std::move(geo);

	CreateBoundingVolumes(vertices, skullBoundingVertPosArray, indices);
}

void App::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for sky.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;

	// The camera is inside the sky sphere, so just turn off culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = mRootSignature.Get();
	skyPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyVS"]->GetBufferPointer()),
		mShaders["skyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["skyPS"]->GetBufferPointer()),
		mShaders["skyPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));
}

void App::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
	}
}

void App::BuildMaterials()
{
	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = 0;
	bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks0->Roughness = 0.3f;

	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = 1;
	tile0->DiffuseSrvHeapIndex = 1;
	tile0->DiffuseAlbedo = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	tile0->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	tile0->Roughness = 1.0f;

	auto mirror0 = std::make_unique<Material>();
	mirror0->Name = "mirror0";
	mirror0->MatCBIndex = 2;
	mirror0->DiffuseSrvHeapIndex = 2;
	mirror0->DiffuseAlbedo = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mirror0->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	mirror0->Roughness = 1.0f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 3;
	skullMat->DiffuseSrvHeapIndex = 2;
	skullMat->DiffuseAlbedo = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	skullMat->Roughness = 0.2f;

	auto sky = std::make_unique<Material>();
	sky->Name = "sky";
	sky->MatCBIndex = 4;
	sky->DiffuseSrvHeapIndex = 3;
	sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	sky->Roughness = 1.0f;

	mMaterials["bricks0"] = std::move(bricks0);
	mMaterials["tile0"] = std::move(tile0);
	mMaterials["mirror0"] = std::move(mirror0);
	mMaterials["skullMat"] = std::move(skullMat);
	mMaterials["sky"] = std::move(sky);
}

void App::BuildRenderItems()
{
	XMMATRIX box1Translation;
	XMMATRIX box2Translation;
	UINT objCBIndex = 0;

	auto skyRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skyRitem->TexTransform = MathHelper::Identity4x4();
	skyRitem->ObjCBIndex = objCBIndex++;
	skyRitem->Mat = mMaterials["sky"].get();
	skyRitem->Geo = mGeometries["shapeGeo"].get();
	skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sphere"].IndexCount;
	skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
	skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Sky].push_back(skyRitem.get());
	mAllRitems.push_back(std::move(skyRitem));

	if (isHost) {
		//can probably remove the translations because we're using pos global
		box1Translation = XMMatrixTranslation(-2.5f, 2.0f, 0.0f);
		box2Translation = XMMatrixTranslation(2.5f, 2.0f, 0.0f);
		pos = { -2.5f, 2.0f, 0.0f };
	}
	else {
		//can probably remove the translations because we're using pos global
		box1Translation = XMMatrixTranslation(2.5f, 2.0f, 0.0f);
		box2Translation = XMMatrixTranslation(-2.5f, 2.0f, 0.0f);
		pos = { 2.5f, 2.0f, 0.0f };
	}
	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * box1Translation);
	XMStoreFloat4x4(&FindEnt("player")->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * box1Translation);
	XMStoreFloat4x4(&boxRitem->TexTransform, XMMatrixScaling(2.0f, 2.0f, 2.0f));
	boxRitem->ObjCBIndex = objCBIndex++;
	boxRitem->Mat = mMaterials["bricks0"].get();
	boxRitem->Geo = mGeometries["shapeGeo"].get();
	boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
	firstbox = boxRitem.get();

	FindEnt("player")->calcAABB(boxBoundingVertPosArray);
	FindEnt("player")->SetPosition(pos);
	mCamera.SetPosition(FindEnt("player")->getHPos());
	mCamera.UpdateViewMatrix();
	FindEnt("player")->SetPositionStart();
	calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);

	auto boxRitem2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem2->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * box2Translation);
	XMStoreFloat4x4(&FindEnt("block")->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * box2Translation);
	XMStoreFloat4x4(&boxRitem2->TexTransform, XMMatrixScaling(2.0f, 2.0f, 2.0f));
	boxRitem2->ObjCBIndex = objCBIndex++;
	boxRitem2->Mat = mMaterials["bricks0"].get();
	boxRitem2->Geo = mGeometries["shapeGeo"].get();
	boxRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem2->IndexCount = boxRitem2->Geo->DrawArgs["box2"].IndexCount;
	boxRitem2->StartIndexLocation = boxRitem2->Geo->DrawArgs["box2"].StartIndexLocation;
	boxRitem2->BaseVertexLocation = boxRitem2->Geo->DrawArgs["box2"].BaseVertexLocation;
	secondbox = boxRitem2.get();
	//OutputDebugString(L"CalcAABB of block entity\n");
	FindEnt("block")->calcAABB(boxBoundingVertPosArray);
	calcAABB(boxBoundingVertPosArray, secondbox->World, secondbox->boundingboxminvertex, secondbox->boundingboxmaxvertex);

	gameClient->setPlayer(boxRitem2.get());

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem.get());
	mAllRitems.push_back(std::move(boxRitem));
	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem2.get());
	mAllRitems.push_back(std::move(boxRitem2));



	BuildEnt("startplatform");

	auto startTunnelRitem = std::make_unique<RenderItem>();
	startTunnelRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&FindEnt("startplatform")->World, XMMatrixTranslation(0.0f, -1.1f, 0.0f));
	XMStoreFloat4x4(&startTunnelRitem->World, XMMatrixTranslation(0.0f, -1.1f, 0.0f));
	XMStoreFloat4x4(&startTunnelRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 5.0f));
	startTunnelRitem->ObjCBIndex = objCBIndex++;
	startTunnelRitem->Mat = mMaterials["tile0"].get();
	startTunnelRitem->Geo = mGeometries["shapeGeo"].get();
	startTunnelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	startTunnelRitem->IndexCount = startTunnelRitem->Geo->DrawArgs["tunnel"].IndexCount;
	startTunnelRitem->StartIndexLocation = startTunnelRitem->Geo->DrawArgs["tunnel"].StartIndexLocation;
	startTunnelRitem->BaseVertexLocation = startTunnelRitem->Geo->DrawArgs["tunnel"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(startTunnelRitem.get());
	mAllRitems.push_back(std::move(startTunnelRitem));

	FindEnt("startplatform")->calcAABB(platformBoundingVertPosArray);

	BuildEnt("road");
	auto roadRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&FindEnt("road")->World, XMMatrixScaling(1.0f, 1.0f, 32.0f) * XMMatrixTranslation(0.0f, -2.0f, 80.0f));
	XMStoreFloat4x4(&roadRitem->World, XMMatrixScaling(1.0f, 1.0f, 32.0f) * XMMatrixTranslation(0.0f, -2.0f, 80.0f));
	XMStoreFloat4x4(&roadRitem->TexTransform, XMMatrixScaling(1.0f, 2.0f, 1.0f) * XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 3.14f / 2));
	roadRitem->ObjCBIndex = objCBIndex++;
	roadRitem->Mat = mMaterials["mirror0"].get();
	roadRitem->Geo = mGeometries["shapeGeo"].get();
	roadRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	roadRitem->IndexCount = roadRitem->Geo->DrawArgs["grid"].IndexCount;
	roadRitem->StartIndexLocation = roadRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	roadRitem->BaseVertexLocation = roadRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(roadRitem.get());
	mAllRitems.push_back(std::move(roadRitem));
	FindEnt("road")->calcAABB(roadBoundingVertPosArray);

	BuildEnt("endplatform");
	auto endTunnelRitem = std::make_unique<RenderItem>();
	endTunnelRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&FindEnt("endplatform")->World, XMMatrixTranslation(0.0f, -1.1f, -2.7f * 59) * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f));
	XMStoreFloat4x4(&endTunnelRitem->World, XMMatrixTranslation(0.0f, -1.1f, -2.7f * 59) * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f));
	XMStoreFloat4x4(&endTunnelRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	endTunnelRitem->ObjCBIndex = objCBIndex++;
	endTunnelRitem->Mat = mMaterials["tile0"].get();
	endTunnelRitem->Geo = mGeometries["shapeGeo"].get();
	endTunnelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	endTunnelRitem->IndexCount = endTunnelRitem->Geo->DrawArgs["tunnel"].IndexCount;
	endTunnelRitem->StartIndexLocation = endTunnelRitem->Geo->DrawArgs["tunnel"].StartIndexLocation;
	endTunnelRitem->BaseVertexLocation = endTunnelRitem->Geo->DrawArgs["tunnel"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(endTunnelRitem.get());
	mAllRitems.push_back(std::move(endTunnelRitem));
	FindEnt("endplatform")->calcAABB(platformBoundingVertPosArray);

	carsCBIndexStart = objCBIndex;
	for (int i = 0; i < carCount / 3; ++i) {
		auto platformRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&platformRitem->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-5.0f, -0.8f, (i * 12.0f)));
		XMStoreFloat4x4(&platformRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		platformRitem->ObjCBIndex = objCBIndex++;
		string entname = "block" + std::to_string(platformRitem->ObjCBIndex);
		platformRitem->Mat = mMaterials["bricks0"].get();
		platformRitem->Geo = mGeometries["semitruckGeo"].get();
		platformRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		platformRitem->IndexCount = platformRitem->Geo->DrawArgs["semitruck"].IndexCount;
		platformRitem->StartIndexLocation = platformRitem->Geo->DrawArgs["semitruck"].StartIndexLocation;
		platformRitem->BaseVertexLocation = platformRitem->Geo->DrawArgs["semitruck"].BaseVertexLocation;
		mRitemLayer[(int)RenderLayer::Opaque].push_back(platformRitem.get());
		mAllRitems.push_back(std::move(platformRitem));

		//code needed to check for collision between entities
		////OutputDebugStringA(entname.c_str());
		////OutputDebugString(L"\n");
		BuildEnt(entname);
		XMStoreFloat4x4(&FindEnt(entname)->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-5.0f, -0.8f, (i * 12.0f)) /** XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f)*/);
		FindEnt(entname)->calcAABB(carBoundingVertPosArray);

	}

	for (int i = 0; i < carCount / 3; ++i) {
		auto platformRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&platformRitem->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.0f, -0.8f, (i * -12.0f) - 4.0f) * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f));
		XMStoreFloat4x4(&platformRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		platformRitem->ObjCBIndex = objCBIndex++;
		string entname = "block" + std::to_string(platformRitem->ObjCBIndex);
		platformRitem->Mat = mMaterials["bricks0"].get();
		platformRitem->Geo = mGeometries["semitruckGeo"].get();
		platformRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		platformRitem->IndexCount = platformRitem->Geo->DrawArgs["semitruck"].IndexCount;
		platformRitem->StartIndexLocation = platformRitem->Geo->DrawArgs["semitruck"].StartIndexLocation;
		platformRitem->BaseVertexLocation = platformRitem->Geo->DrawArgs["semitruck"].BaseVertexLocation;
		mRitemLayer[(int)RenderLayer::Opaque].push_back(platformRitem.get());
		mAllRitems.push_back(std::move(platformRitem));
		//code needed to check for collision between entities
		//OutputDebugStringA(entname.c_str());
		////OutputDebugString(L"\n");
		BuildEnt(entname);
		XMStoreFloat4x4(&FindEnt(entname)->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(0.0f, -0.8f, (i * -12.0f) - 4.0f) * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f));
		FindEnt(entname)->calcAABB(carBoundingVertPosArray);

	}

	for (int i = 0; i < carCount / 3; ++i) {
		auto platformRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&platformRitem->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, -0.8f, (i * 12.0f)));
		XMStoreFloat4x4(&platformRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		platformRitem->ObjCBIndex = objCBIndex++;
		string entname = "block" + std::to_string(platformRitem->ObjCBIndex);
		platformRitem->Mat = mMaterials["bricks0"].get();
		platformRitem->Geo = mGeometries["semitruckGeo"].get();
		platformRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		platformRitem->IndexCount = platformRitem->Geo->DrawArgs["semitruck"].IndexCount;
		platformRitem->StartIndexLocation = platformRitem->Geo->DrawArgs["semitruck"].StartIndexLocation;
		platformRitem->BaseVertexLocation = platformRitem->Geo->DrawArgs["semitruck"].BaseVertexLocation;
		mRitemLayer[(int)RenderLayer::Opaque].push_back(platformRitem.get());
		mAllRitems.push_back(std::move(platformRitem));
		//code needed to check for collision between entities
		////OutputDebugStringA(entname.c_str());
		////OutputDebugString(L"\n");
		BuildEnt(entname);
		XMStoreFloat4x4(&FindEnt(entname)->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, -0.8f, (i * 12.0f)));
		FindEnt(entname)->calcAABB(carBoundingVertPosArray);
	}

	BuildEnt("car");
	auto platformRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&FindEnt("car")->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, 2.0f, 5.0f) * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f));
	XMStoreFloat4x4(&platformRitem->World, XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(5.0f, 2.0f, 5.0f) * XMMatrixRotationRollPitchYaw(0.0f, 3.14f, 0.0f));
	XMStoreFloat4x4(&platformRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	platformRitem->ObjCBIndex = objCBIndex++;
	platformRitem->Mat = mMaterials["bricks0"].get();
	platformRitem->Geo = mGeometries["semitruckGeo"].get();
	platformRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	platformRitem->IndexCount = platformRitem->Geo->DrawArgs["semitruck"].IndexCount;
	platformRitem->StartIndexLocation = platformRitem->Geo->DrawArgs["semitruck"].StartIndexLocation;
	platformRitem->BaseVertexLocation = platformRitem->Geo->DrawArgs["semitruck"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(platformRitem.get());
	mAllRitems.push_back(std::move(platformRitem));
	FindEnt("car")->calcAABB(carBoundingVertPosArray);

	skullCbIndexStart = objCBIndex;
	for (int i = 0; i < 5; ++i) {
		auto leftSkullRitem = std::make_unique<RenderItem>();
		auto rightSkullRitem = std::make_unique<RenderItem>();

		XMMATRIX leftSkullWorld = XMMatrixTranslation(-15.0f, -1.0f, 20.0f + i * 20.0f);
		XMMATRIX rightSkullWorld = XMMatrixTranslation(15.0f, -1.0f, 20.0f + i * 20.0f);
		XMMATRIX skullSize = XMMatrixScaling(0.3f, 0.3f, 0.3f);

		XMStoreFloat4x4(&leftSkullRitem->World, skullSize * leftSkullWorld);
		XMStoreFloat4x4(&leftSkullRitem->TexTransform, skullSize);
		leftSkullRitem->ObjCBIndex = objCBIndex++;
		leftSkullRitem->Mat = mMaterials["bricks0"].get();
		leftSkullRitem->Geo = mGeometries["skullGeo"].get();
		leftSkullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSkullRitem->IndexCount = leftSkullRitem->Geo->DrawArgs["skull"].IndexCount;
		leftSkullRitem->StartIndexLocation = leftSkullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
		leftSkullRitem->BaseVertexLocation = leftSkullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;

		XMStoreFloat4x4(&rightSkullRitem->World, skullSize * rightSkullWorld);
		XMStoreFloat4x4(&rightSkullRitem->TexTransform, skullSize);
		rightSkullRitem->ObjCBIndex = objCBIndex++;
		rightSkullRitem->Mat = mMaterials["bricks0"].get();
		rightSkullRitem->Geo = mGeometries["skullGeo"].get();
		rightSkullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSkullRitem->IndexCount = rightSkullRitem->Geo->DrawArgs["skull"].IndexCount;
		rightSkullRitem->StartIndexLocation = rightSkullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
		rightSkullRitem->BaseVertexLocation = rightSkullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(leftSkullRitem.get());
		mRitemLayer[(int)RenderLayer::Opaque].push_back(rightSkullRitem.get());
		mAllRitems.push_back(std::move(leftSkullRitem));
		mAllRitems.push_back(std::move(rightSkullRitem));

		//code needed to check for collision between entities
		string skullEntname = "skull" + std::to_string(2 * i);
		//OutputDebugStringA(skullEntname.c_str());
		//OutputDebugString(L"\n");
		BuildEnt(skullEntname);
		XMStoreFloat4x4(&FindEnt(skullEntname)->World, skullSize * leftSkullWorld);
		FindEnt(skullEntname)->SetPosition({ -15.0f, -1.0f, 20.0f + i * 20.0f });
		FindEnt(skullEntname)->SetPositionStart();
		SkullAI skullCtrl1(FindEnt(skullEntname));
		skullControllers.push_back(skullCtrl1);
		FindEnt(skullEntname)->calcAABB(skullBoundingVertPosArray);

		skullEntname = "skull" + std::to_string(2 * i + 1);
		BuildEnt(skullEntname);
		XMStoreFloat4x4(&FindEnt(skullEntname)->World, skullSize * rightSkullWorld);
		FindEnt(skullEntname)->SetPosition({ 15.0f, -1.0f, 20.0f + i * 20.0f });
		FindEnt(skullEntname)->SetPositionStart();
		SkullAI skullCtrl2(FindEnt(skullEntname));
		skullControllers.push_back(skullCtrl2);
		FindEnt(skullEntname)->calcAABB(skullBoundingVertPosArray);
	}
	skullCbIndexMax = objCBIndex;
}

void App::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}


std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> App::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}