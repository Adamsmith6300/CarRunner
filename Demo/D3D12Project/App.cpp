//***************************************************************************************
// App.cpp by Frank Luna (C) 2015 All Rights Reserved.
//
// Hold down '1' key to view scene in wireframe mode.
//***************************************************************************************


#include <thread> 
#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "../../Common/camera.h"
#include "../../Common/Entity.h"
#include "../../Physics/Physics.h"
#include "Server.h"
#include "Client.h"
#include "FrameResource.h"
#include "RenderItem.h"
#include <map>

#define ENTMAP map<string, Entity*>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

const int gNumFrameResources = 3;

//For Development only
const bool isTopDown = true;
XMFLOAT3 topPos = { 0.0f, 20.0f, 0.0f };


class App : public D3DApp
{
public:
    App(HINSTANCE hInstance);
    App(const App& rhs) = delete;
    App& operator=(const App& rhs) = delete;
    ~App();

    virtual bool Initialize()override;
	virtual int Run()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	bool collisionCheck(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax, XMMATRIX& firstboxworld, XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, XMMATRIX& secondboxworld);
	void calcAABB(std::vector<XMFLOAT3> boxVerts, XMFLOAT4X4& worldspace, XMVECTOR& boxmin, XMVECTOR& boxmax);
	void CreateBoundingVolumes(std::vector<GeometryGenerator::Vertex>& vertPosArray,std::vector<XMFLOAT3>& boundingBoxVerts, std::vector<DWORD>& boundingBoxIndex);
	void handleCollision(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax,XMFLOAT3& firsttranslation,XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, float velocity);
	XMFLOAT3 makeCeil(XMFLOAT3 first, XMFLOAT3 second);
	XMFLOAT3 makeFloor(XMFLOAT3 first, XMFLOAT3 second);

	void BuildEnt(string name, XMFLOAT3 pos, XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 look);
	Entity* FindEnt(string name);

    void BuildDescriptorHeaps();
    void BuildConstantBufferViews();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildShapeGeometry();
	void BuildSkyBoxGeometry();
	void BuildplatformGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	void SetupClientServer();
	void signalHandler(int signum);
private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
    RenderItem* mBoxItemMovable;
    XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 right = {pos.x+1, pos.y, pos.z};
    XMFLOAT3 up = { pos.x, pos.y+1, pos.z };
    XMFLOAT3 look = { pos.x, pos.y, pos.z+1 };
	ENTMAP ents = {};

	//global variables for the bounding box
	RenderItem* firstbox = nullptr;
	RenderItem* secondbox = nullptr;
	std::vector<XMFLOAT3> boxBoundingVertPosArray;
	std::vector<DWORD> boxBoundingVertIndexArray;
	GeometryGenerator::MeshData box;

	// Render items divided by PSO.
	std::vector<RenderItem*> mOpaqueRitems;

    PassConstants mMainPassCB;

    UINT mPassCbvOffset = 0;

    bool mIsWireframe = false;

	Camera mCamera;

    POINT mLastMousePos;
	Server* gameServer = nullptr;
	Client* gameClient = nullptr;
	thread clientThread;
	thread serverThread;
	bool isHost = false;
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
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
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
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}
int App::Run()
{
	MSG msg = { 0 };

	mTimer.Reset();

	clientThread = thread(&Client::start, gameClient);
	if (isHost) {
		serverThread = thread(&Server::start, gameServer);
	}

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

			/*if (!mAppPaused)
			{*/
				CalculateFrameStats();
				Update(mTimer);
				Draw(mTimer);
			//}
			/*else
			{
				Sleep(100);
			}*/
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
bool App::Initialize()
{
    if (isTopDown) {
        mCamera.SetPosition(topPos);
        mCamera.Pitch(MathHelper::Pi / 2);
    }
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	
	BuildEnt("player", pos, right, up, look);
	SetupClientServer();
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildShapeGeometry();
	BuildSkyBoxGeometry();
	BuildplatformGeometry();

	//for creating the necessary vertices for bounding boxes
	CreateBoundingVolumes(box.Vertices, boxBoundingVertPosArray, boxBoundingVertIndexArray);

	BuildRenderItems();
    BuildFrameResources();
    BuildDescriptorHeaps();
    BuildConstantBufferViews();
    BuildPSOs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}

void App::OnResize()
{
    D3DApp::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

}

void App::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);

	if (collisionCheck(firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex, XMLoadFloat4x4(&firstbox->World),
		secondbox->boundingboxminvertex, secondbox->boundingboxmaxvertex, XMLoadFloat4x4(&secondbox->World))) {
		//OutputDebugString(L"Collision\n");
	}
	else {
		//OutputDebugString(L"No collision \n");
	}

    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

	UpdateObjectCBs(gt);
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
    if(mIsWireframe)
    {
        ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque_wireframe"].Get()));
    }
    else
    {
        ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));
    }

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

    ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

    int passCbvIndex = mPassCbvOffset + mCurrFrameResourceIndex;
    auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    passCbvHandle.Offset(passCbvIndex, mCbvSrvUavDescriptorSize);
    mCommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

    DrawRenderItems(mCommandList.Get(), mOpaqueRitems);

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

void App::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void App::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void App::OnMouseMove(WPARAM btnState, int x, int y)
{
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
            float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));
            mCamera.Pitch(dy);
            mCamera.RotateY(dx);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
}

bool App::collisionCheck(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax, XMMATRIX& firstboxworld, XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, XMMATRIX& secondboxworld)
{
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

void App::handleCollision(XMVECTOR& firstboxmin, XMVECTOR& firstboxmax, XMFLOAT3& firsttranslation, XMVECTOR& secondboxmin, XMVECTOR& secondboxmax, float velocity)
{
	//half length of x y and z of the boxes used to calculate the center of the box
	XMFLOAT3 firstxyz = { (XMVectorGetX(firstboxmax) - XMVectorGetX(firstboxmin)) / 2, (XMVectorGetY(firstboxmax) - XMVectorGetY(firstboxmin)) / 2,(XMVectorGetZ(firstboxmax) - XMVectorGetZ(firstboxmin)) / 2};
	XMFLOAT3 secondxyz = { (XMVectorGetX(secondboxmax) - XMVectorGetX(secondboxmin)) / 2, (XMVectorGetY(secondboxmax) - XMVectorGetY(secondboxmin)) / 2,(XMVectorGetZ(secondboxmax) - XMVectorGetZ(secondboxmin)) / 2 };

	//boxes centers calculated with half the length of a side
	XMFLOAT3 firstboxcenter = {XMVectorGetX(firstboxmin)+firstxyz.x,XMVectorGetY(firstboxmin)+firstxyz.y,XMVectorGetZ(firstboxmin)+firstxyz.z};
	XMFLOAT3 secondboxcenter = { XMVectorGetX(secondboxmin) + secondxyz.x, XMVectorGetY(secondboxmin) + secondxyz.y , XMVectorGetZ(secondboxmin) + secondxyz.z };

	//debugging output to check if values are correct//
	/*std::wostringstream ss;
	ss << XMVectorGetX(firstboxmin) << " " << XMVectorGetX(secondboxmin)<< std::endl;
	ss << "Firstbox "<<firstboxcenter.x << " " << firstboxcenter.y << " " << firstboxcenter.z << std::endl;
	ss << "Secondbox " << secondboxcenter.x << " " << secondboxcenter.y << " " << secondboxcenter.z << std::endl;
	ss << "normal " <<normal.x << " " << normal.y << " " << normal.z << std::endl;
	ss << std::endl;
	OutputDebugString(ss.str().c_str());*/

	XMFLOAT3 firstMin;
	XMFLOAT3 firstMax;
	XMStoreFloat3(&firstMin, firstboxmin);
	XMStoreFloat3(&firstMax, firstboxmax);

	XMFLOAT3 secondMin;
	XMFLOAT3 secondMax;
	XMStoreFloat3(&secondMin, secondboxmin);
	XMStoreFloat3(&secondMax, secondboxmax);

	XMFLOAT3 intMin = makeCeil(firstMin,secondMin);
	XMFLOAT3 intMax = makeFloor(firstMax,secondMax);

	//area of the intersection of the two boxes
	XMFLOAT3 intersection = {intMax.x-intMin.x,intMax.y-intMin.y,intMax.z-intMin.z};

	float ax = fabs(intersection.x);
	float ay = fabs(intersection.y);
	float az = fabs(intersection.z);

	//Calculating x y and z normals for faces
	float sx = firstboxcenter.x < secondboxcenter.x ? -1.0f : 1.0f;
	float sy = firstboxcenter.y < secondboxcenter.y ? -1.0f : 1.0f;
	float sz = firstboxcenter.z < secondboxcenter.z ? -1.0f : 1.0f;

	//checking which face is colliding with and multiplying collision normal of face
	if (ax <= ay && ax <= az) {
		pos.x += velocity * sx;
	}
	else if (ay <= az) {
		pos.y += velocity * sy;
	}
	else {
		pos.z += velocity * sz;
	}

}

XMFLOAT3 App::makeCeil(XMFLOAT3 first, XMFLOAT3 second)
{
	XMFLOAT3 result;
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
	ents.insert(make_pair(name, new Entity{pos, right, up, look}));
}

Entity* App::FindEnt(string name) {
	return ents.find("player")->second;
}
 
void App::OnKeyboardInput(const GameTimer& gt)
{
    const float dt = gt.DeltaTime();
	PhysicsEntity* entPhys = FindEnt("player")->GetPhysHolder();

    float boxSpeed = 3.0f * dt;

	if (GetAsyncKeyState('Q') & 0x8000) {
		entPhys->setAngleNegative();
	}
	if (GetAsyncKeyState('E') & 0x8000) {
		entPhys->setAnglePositive();
		//keyboardInput.y -= boxSpeed;
	}
	if (GetAsyncKeyState('W') & 0x8000) {
		firstbox->moveside = 1;
		entPhys->setZIntentPositive();
		//keyboardInput.z += boxSpeed;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		firstbox->moveside = 2;
		entPhys->setZIntentNegative();
		//keyboardInput.z -= boxSpeed;
	}
	if (GetAsyncKeyState('A') & 0x8000){
		firstbox->moveside = 3;
		entPhys->setXIntentNegative();
	//keyboardInput.x -= boxSpeed;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		firstbox->moveside = 4;
		entPhys->setXIntentPositive();
		//keyboardInput.x += boxSpeed;
	}
	if (GetAsyncKeyState(' ') & 0x8000) {
		entPhys->decrementJump();
	}

	//box translation//
	XMMATRIX boxRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
	XMMATRIX boxScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(pos.x, pos.y, pos.z);
	gameClient->sendToServer(pos.x, pos.y, pos.z);
	XMMATRIX boxWorld = boxRotate * boxScale * boxOffset;
	firstbox->Geo;
	XMStoreFloat4x4(&firstbox->World, boxWorld);
	//calculate new bounding box of first box
	calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);

	if (collisionCheck(firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex, XMLoadFloat4x4(&firstbox->World),
		secondbox->boundingboxminvertex, secondbox->boundingboxmaxvertex, XMLoadFloat4x4(&secondbox->World))) {

		//after entity class is fleshed out some of these parameters can be removed and only refer to the entity
		handleCollision(firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex, pos,
			secondbox->boundingboxminvertex, secondbox->boundingboxmaxvertex,boxSpeed);

		boxOffset = XMMatrixTranslation(pos.x, pos.y, pos.z);
		boxWorld = boxRotate * boxScale * boxOffset;
		XMStoreFloat4x4(&firstbox->World, boxWorld);

		//calculate new bounding box of first box after collision
		calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);
	}
	
	//formerly mboxritemmovable
    firstbox->NumFramesDirty++;

	Physics::XYZPhysics(pos, entPhys, boxSpeed);
	FindEnt("player")->SetPosition(pos);
    if (!isTopDown) {
        mCamera.SetPosition(FindEnt("player")->getHPos());
    }
    mCamera.UpdateViewMatrix();
}
 

void App::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for(auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
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

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
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
	}
}

void App::BuildDescriptorHeaps()
{
    UINT objCount = (UINT)mOpaqueRitems.size();

    // Need a CBV descriptor for each object for each frame resource,
    // +1 for the perPass CBV for each frame resource.
    UINT numDescriptors = (objCount+1) * gNumFrameResources;

    // Save an offset to the start of the pass CBVs.  These are the last 3 descriptors.
    mPassCbvOffset = objCount * gNumFrameResources;

    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = numDescriptors;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&mCbvHeap)));
}

void App::BuildConstantBufferViews()
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    UINT objCount = (UINT)mOpaqueRitems.size();

    // Need a CBV descriptor for each object for each frame resource.
    for(int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
    {
        auto objectCB = mFrameResources[frameIndex]->ObjectCB->Resource();
        for(UINT i = 0; i < objCount; ++i)
        {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

            // Offset to the ith object constant buffer in the buffer.
            cbAddress += i*objCBByteSize;

            // Offset to the object cbv in the descriptor heap.
            int heapIndex = frameIndex*objCount + i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = cbAddress;
            cbvDesc.SizeInBytes = objCBByteSize;

            md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
        }
    }

    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

    // Last three descriptors are the pass CBVs for each frame resource.
    for(int frameIndex = 0; frameIndex < gNumFrameResources; ++frameIndex)
    {
        auto passCB = mFrameResources[frameIndex]->PassCB->Resource();
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress = passCB->GetGPUVirtualAddress();

        // Offset to the pass cbv in the descriptor heap.
        int heapIndex = mPassCbvOffset + frameIndex;
        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(heapIndex, mCbvSrvUavDescriptorSize);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = cbAddress;
        cbvDesc.SizeInBytes = passCBByteSize;
        
        md3dDevice->CreateConstantBufferView(&cbvDesc, handle);
    }
}

void App::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE cbvTable0;
    cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

    CD3DX12_DESCRIPTOR_RANGE cbvTable1;
    cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// Create root CBVs.
    slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
    slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, 
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
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
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_1");
	
    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void App::BuildShapeGeometry()
{
    GeometryGenerator geoGen;
	box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData box2 = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
    
	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT box2VertexOffset = (UINT)box.Vertices.size();
	UINT gridVertexOffset = box2VertexOffset+ (UINT)box2.Vertices.size();
	
	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT box2IndexOffset = (UINT)box.Indices32.size();
	UINT gridIndexOffset = box2IndexOffset+(UINT)box2.Indices32.size();
	
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


    auto totalVertexCount =
        box.Vertices.size() + box2.Vertices.size() +
        grid.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(DirectX::Colors::Purple);
	}

    for (size_t i = 0; i < box2.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = box2.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(DirectX::Colors::Blue);
    }

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
        vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
	}


	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(box2.GetIndices16()), std::end(box2.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	
    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size()  * sizeof(std::uint16_t);

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

	mGeometries[geo->Name] = std::move(geo);
}

void App::BuildSkyBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData skyBox = geoGen.CreateBox(100.0f, 100.0f, 100.0f, 3);

	UINT skyBoxVertexOffset = 0;

	UINT skyBoxIndexOffset = 0;

	SubmeshGeometry skyBoxSubmesh;
	skyBoxSubmesh.IndexCount = (UINT)skyBox.Indices32.size();
	skyBoxSubmesh.StartIndexLocation = skyBoxIndexOffset;
	skyBoxSubmesh.BaseVertexLocation = skyBoxVertexOffset;

	auto totalVertexCount =
		skyBox.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < skyBox.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = skyBox.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::SkyBlue);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(skyBox.GetIndices16()), std::end(skyBox.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "skyBoxGeo";

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

	geo->DrawArgs["skyBox"] = skyBoxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}


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
		vertices[k].Color = XMFLOAT4(DirectX::Colors::Red);
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

	CD3DX12_RASTERIZER_DESC rsDesc(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	rsDesc.CullMode = D3D12_CULL_MODE_NONE;
	opaquePsoDesc.RasterizerState = rsDesc;
	opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
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
    // PSO for opaque wireframe objects.
    //

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));
}

void App::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllRitems.size()));
    }
}

void App::BuildRenderItems()
{
	XMMATRIX box1Translation;
	XMMATRIX box2Translation;
	UINT objCBIndex = 0;
	if (gameServer != nullptr) {
		//can probably remove the translations because we're using pos global
		box1Translation = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
		box2Translation = XMMatrixTranslation(5.0f, 0.5f, 0.0f);
		pos = { 0.0f, 0.5f, 0.0f };
	}
	else {
		//can probably remove the translations because we're using pos global
		box1Translation = XMMatrixTranslation(5.0f, 0.5f, 0.0f);
		box2Translation = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
		pos = { 5.0f, 0.5f, 0.0f };
	}
	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * box1Translation);
	boxRitem->ObjCBIndex = objCBIndex++;
	boxRitem->Geo = mGeometries["shapeGeo"].get();
	boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    firstbox = boxRitem.get();
	mAllRitems.push_back(std::move(boxRitem));

	calcAABB(boxBoundingVertPosArray, firstbox->World, firstbox->boundingboxminvertex, firstbox->boundingboxmaxvertex);


    auto boxRitem2 = std::make_unique<RenderItem>();
    XMStoreFloat4x4(&boxRitem2->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * box2Translation);
    boxRitem2->ObjCBIndex = objCBIndex++;
    boxRitem2->Geo = mGeometries["shapeGeo"].get();
    boxRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    boxRitem2->IndexCount = boxRitem2->Geo->DrawArgs["box2"].IndexCount;
    boxRitem2->StartIndexLocation = boxRitem2->Geo->DrawArgs["box2"].StartIndexLocation;
    boxRitem2->BaseVertexLocation = boxRitem2->Geo->DrawArgs["box2"].BaseVertexLocation;
	secondbox = boxRitem2.get();
	gameClient->setPlayer(boxRitem2.get());
    mAllRitems.push_back(std::move(boxRitem2));

	calcAABB(boxBoundingVertPosArray, secondbox->World, secondbox->boundingboxminvertex, secondbox->boundingboxmaxvertex);

    auto gridRitem = std::make_unique<RenderItem>();
    XMStoreFloat4x4(&gridRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	gridRitem->ObjCBIndex = objCBIndex++;
	gridRitem->Geo = mGeometries["shapeGeo"].get();
	gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	mAllRitems.push_back(std::move(gridRitem));

	auto skyBoxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&skyBoxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	skyBoxRitem->ObjCBIndex = objCBIndex++;
	skyBoxRitem->Geo = mGeometries["skyBoxGeo"].get();
	skyBoxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skyBoxRitem->IndexCount = skyBoxRitem->Geo->DrawArgs["skyBox"].IndexCount;
	skyBoxRitem->StartIndexLocation = skyBoxRitem->Geo->DrawArgs["skyBox"].StartIndexLocation;
	skyBoxRitem->BaseVertexLocation = skyBoxRitem->Geo->DrawArgs["skyBox"].BaseVertexLocation;
	mAllRitems.push_back(std::move(skyBoxRitem));

	for (int i = 0; i < 10; ++i) {
		auto platformRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&platformRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(-5.0f, 0.0f, i*5.0f));
		platformRitem->ObjCBIndex = objCBIndex++;
		platformRitem->Geo = mGeometries["platformGeo"].get();
		platformRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		platformRitem->IndexCount = platformRitem->Geo->DrawArgs["platform"].IndexCount;
		platformRitem->StartIndexLocation = platformRitem->Geo->DrawArgs["platform"].StartIndexLocation;
		platformRitem->BaseVertexLocation = platformRitem->Geo->DrawArgs["platform"].BaseVertexLocation;
		mAllRitems.push_back(std::move(platformRitem));
	}
	

	/*UINT objCBIndex = 2;
	for(int i = 0; i < 5; ++i)
	{
		auto leftCylRitem = std::make_unique<RenderItem>();
		auto rightCylRitem = std::make_unique<RenderItem>();
		auto leftSphereRitem = std::make_unique<RenderItem>();
		auto rightSphereRitem = std::make_unique<RenderItem>();

		XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f);
		XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f);

		XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f);
		XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f);

		XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
		leftCylRitem->ObjCBIndex = objCBIndex++;
		leftCylRitem->Geo = mGeometries["shapeGeo"].get();
		leftCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
		leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
		rightCylRitem->ObjCBIndex = objCBIndex++;
		rightCylRitem->Geo = mGeometries["shapeGeo"].get();
		rightCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRitem->IndexCount = rightCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
		rightCylRitem->StartIndexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
		leftSphereRitem->ObjCBIndex = objCBIndex++;
		leftSphereRitem->Geo = mGeometries["shapeGeo"].get();
		leftSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRitem->IndexCount = leftSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

		XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
		rightSphereRitem->ObjCBIndex = objCBIndex++;
		rightSphereRitem->Geo = mGeometries["shapeGeo"].get();
		rightSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRitem->IndexCount = rightSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
		rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
		rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

		mAllRitems.push_back(std::move(leftCylRitem));
		mAllRitems.push_back(std::move(rightCylRitem));
		mAllRitems.push_back(std::move(leftSphereRitem));
		mAllRitems.push_back(std::move(rightSphereRitem));
	}*/

	// All the render items are opaque.
	for(auto& e : mAllRitems)
		mOpaqueRitems.push_back(e.get());
}

void App::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
 
	auto objectCB = mCurrFrameResource->ObjectCB->Resource();

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        // Offset to the CBV in the descriptor heap for this object and for this frame resource.
        UINT cbvIndex = mCurrFrameResourceIndex*(UINT)mOpaqueRitems.size() + ri->ObjCBIndex;
        auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
        cbvHandle.Offset(cbvIndex, mCbvSrvUavDescriptorSize);

        cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}
