//***************************************************************************************
// MainApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common//UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/Camera.h"
#include "FrameResource.h"
#include <iostream>
#include <fbxsdk.h>
#include "Ssao.h"
#include <string>
#include "ShadowMap.h"
#include "EditorGUIincludes.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;


struct test
{
	test() = default;
	test(const test& rhs) = delete;
	std::string name;
	XMFLOAT4X4 World;
	bool bIs2D;
	XMFLOAT4X4 TexTransform;
	UINT instanceBufferIndex;
	BoundingBox Bounds;
	UINT MatIndex;
	std::vector<InstanceData> Instances;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT InstanceCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;
    RenderItem(const RenderItem& rhs) = delete;
 
    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();
	bool bIs2D = false;
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	
	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	//int NumFramesDirty = gNumFrameResources;
	//index into array of instance buffers in a frameresource
	UINT instanceBufferIndex = 0;
	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	BoundingBox Bounds;
	std::vector<InstanceData> Instances;
	
	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
	UINT InstanceCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

struct indiceHolder
{
	UINT texCIndex;
	UINT posIndex;
};

enum class RenderLayer : int
{
	Opaque = 0,
    Debug = 1,
	Sky = 2,
	Rendered = 3,
	EditorGUI = 4,
	Count = 5
	
};

class MainApp : public D3DApp
{
public:
    MainApp(HINSTANCE hInstance);
    MainApp(const MainApp& rhs) = delete;
    MainApp& operator=(const MainApp& rhs) = delete;
    ~MainApp();

    virtual bool Initialize()override;

private:
    virtual void CreateRtvAndDsvDescriptorHeaps()override;
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateInstanceData(const GameTimer& gt);
	void UpdateGUIdata(const GameTimer& gt);
	int foundSimilarVertex(Vertex temp,std::vector<Vertex> vertices);
	bool is_near(float v1, float v2);
		
	
	void UpdateMaterialBuffer(const GameTimer& gt);
    void UpdateShadowTransform(const GameTimer& gt);
    void UpdateShadowPassCB(const GameTimer& gt);
	void UpdatePlayerPassCB(const GameTimer& gt);
	void UpdateSpectatePassCB(const GameTimer& gt);
	void UpdateSsaoCB(const GameTimer& gt);

	void CheckGuiInteraction(int x, int y);
	void LoadTextures();
	void CreatePlayerView();
    void BuildRootSignature();
	void BuildSsaoRootSignature();
	void BuildDescriptorHeaps();
	HRESULT LoadFBX(std::vector<Vertex> &pOutVertexVector);
    void BuildShadersAndInputLayout();
    void BuildShapeGeometry();
    void BuildSkullGeometry();
	void FBXFunction();
	void BuildBoxModel();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildRenderItems();
	void DrawEditorGUI(ID3D12GraphicsCommandList* cmdList);
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	void DrawImmerseObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<ImmerseObject*>& iObjects);
    void DrawSceneToShadowMap();
	void DrawNormalsAndDepth();
	void BuildEditorGUI();
	void CreateMainFont();
	void CreateText(std::string& text,XMFLOAT2 position,float fontSize,XMFLOAT4 color = XMFLOAT4(1.0f,1.0f,1.0f,1.0f));
	void DrawTextGUI(ID3D12GraphicsCommandList* cmdList);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index)const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index)const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index)const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const;

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mSsaoRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;
	
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unique_ptr<GUIgeometry> mainGUIgeometry;
	std::vector<Vertex> CalculateVerts();
	std::vector<std::unique_ptr<MeshGeometry>> mGUIGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	FbxManager* g_pFbxSdkManager = nullptr;
 
	std::vector<ImmerseText*> mImmerseTextObjects;
	UINT textBufferLastIndiceIndex = 0;
	UINT textBufferLastVertexIndex = 0;
	
	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	std::vector < std::unique_ptr<ImmerseObject>> mImmerseObjects;
	std::vector<ImmerseObject*> mAllImmerseObjects;
	std::unordered_map<std::string, std::unique_ptr<ImmerseObject>> mImmerseObjectMap;
	
	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	UINT mSkyTexHeapIndex = 0;
    UINT mShadowMapHeapIndex = 0;
	UINT mSsaoHeapIndexStart = 0;
	UINT mSsaoAmbientMapIndex = 0;
    UINT mNullCubeSrvIndex = 0;
	UINT mNullTexSrvIndex1 = 0;
	UINT mNullTexSrvIndex2 = 0;
	UINT mFontTexSrvIndex = 0;
	UINT playerViewSrvIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE playerViewDSVHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE playerViewSRVHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE nullSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE fontTextureSrv;
    PassConstants mMainPassCB;  // index 0 of pass cbuffer.
    PassConstants mShadowPassCB;// index 1 of pass cbuffer.
	PassConstants mSpectatePassCB;
	CD3DX12_CPU_DESCRIPTOR_HANDLE playerViewRTVHandle;
	Camera mCamera;
	Camera mSpectateCamera;

    std::unique_ptr<ShadowMap> mShadowMap;
	std::unique_ptr<ImmerseFont> mainFont;
	std::vector<Vertex> fontVertices;
	std::unique_ptr<Ssao> mSsao;
	std::unique_ptr<Texture> fontTexture;
	std::unique_ptr<Texture> testTexture;

	D3D12_VIEWPORT mPlayerViewViewport;
	D3D12_RECT mPlayerViewScissorRect;

	Microsoft::WRL::ComPtr<ID3D12Resource>  mPlayerViewTexMap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>  mPlayerViewRTVTexMap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>  mPlayerViewDepthStencilBuffer = nullptr;

    DirectX::BoundingSphere mSceneBounds;

    float mLightNearZ = 0.0f;
    float mLightFarZ = 0.0f;
    XMFLOAT3 mLightPosW;
    XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
	XMFLOAT4X4 mSpectateView = MathHelper::Identity4x4();
    XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
    XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

    float mLightRotationAngle = 0.0f;
    XMFLOAT3 mBaseLightDirections[3] = {
        XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(0.0f, -0.707f, -0.707f)
    };

	XMFLOAT3 mSpectateDirection = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
    XMFLOAT3 mRotatedLightDirections[3];
	XMFLOAT3 mMoonLightDirection = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);


	UINT mInstanceCount = 0;

	bool mFrustumCullingEnabled = true;

	BoundingFrustum mCamFrustum;


    POINT mLastMousePos;
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
        MainApp theApp(hInstance);
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

MainApp::MainApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
    // Estimate the scene bounding sphere manually since we know how the scene was constructed.
    // The grid is the "widest object" with a width of 20 and depth of 30.0f, and centered at
    // the world space origin.  In general, you need to loop over every world space vertex
    // position and compute the bounding sphere.
    mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);
}

MainApp::~MainApp()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool MainApp::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	mCamera.SetPosition(0.0f, 2.0f, -15.0f);
	mSpectateCamera.SetPosition(30.0f, 30.0f, 20.0f);
	mSpectateCamera.LookAt(XMFLOAT3(mSpectateCamera.GetPosition3f()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
    mShadowMap = std::make_unique<ShadowMap>(
        md3dDevice.Get(), 2048, 2048);
	mainGUIgeometry = std::make_unique<GUIgeometry>();
	mSsao = std::make_unique<Ssao>(
		md3dDevice.Get(),
		mCommandList.Get(),
		mClientWidth, mClientHeight);
	
	LoadTextures();
	BuildRootSignature();
	BuildSsaoRootSignature();
	BuildDescriptorHeaps();
	//CreatePlayerView();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildEditorGUI();
	CreateMainFont();
	BuildSkullGeometry();
	BuildBoxModel();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();
	
	

	mSsao->SetPSOs(mPSOs["ssao"].Get(), mPSOs["ssaoBlur"].Get());
    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}

void MainApp::CreateRtvAndDsvDescriptorHeaps()
{
    // Add +6 RTV for cube render target.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 4;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

    // Add +1 DSV for shadow map.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 3;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));


}
 
void MainApp::OnResize()
{
    D3DApp::OnResize();

	mCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mSpectateCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera.GetProj());

	if (mSsao != nullptr)
	{
		mSsao->OnResize(mClientWidth, mClientHeight);

		// Resources changed, so need to rebuild descriptors.
		mSsao->RebuildDescriptors(mDepthStencilBuffer.Get());
	}
}

void MainApp::Update(const GameTimer& gt)
{
	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();


    OnKeyboardInput(gt);
	

	auto currGUIVB = mCurrFrameResource->EditorGUIVB.get();

	std::vector<Vertex> verts = CalculateVerts();

	
	for (UINT i = 0; i < (UINT)verts.size(); i++)
	{
	Vertex vert = verts[i];
	currGUIVB->CopyData(i, vert);
	}

	mainGUIgeometry->VertexBufferGPU = currGUIVB->Resource();


    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }


	AnimateMaterials(gt);
	
	UpdateInstanceData(gt);
	UpdateGUIdata(gt);
	UpdateMaterialBuffer(gt);
    UpdateShadowTransform(gt);
	UpdatePlayerPassCB(gt);
    UpdateShadowPassCB(gt);
	UpdateSpectatePassCB(gt);
	UpdateSsaoCB(gt);
}

void MainApp::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

    ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    mCommandList->SetGraphicsRootSignature(mRootSignature.Get());


    // Bind all the materials used in this scene.  For structured buffers, we can bypass the heap and 
    // set as a root descriptor.
    auto matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
    mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());
	auto passCB = mCurrFrameResource->PassCB->Resource();
    // Bind null SRV for shadow map pass and playerview map pass.
    mCommandList->SetGraphicsRootDescriptorTable(3, mNullSrv);	 
	
	

    // Bind all the textures used in this scene.  Observe
    // that we only have to specify the first descriptor in the table.  
    // The root signature knows how many descriptors are expected in the table.
    mCommandList->SetGraphicsRootDescriptorTable(4, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

    DrawSceneToShadowMap();

	DrawNormalsAndDepth();
	
	//
	// Compute SSAO.
	// 

	mCommandList->SetGraphicsRootSignature(mSsaoRootSignature.Get());
	mSsao->ComputeSsao(mCommandList.Get(), mCurrFrameResource, 3);




	//
	// Main rendering pass.
	//on
	
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	matBuffer = mCurrFrameResource->MaterialBuffer->Resource();
	mCommandList->SetGraphicsRootShaderResourceView(2, matBuffer->GetGPUVirtualAddress());

	
	

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightBlue, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	mCommandList->SetGraphicsRootDescriptorTable(4, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());


	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(mSkyTexHeapIndex, mCbvSrvUavDescriptorSize);

	mCommandList->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

    mCommandList->SetPipelineState(mPSOs["opaque"].Get());
    DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);
	
	DrawImmerseObjects(mCommandList.Get(), mAllImmerseObjects);

   // mCommandList->SetPipelineState(mPSOs["debug"].Get());
	//DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Debug]);

	mCommandList->SetPipelineState(mPSOs["sky"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);

	mCommandList->SetPipelineState(mPSOs["EditorGUI"].Get());
	DrawEditorGUI(mCommandList.Get());
	if (!fontVertices.empty())
	{
		mCommandList->SetPipelineState(mPSOs["FontShader"].Get());
		DrawTextGUI(mCommandList.Get());
	}
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

void MainApp::OnMouseDown(WPARAM btnState, int x, int y)
{

    mLastMousePos.x = x;
    mLastMousePos.y = y;
	CheckGuiInteraction(x, y);
	

    SetCapture(mhMainWnd);
}

void MainApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	//check if an instance of the object has already been created
	//if there is no created instance, create the object
	//if there is a created instance, resize the object's instance array, increase its instance count and set the last instance 
	/*
	bool contains = false;
	UINT index;
	for (UINT i = 0; i < (UINT)mImmerseObjects.size(); i++)
	{
		if (mImmerseObjects[i].get()->name == "testBox")
		{
			index = i;
			contains = true;
			break;

		}
	}
	if (!contains)
	{
		auto testObject = std::make_unique<ImmerseObject>(
			"testBox",
			XMMatrixScaling(2.0f, 2.0f, 2.0f)* DirectX::XMMatrixTranslationFromVector(mCamera.GetPosition() + XMVector3Normalize(mCamera.GetLook()) * 10.0f),
			XMMatrixScaling(1.0f, 1.0f, 1.0f),
			mMaterials["bricks0"].get(),
			mGeometries["shapeGeo"].get(),
			mGeometries["shapeGeo"].get()->DrawArgs["box"].IndexCount,
			mGeometries["shapeGeo"].get()->DrawArgs["box"].StartIndexLocation,
			mGeometries["shapeGeo"].get()->DrawArgs["box"].BaseVertexLocation,
			0,
			1
			);
		
		mAllImmerseObjects.push_back(testObject.get());
		mImmerseObjects.push_back(std::move(testObject));
	}
	else
	{
		
		auto blah = mImmerseObjects[index].get();
		blah->InstanceCount += 1;
		blah->Instances.resize(blah->Instances.size() + 1);
		XMStoreFloat4x4(&blah->Instances.back().World, XMMatrixScaling(2.0f, 2.0f, 2.0f)* DirectX::XMMatrixTranslationFromVector(mCamera.GetPosition() + XMVector3Normalize(mCamera.GetLook()) * 10.0f));
		XMStoreFloat4x4(&blah->Instances.back().TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		blah->Instances.back().MaterialIndex = 0;
		
	}
	*/

	/*
	auto test = mAllRitems[3].get();
	test->Instances.resize(test->Instances.size() + 1);
	XMStoreFloat4x4(&test->Instances.back().World, XMMatrixScaling(2.0f, 2.0f, 2.0f)* DirectX::XMMatrixTranslationFromVector(mCamera.GetPosition() + XMVector3Normalize(mCamera.GetLook()) * 10.0f));
	XMStoreFloat4x4(&test->Instances.back().TexTransform, XMMatrixScaling(1.5f, 2.0f, 1.0f));
	test->Instances.back().MaterialIndex = 0;
	
	*/
	
    ReleaseCapture();
}

void MainApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
 
void MainApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if(GetAsyncKeyState('W') & 0x8000)
		mCamera.Walk(10.0f*dt);

	if(GetAsyncKeyState('S') & 0x8000)
		mCamera.Walk(-10.0f*dt);

	if(GetAsyncKeyState('A') & 0x8000)
		mCamera.Strafe(-10.0f*dt);

	if(GetAsyncKeyState('D') & 0x8000)
		mCamera.Strafe(10.0f*dt);
	mSpectateCamera.UpdateViewMatrix();
	mCamera.UpdateViewMatrix();
	//
}
 
void MainApp::AnimateMaterials(const GameTimer& gt)
{
	
}

void MainApp::UpdateInstanceData(const GameTimer & gt)
{
	
	XMMATRIX view = mCamera.GetView();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);


			



	for (auto& e : mAllRitems)
	{

		const auto& instanceData = e->Instances;
		auto currInstanceBuffer = mCurrFrameResource->renderItemBuffers[e->instanceBufferIndex].get();
		int visibleInstanceCount = 0;
		
		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{

			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			mCamFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.

			InstanceData data;
			XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
			data.MaterialIndex = instanceData[i].MaterialIndex;


			// Write the instance data to structured buffer for the visible objects.
			currInstanceBuffer->CopyData(visibleInstanceCount++, data);
	
		}
				

		e->InstanceCount = visibleInstanceCount;


	}

	
	for (auto& e : mAllImmerseObjects)
	{

		const auto& instanceData = e->Instances;
		auto currInstanceBuffer = mCurrFrameResource->ImmerseObjectBuffer.get();
		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{

			XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

			// View space to the object's local space.
			XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			BoundingFrustum localSpaceFrustum;
			mCamFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.

			InstanceData data;
			XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
			data.MaterialIndex = instanceData[i].MaterialIndex;


			// Write the instance data to structured buffer for the visible objects.
			currInstanceBuffer->CopyData(visibleInstanceCount++, data);

		}


		e->InstanceCount = visibleInstanceCount;


	}
	
	


	
	
}
void MainApp::UpdateGUIdata(const GameTimer & gt)
{
	
	for (auto& g : mainGUIgeometry->subGeos)
	{
		int visableGUIcount = 0;
		auto gui = g->myGUI;
		auto currGUIdataBuffer = mCurrFrameResource->GUIdataBuffers[gui->bufferIndex].get();
		
		GUIdata tempGUIdata;
		tempGUIdata.color = gui->color;
		currGUIdataBuffer->CopyData(visableGUIcount++, tempGUIdata);
	}

	auto currFontVB = mCurrFrameResource->fontVB.get();

	
	for (auto immerseText : mImmerseTextObjects)
	{


		if (immerseText->bVisible)
		{
			immerseText->instanceCount = 1;
		}
		else
		{
			immerseText->instanceCount = 0;
			/*
			std::string output = " INVIS ";

			
			std::wstring temp(output.begin(), output.end());
			OutputDebugStringW(temp.c_str());
			*/
		}
		for (UINT i = 0; i < (UINT)immerseText->myVerticesPos.size(); i++)
		{
			
			Vertex vert;

			vert.Pos = immerseText->myVerticesPos[i];

			vert.TexC = immerseText->myVerticesTex[i];
			/*
			std::string output = "working: " + std::to_string(vert.Pos.x);
			output += " ";

			std::wstring temp(output.begin(), output.end());
			OutputDebugStringW(temp.c_str());
			*/
			//vertsToRender.push_back(vert);
			
			currFontVB->CopyData(i, vert);
			
		}
	}
	/*
	for (UINT i = 0; i < (UINT)vertsToRender.size(); i++)
	{
		Vertex vert = vertsToRender[i];
		currFontVB->CopyData(i, vert);
	}
	*/
	/*
	for (UINT i = 0; i < (UINT)fontVertices.size(); i++)
	{
		Vertex vert = fontVertices[i];
		currFontVB->CopyData(i, vert);
	}
	*/
	mainFont->VertexBufferGPU = currFontVB->Resource();


}
void MainApp::UpdateMaterialBuffer(const GameTimer& gt)
{
	auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
	for(auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialData matData;
			matData.DiffuseAlbedo = mat->DiffuseAlbedo;
			matData.FresnelR0 = mat->FresnelR0;
			matData.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
			matData.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
			matData.NormalMapIndex = mat->NormalSrvHeapIndex;

			currMaterialBuffer->CopyData(mat->MatCBIndex, matData);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}


}

void MainApp::UpdateShadowTransform(const GameTimer& gt)
{
    // Only the first "main" light casts a shadow.
    XMVECTOR lightDir = XMLoadFloat3(&mMoonLightDirection);
    XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
    XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
    XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

    XMStoreFloat3(&mLightPosW, lightPos);

    // Transform bounding sphere to light space.
    XMFLOAT3 sphereCenterLS;
    XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - mSceneBounds.Radius;
    float b = sphereCenterLS.y - mSceneBounds.Radius;
    float n = sphereCenterLS.z - mSceneBounds.Radius;
    float r = sphereCenterLS.x + mSceneBounds.Radius;
    float t = sphereCenterLS.y + mSceneBounds.Radius;
    float f = sphereCenterLS.z + mSceneBounds.Radius;

    mLightNearZ = n;
    mLightFarZ = f;
    XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    XMMATRIX S = lightView*lightProj*T;
    XMStoreFloat4x4(&mLightView, lightView);
    XMStoreFloat4x4(&mLightProj, lightProj);
    XMStoreFloat4x4(&mShadowTransform, S);
}

void MainApp::UpdateSpectatePassCB(const GameTimer& gt)
{
	//mSpectateCamera.UpdateViewMatrix();
	XMMATRIX view = mSpectateCamera.GetView();
	XMMATRIX proj = mSpectateCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

	XMStoreFloat4x4(&mSpectatePassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mSpectatePassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mSpectatePassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mSpectatePassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mSpectatePassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mSpectatePassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&mSpectatePassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));
	mSpectatePassCB.EyePosW = mSpectateCamera.GetPosition3f();
	mSpectatePassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mSpectatePassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mSpectatePassCB.NearZ = 1.0f;
	mSpectatePassCB.FarZ = 1000.0f;
	mSpectatePassCB.TotalTime = gt.TotalTime();
	mSpectatePassCB.DeltaTime = gt.DeltaTime();
	mSpectatePassCB.AmbientLight = { 0.15f, 0.15f, 0.25f, 1.0f };
	mSpectatePassCB.Lights[0].Direction = mMoonLightDirection;
	mSpectatePassCB.Lights[0].Strength = { 0.5f, 0.6f, 0.9f };


	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(2, mSpectatePassCB);
}

void MainApp::UpdateSsaoCB(const GameTimer & gt)
{
	SsaoConstants ssaoCB;

	XMMATRIX P = mCamera.GetProj();

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	ssaoCB.Proj = mMainPassCB.Proj;
	ssaoCB.InvProj = mMainPassCB.InvProj;
	XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P*T));

	mSsao->GetOffsetVectors(ssaoCB.OffsetVectors);

	auto blurWeights = mSsao->CalcGaussWeights(2.5f);
	ssaoCB.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
	ssaoCB.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
	ssaoCB.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);

	ssaoCB.InvRenderTargetSize = XMFLOAT2(1.0f / mSsao->SsaoMapWidth(), 1.0f / mSsao->SsaoMapHeight());

	// Coordinates given in view space.
	ssaoCB.OcclusionRadius = 0.5f;
	ssaoCB.OcclusionFadeStart = 0.2f;
	ssaoCB.OcclusionFadeEnd = 1.0f;
	ssaoCB.SurfaceEpsilon = 0.05f;

	auto currSsaoCB = mCurrFrameResource->SsaoCB.get();
	currSsaoCB->CopyData(0, ssaoCB);
}

void MainApp::UpdatePlayerPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX viewProjTex = XMMatrixMultiply(viewProj, T);
	XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProjTex, XMMatrixTranspose(viewProjTex));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&mMainPassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));
	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.75f, 0.75f, 0.75f, 1.0f };
	mMainPassCB.Lights[0].Direction = mMoonLightDirection;
	mMainPassCB.Lights[0].Strength = { 0.9f, 0.9f, 0.9f };


	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}



void MainApp::UpdateShadowPassCB(const GameTimer& gt)
{
    XMMATRIX view = XMLoadFloat4x4(&mLightView);
    XMMATRIX proj = XMLoadFloat4x4(&mLightProj);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
    XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
    XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

    UINT w = mShadowMap->Width();
    UINT h = mShadowMap->Height();

    XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.EyePosW = mLightPosW;
    mShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
    mShadowPassCB.NearZ = mLightNearZ;
    mShadowPassCB.FarZ = mLightFarZ;

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(1, mShadowPassCB);
}

void MainApp::CheckGuiInteraction(int x, int y)
{

	for (UINT i = 0; i < (UINT)mainGUIgeometry->subGeos.size(); i++)
	{
		auto gui = mainGUIgeometry->subGeos[i];
		float guiX = gui->myGUI->x;
		float guiY = gui->myGUI->y;
		float guiWidth = gui->myGUI->width;
		float guiHeight = gui->myGUI->height;
		float mouseXndc = (2.0f * x) / (mScreenViewport.Width) - 1;
		float mouseYndc = -(2.0f * y) / (mScreenViewport.Height) + 1;
		/*
		std::string pi = "guiX: " + std::to_string(guiX);
		pi += " guiY: " + std::to_string(guiY);
		pi += " guiWidth: " + std::to_string(guiWidth);
		pi += " guiHeight: " + std::to_string(guiHeight);
		pi += " mouseXndc: " + std::to_string(mouseXndc);
		pi += " mouseYndc: " + std::to_string(mouseYndc);
		pi += "  ";
		std::wstring stemp = std::wstring(pi.begin(), pi.end());
		LPCWSTR sw = stemp.c_str();
		OutputDebugString(sw);
		OutputDebugStringW(L"\n");
		*/

		if (guiX < mouseXndc && guiY > mouseYndc && guiX + guiWidth > mouseXndc && guiY - guiHeight < mouseYndc)
		{
			
			//gui->myGUI->color = DirectX::XMFLOAT4{ 0.0,0.0,0.0,1.0 };
			BaseGUI* temp = gui->myGUI;
			PanelGUI* checkPanel = dynamic_cast<PanelGUI*>(temp);
			if (checkPanel != nullptr)
			{
				if (!checkPanel->bIsClosed)
				{
					checkPanel->ChangeSize(true);
					if (checkPanel->myTitle != nullptr)
					{
						checkPanel->myTitle->bVisible = false;
					}
				}
				else
				{
					checkPanel->ChangeSize(false);
					if (checkPanel->myTitle != nullptr)
					{
						checkPanel->myTitle->bVisible = true;
					}
					else
					{

						
						std::string output = " NULL ";
						

						std::wstring temp(output.begin(), output.end());
						OutputDebugStringW(temp.c_str());
						

					}
				}
				
			}
			/*
			std::vector<Vertex> verts = CalculateVerts();

			
			for (UINT n = 0; n < (UINT)verts.size(); n++)
			{
				Vertex nVert = verts[n];
				mCurrFrameResource->EditorGUIVB.get()->CopyData(n, nVert);
			}

			mainGUIgeometry->VertexBufferGPU = mCurrFrameResource->EditorGUIVB.get()->Resource();
			*/
			std::string pi = " Clicked A Button ";

			std::wstring stemp = std::wstring(pi.begin(), pi.end());
			LPCWSTR sw = stemp.c_str();
			OutputDebugString(sw);
			OutputDebugStringW(L"\n");
		}

	}

}

void MainApp::LoadTextures()
{
	std::vector<std::string> texNames = 
	{
		"bricksDiffuseMap",
		"bricksNormalMap",
		"tileDiffuseMap",
		"tileNormalMap",
		"defaultDiffuseMap",
		"defaultNormalMap",
		"skyCubeMap",
		"AsphaltDiffuseMap",
		"AsphaltNormalMap",
		"ManDiffuseMap",
		"ManNormalMap"

	};
	
    std::vector<std::wstring> texFilenames =
    {
        L"Textures//bricks2.dds",
        L"Textures//bricks2_nmap.dds",
        L"Textures//tile.dds",
        L"Textures//tile_nmap.dds",
        L"Textures//white1x1.dds",
        L"Textures//default_nmap.dds",
        L"Textures//desertcube1024.dds",
		L"Textures//Asphalt2DDS.dds",
		L"Textures//Asphalt2DDSNorm.dds",
		L"Textures//average_man_color1_df.dds",
		L"Textures//average_man_nm+y.dds"
    };
	
	for(int i = 0; i < (int)texNames.size(); ++i)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->Name = texNames[i];
		texMap->Filename = texFilenames[i];
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));
			
		mTextures[texMap->Name] = std::move(texMap);
	}
	fontTexture = std::make_unique<Texture>();
	fontTexture->Name = "fontTexture";
	fontTexture->Filename = L"Textures//Font1dds.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), fontTexture->Filename.c_str(),
		fontTexture->Resource, fontTexture->UploadHeap));
	mainFont = std::make_unique<ImmerseFont>("Font1", (int)fontTexture->Resource->GetDesc().Width, (int)fontTexture->Resource->GetDesc().Height);


	testTexture = std::make_unique<Texture>();
	testTexture->Name = "testTexture";
	testTexture->Filename = L"Textures//checkboard.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), testTexture->Filename.c_str(),
		testTexture->Resource, testTexture->UploadHeap));

}

void MainApp::CreatePlayerView()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 600;
	texDesc.Height = 600;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	mPlayerViewViewport = { 0.0f, 0.0f, (float)600, (float)600, 0.0f, 1.0f };
	mPlayerViewScissorRect = { 0, 0, (int)600, (int)600 };
	D3D12_CLEAR_VALUE optClear;
	optClear.Format = texDesc.Format;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	
	

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mPlayerViewTexMap)));
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&mPlayerViewRTVTexMap)));


	D3D12_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = texDesc.Format;
	
	rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	rtDesc.Texture2D.PlaneSlice = 0;
	
	playerViewRTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	playerViewRTVHandle.Offset(SwapChainBufferCount,mRtvDescriptorSize);
	md3dDevice->CreateRenderTargetView(mPlayerViewRTVTexMap.Get(), &rtDesc, playerViewRTVHandle);

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = 600;
	depthStencilDesc.Height = 600;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(mPlayerViewDepthStencilBuffer.GetAddressOf())));
		
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mPlayerViewDepthStencilBuffer.Get(), &dsvDesc, playerViewDSVHandle);
	


	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.PlaneSlice = 0;
	md3dDevice->CreateShaderResourceView(mPlayerViewTexMap.Get(), &srvDesc, playerViewSRVHandle);



	
}

void MainApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 9, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 14, 9, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[7];

	// Perfomance TIP: Order from most frequent to least frequent.
    slotRootParameter[0].InitAsShaderResourceView(1, 1);
    slotRootParameter[1].InitAsConstantBufferView(1);
    slotRootParameter[2].InitAsShaderResourceView(0, 1);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[4].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[5].InitAsShaderResourceView(2, 1);
	slotRootParameter[6].InitAsShaderResourceView(3, 1);



	auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(7, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
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

void MainApp::BuildSsaoRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable0;
	texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

	CD3DX12_DESCRIPTOR_RANGE texTable1;
	texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstants(1, 1);
	slotRootParameter[2].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,
		0,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers =
	{
		pointClamp, linearClamp, depthMapSam, linearWrap
	};

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
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
		IID_PPV_ARGS(mSsaoRootSignature.GetAddressOf())));
}





void MainApp::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 19;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	std::vector<ComPtr<ID3D12Resource>> tex2DList = 
	{
		mTextures["bricksDiffuseMap"]->Resource,
		mTextures["bricksNormalMap"]->Resource,
		mTextures["tileDiffuseMap"]->Resource,
		mTextures["tileNormalMap"]->Resource,
		mTextures["defaultDiffuseMap"]->Resource,
		mTextures["defaultNormalMap"]->Resource,
		mTextures["AsphaltDiffuseMap"]->Resource,
		mTextures["AsphaltNormalMap"]->Resource,
		mTextures["ManDiffuseMap"]->Resource,
		mTextures["ManNormalMap"]->Resource
	};
	
	auto skyCubeMap = mTextures["skyCubeMap"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	
	for(UINT i = 0; i < (UINT)tex2DList.size(); ++i)
	{
		srvDesc.Format = tex2DList[i]->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = tex2DList[i]->GetDesc().MipLevels;
		md3dDevice->CreateShaderResourceView(tex2DList[i].Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, mCbvSrvUavDescriptorSize);
	}
	
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyCubeMap->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyCubeMap->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(skyCubeMap.Get(), &srvDesc, hDescriptor);
	
	mSkyTexHeapIndex = (UINT)tex2DList.size();
    mShadowMapHeapIndex = mSkyTexHeapIndex + 1;
	mSsaoHeapIndexStart = mShadowMapHeapIndex + 1;
	mSsaoAmbientMapIndex = mSsaoHeapIndexStart + 3;
    mNullCubeSrvIndex = mSsaoHeapIndexStart + 5;
	
    mNullTexSrvIndex1 = mNullCubeSrvIndex + 1;
	



    auto srvCpuStart = mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    auto srvGpuStart = mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    auto dsvCpuStart = mDsvHeap->GetCPUDescriptorHandleForHeapStart();



    nullSrv = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mNullCubeSrvIndex, mCbvSrvUavDescriptorSize);
    mNullSrv = CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, mNullCubeSrvIndex, mCbvSrvUavDescriptorSize);
	playerViewSRVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mNullCubeSrvIndex, mCbvSrvUavDescriptorSize);
	playerViewDSVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 2, mDsvDescriptorSize);
	

   // md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
   // nullSrv.Offset(1, mCbvSrvUavDescriptorSize);

    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
   // md3dDevice->CreateShaderResourceView(nullptr, &srvDesc, nullSrv);
	//nullSrv.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = fontTexture->Resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = fontTexture->Resource->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(fontTexture->Resource.Get(), &srvDesc, nullSrv);
	nullSrv.Offset(1, mCbvSrvUavDescriptorSize);
	srvDesc.Format = testTexture->Resource->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = testTexture->Resource->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(testTexture->Resource.Get(), &srvDesc, nullSrv);
	
    
    mShadowMap->BuildDescriptors(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, mShadowMapHeapIndex, mCbvSrvUavDescriptorSize),
        CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, mShadowMapHeapIndex, mCbvSrvUavDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 1, mDsvDescriptorSize));

	mSsao->BuildDescriptors(
		mDepthStencilBuffer.Get(),
		GetCpuSrv(mSsaoHeapIndexStart),
		GetGpuSrv(mSsaoHeapIndexStart),
		GetRtv(SwapChainBufferCount + 1), 
		mCbvSrvUavDescriptorSize,
		mRtvDescriptorSize);
	

	
}

HRESULT MainApp::LoadFBX(std::vector<Vertex> &pOutVertexVector)
{
	if (g_pFbxSdkManager == nullptr)
	{
		g_pFbxSdkManager = FbxManager::Create();

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
		g_pFbxSdkManager->SetIOSettings(pIOsettings);
	}
	std::string pi = " Failed to set file name ";
	std::wstring stemp = std::wstring(pi.begin(), pi.end());
	LPCWSTR sw = stemp.c_str();
	
	FbxImporter* pImporter = FbxImporter::Create(g_pFbxSdkManager, "");
	FbxScene* pFbxScene = FbxScene::Create(g_pFbxSdkManager, "");

	bool bSuccess = pImporter->Initialize("MyModels//man.fbx", -1, g_pFbxSdkManager->GetIOSettings());
	if (!bSuccess)
	{
		
		return E_FAIL;
	}
	bSuccess = pImporter->Import(pFbxScene);
	if (!bSuccess) return E_FAIL;

	pImporter->Destroy();

	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();


	if (pFbxRootNode)
	{
		
		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
		{

			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);
			

			if (pFbxChildNode->GetNodeAttribute() == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
			FbxStringList lUVSetNameList;
			pMesh->GetUVSetNames(lUVSetNameList);
			int materialCount = pFbxChildNode->GetSrcObjectCount<FbxSurfaceMaterial>();
			std::string nodeName(pFbxChildNode->GetName());

			for (int index = 0; index < materialCount; index++)
			{

				
				FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)pFbxChildNode->GetSrcObject<FbxSurfaceMaterial>(index);
				
				

				
				if (material != NULL)
				{

					
					// This only gets the material of type sDiffuse, you probably need to traverse all Standard Material Property by its name to get all possible textures.
					FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
					
					// Check if it's layeredtextures
					int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();

					if (layeredTextureCount > 0)
					{
						for (int j = 0; j < layeredTextureCount; j++)
						{
							FbxLayeredTexture* layered_texture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(j));
							int lcount = layered_texture->GetSrcObjectCount<FbxTexture>();

							for (int k = 0; k < lcount; k++)
							{
								FbxTexture* texture = FbxCast<FbxTexture>(layered_texture->GetSrcObject<FbxTexture>(k));
								
								// Then, you can get all the properties of the texture, include its name
								FbxFileTexture* fTexture = FbxCast<FbxFileTexture>(texture);
								if (fTexture != NULL)
								{	


								}



							}
						}
					}
					else
					{
						// Directly get textures
						int textureCount = prop.GetSrcObjectCount<FbxTexture>();
						for (int j = 0; j < textureCount; j++)
						{
							FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(j));
							FbxFileTexture* fTexture = FbxCast<FbxFileTexture>(texture);
							if (fTexture != NULL)
							{

							}
							
							

							
						}
					}
				}

			}
			
			
			FbxVector4* pVertices = pMesh->GetControlPoints();
			

			for (int j = 0; j < pMesh->GetPolygonCount(); j++)
			{
				int iNumVertices = pMesh->GetPolygonSize(j);
				assert(iNumVertices == 3);

				for (int k = 0; k < iNumVertices; k++) {
					int iControlPointIndex = pMesh->GetPolygonVertex(j, k);
					FbxVector2 vecFBX;
					

					Vertex vertex;
					vertex.Pos.x = (float)pVertices[iControlPointIndex].mData[0];
					vertex.Pos.y = (float)pVertices[iControlPointIndex].mData[1];
					vertex.Pos.z = (float)pVertices[iControlPointIndex].mData[2];
					int uvElementCount = pMesh->GetElementUVCount();

					for (int uvElement = 0; uvElement < uvElementCount; uvElement++)
					{
						FbxGeometryElementUV* geomElementUV = pMesh->GetElementUV(uvElement);
						
						const char* name = geomElementUV->GetName();
						FbxLayerElement::EMappingMode mapMode = geomElementUV->GetMappingMode();
						FbxLayerElement::EReferenceMode refMode = geomElementUV->GetReferenceMode();

						int directIndex = -1;

						if (FbxGeometryElement::eByControlPoint == mapMode)
						{

							if (FbxGeometryElement::eDirect == refMode)
							{
								directIndex = iControlPointIndex;
							}
							else if (FbxGeometryElement::eIndexToDirect == refMode)
							{
								directIndex = geomElementUV->GetIndexArray().GetAt(iControlPointIndex);
							}
						}
						else if (FbxGeometryElement::eByPolygonVertex == mapMode)
						{

							if (FbxGeometryElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
							{
								directIndex = pMesh->GetTextureUVIndex(j, k);
							}

						}

						// If we got a UV index
						if (directIndex != -1)
						{
							FbxVector2 uv = geomElementUV->GetDirectArray().GetAt(directIndex);
							float x = static_cast<float>(uv.mData[0]);
							float y = static_cast<float>(uv.mData[1]);
							
							if (x == 0.0f && y == 0.0f)
							{
								vertex.TexC.x = x;
								vertex.TexC.y = 1.0f - y;

							}
							else
							{
								vertex.TexC.x = x;
								vertex.TexC.y = 1.0f - y;
							}
							
	
						}
						else
						{
							OutputDebugString(sw);
						}
					}
					// Grab normals
					int normElementCount = pMesh->GetElementNormalCount();

					for (int normElement = 0; normElement < normElementCount; normElement++)
					{
						FbxGeometryElementNormal* geomElementNormal = pMesh->GetElementNormal(normElement);

						FbxLayerElement::EMappingMode mapMode = geomElementNormal->GetMappingMode();
						FbxLayerElement::EReferenceMode refMode = geomElementNormal->GetReferenceMode();

						int directIndex = -1;

						if (FbxGeometryElement::eByPolygonVertex == mapMode)
						{
							if (FbxGeometryElement::eDirect == refMode)
							{
								directIndex = iControlPointIndex;
							}
							else if (FbxGeometryElement::eIndexToDirect == refMode)
							{
								directIndex = geomElementNormal->GetIndexArray().GetAt(iControlPointIndex);
							}
						}

						// If we got an index
						if (directIndex != -1)
						{
							FbxVector4 norm = geomElementNormal->GetDirectArray().GetAt(directIndex);
							vertex.Normal.x = static_cast<float>(norm[0]);
							vertex.Normal.x = static_cast<float>(norm[1]);
							vertex.Normal.x = static_cast<float>(norm[2]);
							

						}
						else
						{
							pi = " vert has no Normal ";
							stemp = std::wstring(pi.begin(), pi.end());
							sw = stemp.c_str();
							OutputDebugString(sw);
						}
					}
					pOutVertexVector.push_back(vertex);
					
				}
			}

		}

	}
	return S_OK;
}

void MainApp::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["shadowVS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["shadowOpaquePS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_1");
    mShaders["shadowAlphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Shadows.hlsl", alphaTestDefines, "PS", "ps_5_1");
	
    mShaders["debugVS"] = d3dUtil::CompileShader(L"Shaders\\ShadowDebug.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["debugPS"] = d3dUtil::CompileShader(L"Shaders\\ShadowDebug.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["drawNormalsVS"] = d3dUtil::CompileShader(L"Shaders\\DrawNormals.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["drawNormalsPS"] = d3dUtil::CompileShader(L"Shaders\\DrawNormals.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["ssaoVS"] = d3dUtil::CompileShader(L"Shaders\\Ssao.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["ssaoPS"] = d3dUtil::CompileShader(L"Shaders\\Ssao.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["ssaoBlurVS"] = d3dUtil::CompileShader(L"Shaders\\SsaoBlur.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["ssaoBlurPS"] = d3dUtil::CompileShader(L"Shaders\\SsaoBlur.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["skyVS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["skyPS"] = d3dUtil::CompileShader(L"Shaders\\Sky.hlsl", nullptr, "PS", "ps_5_1");

	mShaders["editorVS"] = d3dUtil::CompileShader(L"Shaders\\EditorGUI.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["editorPS"] = d3dUtil::CompileShader(L"Shaders\\EditorGUI.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["fontVS"] = d3dUtil::CompileShader(L"Shaders\\FontShader.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["fontPS"] = d3dUtil::CompileShader(L"Shaders\\FontShader.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void MainApp::BuildShapeGeometry()
{
    GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData wall = geoGen.CreateBox(5.0f, 2.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
    GeometryGenerator::MeshData quad = geoGen.CreateQuad(0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    



	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();
    UINT quadVertexOffset = cylinderVertexOffset + (UINT)cylinder.Vertices.size();
	UINT wallVertexOffset = quadVertexOffset + (UINT)quad.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();
    UINT quadIndexOffset = cylinderIndexOffset + (UINT)cylinder.Indices32.size();
	UINT wallIndexOffset = quadIndexOffset + (UINT)quad.Indices32.size();

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;


	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;


	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;


	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;


    SubmeshGeometry quadSubmesh;
    quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
    quadSubmesh.StartIndexLocation = quadIndexOffset;
    quadSubmesh.BaseVertexLocation = quadVertexOffset;

	SubmeshGeometry wallSubmesh;
	wallSubmesh.IndexCount = (UINT)wall.Indices32.size();
	wallSubmesh.StartIndexLocation = wallIndexOffset;
	wallSubmesh.BaseVertexLocation = wallVertexOffset;


	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size() + 
        quad.Vertices.size() +
		wall.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].TexC = box.Vertices[i].TexC;
		vertices[k].TangentU = box.Vertices[i].TangentU;
	}

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Normal = grid.Vertices[i].Normal;
		vertices[k].TexC = grid.Vertices[i].TexC;
		vertices[k].TangentU = grid.Vertices[i].TangentU;
	}

	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Normal = sphere.Vertices[i].Normal;
		vertices[k].TexC = sphere.Vertices[i].TexC;
		vertices[k].TangentU = sphere.Vertices[i].TangentU;
	}

	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Normal = cylinder.Vertices[i].Normal;
		vertices[k].TexC = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = cylinder.Vertices[i].TangentU;
	}

    for(int i = 0; i < quad.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = quad.Vertices[i].Position;
        vertices[k].Normal = quad.Vertices[i].Normal;
        vertices[k].TexC = quad.Vertices[i].TexC;
        vertices[k].TangentU = quad.Vertices[i].TangentU;
    }

	for (size_t i = 0; i < wall.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = wall.Vertices[i].Position;
		vertices[k].Normal = { 0,0,0 };//wall.Vertices[i].Normal;
		vertices[k].TexC = wall.Vertices[i].TexC;
		vertices[k].TangentU = wall.Vertices[i].TangentU;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));
    indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));
	indices.insert(indices.end(), std::begin(wall.GetIndices16()), std::end(wall.GetIndices16()));

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
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
    geo->DrawArgs["quad"] = quadSubmesh;
	geo->DrawArgs["wall"] = wallSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void MainApp::BuildSkullGeometry()
{
    std::ifstream fin("MyModels//skull.txt");

    if (!fin)
    {
        MessageBox(0, L"MyModels//skull.txt not found.", 0, 0);
        return;
    }

    UINT vcount = 0;
    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> vcount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
    XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

    std::vector<Vertex> vertices(vcount);
    for (UINT i = 0; i < vcount; ++i)
    {
        fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		
        fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
        vertices[i].TexC = { 0.0f, 0.0f };

        XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

        XMVECTOR N = XMLoadFloat3(&vertices[i].Normal);

        // Generate a tangent vector so normal mapping works.  We aren't applying
        // a texture map to the skull, so we just need any tangent vector so that
        // the math works out to give us the original interpolated vertex normal.
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        if(fabsf(XMVectorGetX(XMVector3Dot(N, up))) < 1.0f - 0.001f)
        {
            XMVECTOR T = XMVector3Normalize(XMVector3Cross(up, N));
            XMStoreFloat3(&vertices[i].TangentU, T);
        }
        else
        {
            up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
            XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, up));
            XMStoreFloat3(&vertices[i].TangentU, T);
        }
        


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
}

void MainApp::FBXFunction()
{

}




bool MainApp::is_near(float v1, float v2) {
	return fabs(v1 - v2) < 0.01f;
}

int MainApp::foundSimilarVertex(Vertex temp, std::vector<Vertex> verts)
{
	if (verts.size() > 0)
	{
		for (UINT i = 0; i < (UINT)verts.size(); i++)
		{
			if (
				is_near(temp.Pos.x, verts[i].Pos.x) &&
				is_near(temp.Pos.y, verts[i].Pos.y) &&
				is_near(temp.Pos.z, verts[i].Pos.z) &&
				is_near(temp.Normal.x, verts[i].Normal.x) &&
				is_near(temp.Normal.y, verts[i].Normal.y) &&
				is_near(temp.Normal.z, verts[i].Normal.z)
				)
			{
				return i;
			}
		}
	}
	return -1;
}

void MainApp::BuildBoxModel()
{
	std::wstring Filename = L"../../MyModels/car.obj";

	
	std::vector<Vertex> vertices;
	std::vector<Vertex> outVertices;
	std::vector<Vertex> checkedVertices;
	std::vector<XMFLOAT3> vertPos;
	std::vector<XMFLOAT3> vertPosNorms;
	std::vector<XMFLOAT2> vertTC;
	UINT tCount = 12;


	std::vector<std::int32_t> indices;
	std::vector<std::int32_t> normalIndices;
	std::vector<std::int32_t> texIndices;
	std::vector<indiceHolder> indiceHolders;
	LoadFBX(vertices);
	for (UINT i = 0; i < (UINT)vertices.size(); i++)
	{
		indices.push_back(i);
	}
	
	
	
	if (vertices.size() == 0)
	{
		std::string pi = "size is zero";
		std::wstring stemp = std::wstring(pi.begin(), pi.end());
		LPCWSTR sw = stemp.c_str();
		OutputDebugString(sw);
	}
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::int32_t);
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

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

	geo->DrawArgs["boxModel"] = submesh;
	mGeometries[geo->Name] = std::move(geo);

}

void MainApp::BuildPSOs()
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
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	opaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
	opaquePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));
	
	opaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	opaquePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	//
	// PSO for drawing normals.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalsPsoDesc = opaquePsoDesc;
	drawNormalsPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["drawNormalsVS"]->GetBufferPointer()),
		mShaders["drawNormalsVS"]->GetBufferSize()
	};
	drawNormalsPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["drawNormalsPS"]->GetBufferPointer()),
		mShaders["drawNormalsPS"]->GetBufferSize()
	};
	drawNormalsPsoDesc.RTVFormats[0] = Ssao::NormalMapFormat;
	drawNormalsPsoDesc.SampleDesc.Count = 1;
	drawNormalsPsoDesc.SampleDesc.Quality = 0;
	drawNormalsPsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&drawNormalsPsoDesc, IID_PPV_ARGS(&mPSOs["drawNormals"])));


	//
	// PSO for SSAO.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = opaquePsoDesc;
	ssaoPsoDesc.InputLayout = { nullptr, 0 };
	ssaoPsoDesc.pRootSignature = mSsaoRootSignature.Get();
	ssaoPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["ssaoVS"]->GetBufferPointer()),
		mShaders["ssaoVS"]->GetBufferSize()
	};
	ssaoPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["ssaoPS"]->GetBufferPointer()),
		mShaders["ssaoPS"]->GetBufferSize()
	};

	// SSAO effect does not need the depth buffer.
	ssaoPsoDesc.DepthStencilState.DepthEnable = false;
	ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	ssaoPsoDesc.RTVFormats[0] = Ssao::AmbientMapFormat;
	ssaoPsoDesc.SampleDesc.Count = 1;
	ssaoPsoDesc.SampleDesc.Quality = 0;
	ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&mPSOs["ssao"])));

	//
	// PSO for SSAO blur.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoBlurPsoDesc = ssaoPsoDesc;
	ssaoBlurPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["ssaoBlurVS"]->GetBufferPointer()),
		mShaders["ssaoBlurVS"]->GetBufferSize()
	};
	ssaoBlurPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["ssaoBlurPS"]->GetBufferPointer()),
		mShaders["ssaoBlurPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&ssaoBlurPsoDesc, IID_PPV_ARGS(&mPSOs["ssaoBlur"])));

    //
    // PSO for shadow map pass.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = opaquePsoDesc;
    smapPsoDesc.RasterizerState.DepthBias = 100000;
    smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    smapPsoDesc.pRootSignature = mRootSignature.Get();
    smapPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["shadowVS"]->GetBufferPointer()),
        mShaders["shadowVS"]->GetBufferSize()
    };
    smapPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["shadowOpaquePS"]->GetBufferPointer()),
        mShaders["shadowOpaquePS"]->GetBufferSize()
    };
    
    // Shadow map pass does not have a render target.
    smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    smapPsoDesc.NumRenderTargets = 0;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadow_opaque"])));

    //
    // PSO for debug layer.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = opaquePsoDesc;
    debugPsoDesc.pRootSignature = mRootSignature.Get();
    debugPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["debugVS"]->GetBufferPointer()),
        mShaders["debugVS"]->GetBufferSize()
    };
    debugPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["debugPS"]->GetBufferPointer()),
        mShaders["debugPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&mPSOs["debug"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC editorPsoDesc = debugPsoDesc;
	editorPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["editorVS"]->GetBufferPointer()),
		mShaders["editorVS"]->GetBufferSize()
	};
	editorPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["editorPS"]->GetBufferPointer()),
		mShaders["editorPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&editorPsoDesc, IID_PPV_ARGS(&mPSOs["EditorGUI"])));


	D3D12_BLEND_DESC textBlendStateDesc = {};
	textBlendStateDesc.AlphaToCoverageEnable = FALSE;
	textBlendStateDesc.IndependentBlendEnable = FALSE;
	textBlendStateDesc.RenderTarget[0].BlendEnable = TRUE;

	textBlendStateDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	textBlendStateDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;// D3D12_BLEND_ONE;
	textBlendStateDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	textBlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	textBlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	textBlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	textBlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC fontPsoDesc = debugPsoDesc;
	fontPsoDesc.BlendState = textBlendStateDesc;
	D3D12_DEPTH_STENCIL_DESC textDepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	textDepthStencilDesc.DepthEnable = false;
	fontPsoDesc.DepthStencilState = textDepthStencilDesc;
	fontPsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["fontVS"]->GetBufferPointer()),
		mShaders["fontVS"]->GetBufferSize()
	};
	fontPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["fontPS"]->GetBufferPointer()),
		mShaders["fontPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&fontPsoDesc, IID_PPV_ARGS(&mPSOs["FontShader"])));


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

void MainApp::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            3, 200, (UINT)mMaterials.size(),4));
    }

	
}

void MainApp::BuildMaterials()
{
    auto bricks0 = std::make_unique<Material>();
    bricks0->Name = "bricks0";
    bricks0->MatCBIndex = 0;
    bricks0->DiffuseSrvHeapIndex = 0;
    bricks0->NormalSrvHeapIndex = 1;
    bricks0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    bricks0->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
    bricks0->Roughness = 0.3f;

    auto tile0 = std::make_unique<Material>();
    tile0->Name = "tile0";
    tile0->MatCBIndex = 1;
    tile0->DiffuseSrvHeapIndex = 2;
    tile0->NormalSrvHeapIndex = 3;
    tile0->DiffuseAlbedo = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
    tile0->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
    tile0->Roughness = 0.1f;

    auto mirror0 = std::make_unique<Material>();
    mirror0->Name = "mirror0";
    mirror0->MatCBIndex = 2;
    mirror0->DiffuseSrvHeapIndex = 4;
    mirror0->NormalSrvHeapIndex = 5;
    mirror0->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mirror0->FresnelR0 = XMFLOAT3(0.98f, 0.97f, 0.95f);
    mirror0->Roughness = 0.1f;

    auto skullMat = std::make_unique<Material>();
    skullMat->Name = "skullMat";
    skullMat->MatCBIndex = 3;
    skullMat->DiffuseSrvHeapIndex = 4;
    skullMat->NormalSrvHeapIndex = 5;
    skullMat->DiffuseAlbedo = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    skullMat->FresnelR0 = XMFLOAT3(0.6f, 0.6f, 0.6f);
    skullMat->Roughness = 0.2f;

    auto sky = std::make_unique<Material>();
    sky->Name = "sky";
    sky->MatCBIndex = 4;
    sky->DiffuseSrvHeapIndex = 6;
    sky->NormalSrvHeapIndex = 7;
    sky->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    sky->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
    sky->Roughness = 1.0f;
	
	auto asphalt = std::make_unique<Material>();
	asphalt->Name = "asphalt";
	asphalt->MatCBIndex = 5;
	asphalt->DiffuseSrvHeapIndex = 6;
	asphalt->NormalSrvHeapIndex = 7;
	asphalt->DiffuseAlbedo = XMFLOAT4(.8f, .8f, .8f, 1.0f);
	asphalt->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	asphalt->Roughness = 1.0f;

	auto manMat = std::make_unique<Material>();
	manMat->Name = "manMat";
	manMat->MatCBIndex = 6;
	manMat->DiffuseSrvHeapIndex = 8;
	manMat->NormalSrvHeapIndex = 9;
	manMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	manMat->FresnelR0 = XMFLOAT3(0.2f, 0.2f, 0.2f);
	manMat->Roughness = .9f;
	
    mMaterials["bricks0"] = std::move(bricks0);
    mMaterials["tile0"] = std::move(tile0);
    mMaterials["mirror0"] = std::move(mirror0);
    mMaterials["skullMat"] = std::move(skullMat);
    mMaterials["sky"] = std::move(sky);
	mMaterials["asphalt"] = std::move(asphalt);
	mMaterials["manMat"] = std::move(manMat);
}

void MainApp::BuildRenderItems()
{
	auto skyRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skyRitem->TexTransform = MathHelper::Identity4x4();
	skyRitem->ObjCBIndex = 0;
	skyRitem->Mat = mMaterials["sky"].get();
	skyRitem->Geo = mGeometries["shapeGeo"].get();
	skyRitem->InstanceCount = 1;
	skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skyRitem->IndexCount = skyRitem->Geo->DrawArgs["sphere"].IndexCount;
	skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
	skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
	skyRitem->instanceBufferIndex = 0;

	skyRitem->Instances.resize(1);
	skyRitem->Instances[0].MaterialIndex = 4;
	XMStoreFloat4x4(&skyRitem->Instances[0].World, XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	skyRitem->Instances[0].TexTransform = MathHelper::Identity4x4();
	mRitemLayer[(int)RenderLayer::Sky].push_back(skyRitem.get());
	mAllRitems.push_back(std::move(skyRitem));
	
    auto quadRitem = std::make_unique<RenderItem>();
	
    quadRitem->World = MathHelper::Identity4x4();
    quadRitem->TexTransform = MathHelper::Identity4x4();
    quadRitem->ObjCBIndex = 1;
	quadRitem->InstanceCount = 1;
    quadRitem->Mat = mMaterials["bricks0"].get();
    quadRitem->Geo = mGeometries["shapeGeo"].get();
    quadRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    quadRitem->IndexCount = quadRitem->Geo->DrawArgs["quad"].IndexCount;
    quadRitem->StartIndexLocation = quadRitem->Geo->DrawArgs["quad"].StartIndexLocation;
    quadRitem->BaseVertexLocation = quadRitem->Geo->DrawArgs["quad"].BaseVertexLocation;
	quadRitem->Instances.resize(1);
	quadRitem->instanceBufferIndex = 1;
	quadRitem->bIs2D = true;
	quadRitem->Instances[0].MaterialIndex = 0;
	quadRitem->Instances[0].TexTransform = MathHelper::Identity4x4();

	XMStoreFloat4x4(&quadRitem->Instances[0].World, XMMatrixScaling(1.0f, 1.0f, 1.0f));	
	mRitemLayer[(int)RenderLayer::Debug].push_back(quadRitem.get());	
	mAllRitems.push_back(std::move(quadRitem));
	
	/*
	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 1.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	XMStoreFloat4x4(&boxRitem->TexTransform, XMMatrixScaling(1.0f, 0.5f, 1.0f));
	boxRitem->ObjCBIndex = 2;
	boxRitem->Mat = mMaterials["bricks0"].get();
	boxRitem->Geo = mGeometries["shapeGeo"].get();
	boxRitem->InstanceCount = 1;
	boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
	boxRitem->instanceBufferIndex = 2;

	boxRitem->Instances.resize(1);
	boxRitem->Instances[0].MaterialIndex = 0;
	
	XMStoreFloat4x4(&boxRitem->Instances[0].TexTransform, XMMatrixScaling(1.0f, 0.5f, 1.0f));
	XMStoreFloat4x4(&boxRitem->Instances[0].World, XMMatrixScaling(2.0f, 1.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem.get());
	mAllRitems.push_back(std::move(boxRitem));
	*/
	/*
	auto skullRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&skullRitem->World, XMMatrixScaling(0.4f, 0.4f, 0.4f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f));
    skullRitem->TexTransform = MathHelper::Identity4x4();
    skullRitem->ObjCBIndex = 3;
    skullRitem->Mat = mMaterials["skullMat"].get();
    skullRitem->Geo = mGeometries["skullGeo"].get();
	skullRitem->InstanceCount = 1;
	skullRitem->instanceBufferIndex = 3;
    skullRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    skullRitem->IndexCount = skullRitem->Geo->DrawArgs["skull"].IndexCount;
    skullRitem->StartIndexLocation = skullRitem->Geo->DrawArgs["skull"].StartIndexLocation;
    skullRitem->BaseVertexLocation = skullRitem->Geo->DrawArgs["skull"].BaseVertexLocation;

	skullRitem->Instances.resize(1);
	skullRitem->Instances[0].MaterialIndex = 3;
	skullRitem->Instances[0].TexTransform = MathHelper::Identity4x4();
	XMStoreFloat4x4(&skullRitem->Instances[0].World, XMMatrixScaling(0.4f, 0.4f, 0.4f)*XMMatrixTranslation(0.0f, 1.0f, 0.0f));
    mRitemLayer[(int)RenderLayer::Opaque].push_back(skullRitem.get());
    mAllRitems.push_back(std::move(skullRitem));
	*/
	auto gridRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&gridRitem->World,XMMatrixTranslation(0,20,0));
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
	gridRitem->ObjCBIndex = 4;
	gridRitem->Mat = mMaterials["asphalt"].get();
	gridRitem->Geo = mGeometries["shapeGeo"].get();
	gridRitem->InstanceCount = 1;
	gridRitem->instanceBufferIndex = 2;
	gridRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	
	gridRitem->Instances.resize(1);
	gridRitem->Instances[0].MaterialIndex = 5;
	XMStoreFloat4x4(&gridRitem->Instances[0].TexTransform, XMMatrixScaling(8.0f, 8.0f, 1.0f));
	gridRitem->Instances[0].World = MathHelper::Identity4x4();
	

	mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());
	mAllRitems.push_back(std::move(gridRitem));

	
	

	auto leftCylRitem = std::make_unique<RenderItem>();
	XMMATRIX brickTexTransform = XMMatrixScaling(1.5f, 2.0f, 1.0f);
	UINT objCBIndex = 5;
	leftCylRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&leftCylRitem->TexTransform, brickTexTransform);
	leftCylRitem->ObjCBIndex = objCBIndex;
	leftCylRitem->Mat = mMaterials["bricks0"].get();
	leftCylRitem->Geo = mGeometries["shapeGeo"].get();
	leftCylRitem->InstanceCount = 4;
	leftCylRitem->instanceBufferIndex = 3;
	leftCylRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
	leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
	leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;


	leftCylRitem->Instances.resize(4);
	leftCylRitem->InstanceCount = 4;
	for (int i = 0; i < 4; ++i)
	{
		XMStoreFloat4x4(&leftCylRitem->Instances[i].World, XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		leftCylRitem->Instances[i].MaterialIndex = 0;
		XMStoreFloat4x4(&leftCylRitem->Instances[i].TexTransform, XMMatrixScaling(1.5f, 2.0f, 1.0f));
	}
	mRitemLayer[(int)RenderLayer::Opaque].push_back(leftCylRitem.get());
	mAllRitems.push_back(std::move(leftCylRitem));
	/*
	auto testObject = std::make_unique<ImmerseObject>(
		"testBox",
		XMMatrixScaling(2.0f, 2.0f, 2.0f)*XMMatrixTranslation(0.0f, 4.0f, 0.0f),
		XMMatrixScaling(1.0f, 1.0f, 1.0f),
		mMaterials["bricks0"].get(),
		mGeometries["shapeGeo"].get(),
		mGeometries["shapeGeo"].get()->DrawArgs["box"].IndexCount,
		mGeometries["shapeGeo"].get()->DrawArgs["box"].StartIndexLocation,
		mGeometries["shapeGeo"].get()->DrawArgs["box"].BaseVertexLocation,
		0,
		1
		);
		*/
	/*
	auto testObject = std::make_unique<test>();
	testObject->name = "testBox";
	XMStoreFloat4x4(&testObject->World, XMMatrixScaling(2.0f, 1.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	XMStoreFloat4x4(&testObject->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	testObject->Mat = mMaterials["bricks0"].get();
	testObject->Geo = mGeometries["shapeGeo"].get();
	testObject->IndexCount = testObject->Geo->DrawArgs["box"].IndexCount;
	testObject->StartIndexLocation = testObject->Geo->DrawArgs["box"].StartIndexLocation;
	testObject->BaseVertexLocation = testObject->Geo->DrawArgs["box"].BaseVertexLocation;
	testObject->MatIndex = 0;
	testObject->InstanceCount = 1;
	testObject->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	testObject->Instances.resize(1);
	testObject->Instances[0].MaterialIndex = 0;
	XMStoreFloat4x4(&testObject->Instances[0].TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	XMStoreFloat4x4(&testObject->Instances[0].World, XMMatrixScaling(2.0f, 1.0f, 2.0f)*XMMatrixTranslation(0.0f, 0.5f, 0.0f));
	mAllImmerseObjects.push_back(testObject.get());
	mImmerseObjects.push_back(std::move(testObject));
	*/
	/*
	auto wallRitem = std::make_unique<RenderItem>();
	wallRitem->World = MathHelper::Identity4x4();
	//XMStoreFloat4x4(&wallRitem->World, XMMatrixTranslation(5, 1, 0));
	wallRitem->TexTransform = MathHelper::Identity4x4();
	wallRitem->Mat = mMaterials["bricks0"].get();
	wallRitem->Geo = mGeometries["shapeGeo"].get();
	wallRitem->InstanceCount = 4;
	wallRitem->instanceBufferIndex = 6;
	wallRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wallRitem->IndexCount = wallRitem->Geo->DrawArgs["wall"].IndexCount;
	wallRitem->StartIndexLocation = wallRitem->Geo->DrawArgs["wall"].StartIndexLocation;
	wallRitem->BaseVertexLocation = wallRitem->Geo->DrawArgs["wall"].BaseVertexLocation;
	wallRitem->Instances.resize(wallRitem->InstanceCount);
	XMStoreFloat4x4(&wallRitem->Instances[0].TexTransform, XMMatrixScaling(4, 3.5, 1));
	XMStoreFloat4x4(&wallRitem->Instances[0].World, XMMatrixTranslation(0, 1, 15) * XMMatrixScaling(4,3,1));
	wallRitem->Instances[0].MaterialIndex = 0;
	XMStoreFloat4x4(&wallRitem->Instances[1].TexTransform, XMMatrixScaling(4, 3.5, 1));
	XMStoreFloat4x4(&wallRitem->Instances[1].World, XMMatrixTranslation(50, 1, 0) * XMMatrixScaling(.2f, 3, 30));
	wallRitem->Instances[1].MaterialIndex = 0;
	XMStoreFloat4x4(&wallRitem->Instances[2].TexTransform, XMMatrixScaling(4, 3.5, 1));
	XMStoreFloat4x4(&wallRitem->Instances[2].World, XMMatrixTranslation(-50, 1, 0) * XMMatrixScaling(.2f, 3, 30));
	wallRitem->Instances[2].MaterialIndex = 0;
	XMStoreFloat4x4(&wallRitem->Instances[3].TexTransform, XMMatrixScaling(4, 3.5, 1));
	XMStoreFloat4x4(&wallRitem->Instances[3].World, XMMatrixTranslation(0, 1, -15) * XMMatrixScaling(4, 3, 1));
	wallRitem->Instances[3].MaterialIndex = 0;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem.get());
	mAllRitems.push_back(std::move(wallRitem));
	*/

	/*
	auto boxModelRitem = std::make_unique<RenderItem>();
	boxModelRitem->World = MathHelper::Identity4x4();
	boxModelRitem->TexTransform = MathHelper::Identity4x4();
	boxModelRitem->Mat = mMaterials["manMat"].get();
	boxModelRitem->Geo = mGeometries["boxGeo"].get();
	boxModelRitem->IndexCount = boxModelRitem->Geo->DrawArgs["boxModel"].IndexCount;
	boxModelRitem->StartIndexLocation = boxModelRitem->Geo->DrawArgs["boxModel"].StartIndexLocation;
	boxModelRitem->BaseVertexLocation = boxModelRitem->Geo->DrawArgs["boxModel"].BaseVertexLocation;
	boxModelRitem->InstanceCount = 1;
	boxModelRitem->instanceBufferIndex = 4;
	boxModelRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxModelRitem->Instances.resize(1);
	boxModelRitem->Instances[0].MaterialIndex = 6;
	boxModelRitem->Instances[0].TexTransform = MathHelper::Identity4x4();
	XMStoreFloat4x4(&boxModelRitem->Instances[0].World, XMMatrixTranslation(0, -120, 0) * XMMatrixRotationRollPitchYaw(XMConvertToRadians(-90), XMConvertToRadians(180), 0) *XMMatrixScaling(.02, .02, .02));
	
	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxModelRitem.get());
	mAllRitems.push_back(std::move(boxModelRitem));
	*/
	
	std::string pi = "There are " + std::to_string((int)mAllRitems.size()) + " render items" ;
	std::wstring stemp = std::wstring(pi.begin(), pi.end());
	LPCWSTR sw = stemp.c_str();
	OutputDebugString(sw);

}

void MainApp::DrawEditorGUI(ID3D12GraphicsCommandList * cmdList)
{
	for (UINT i = 0; i < (UINT)mainGUIgeometry->subGeos.size(); i++)
	{

		auto gui = mainGUIgeometry->subGeos[i];
		//auto gui = mainGUIgeometry.get()->subGeos[i];
		
		auto GUIdataBuffer = mCurrFrameResource->GUIdataBuffers[gui->myGUI->bufferIndex]->Resource();
		cmdList->IASetVertexBuffers(0, 1, &mainGUIgeometry->VertexBufferView());
		cmdList->IASetIndexBuffer(&mainGUIgeometry->IndexBufferView());
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->SetGraphicsRootShaderResourceView(5, GUIdataBuffer->GetGPUVirtualAddress());


		cmdList->DrawIndexedInstanced(gui->IndexCount, 1, gui->StartIndexLocation, gui->BaseVertexLocation, 0);
		
		
	}
}

void MainApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{

	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];
		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
		auto instanceBuffer = mCurrFrameResource->renderItemBuffers[ri->instanceBufferIndex]->Resource();
		cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());
		cmdList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	
	}
	

}


void MainApp::DrawImmerseObjects(ID3D12GraphicsCommandList* cmdList, const std::vector<ImmerseObject*>& iObjects)
{
	for (size_t i = 0; i < iObjects.size(); ++i)
	{
		
		auto iO = iObjects[i];

		cmdList->IASetVertexBuffers(0, 1, &iO->Geo->VertexBufferView());

		cmdList->IASetIndexBuffer(&iO->Geo->IndexBufferView());

		cmdList->IASetPrimitiveTopology(iO->PrimitiveType);

		auto instanceBuffer = mCurrFrameResource->ImmerseObjectBuffer->Resource();

		cmdList->SetGraphicsRootShaderResourceView(0, instanceBuffer->GetGPUVirtualAddress());

		cmdList->DrawIndexedInstanced(iO->IndexCount, iO->InstanceCount, iO->StartIndexLocation, iO->BaseVertexLocation, 0);
		
		
	}
}



void MainApp::DrawSceneToShadowMap()
{
    mCommandList->RSSetViewports(1, &mShadowMap->Viewport());
    mCommandList->RSSetScissorRects(1, &mShadowMap->ScissorRect());

    // Change to DEPTH_WRITE.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->Resource(),
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearDepthStencilView(mShadowMap->Dsv(), 
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Set null render target because we are only going to draw to
    // depth buffer.  Setting a null render target will disable color writes.
    // Note the active PSO also must specify a render target count of 0.
    mCommandList->OMSetRenderTargets(0, nullptr, false, &mShadowMap->Dsv());

    // Bind the pass constant buffer for the shadow map pass.
    auto passCB = mCurrFrameResource->PassCB->Resource();
    D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1*passCBByteSize;
    mCommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);

    mCommandList->SetPipelineState(mPSOs["shadow_opaque"].Get());

    DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);
	DrawImmerseObjects(mCommandList.Get(), mAllImmerseObjects);


    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap->Resource(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	

	
}

void MainApp::DrawNormalsAndDepth()
{
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	auto normalMap = mSsao->NormalMap();
	auto normalMapRtv = mSsao->NormalMapRtv();


	
	// Change to RENDER_TARGET.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the screen normal map and depth buffer.
	float clearValue[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	mCommandList->ClearRenderTargetView(normalMapRtv, clearValue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &normalMapRtv, true, &DepthStencilView());
	
	// Bind the constant buffer for this pass.
	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

	mCommandList->SetPipelineState(mPSOs["drawNormals"].Get());

	

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);
	DrawImmerseObjects(mCommandList.Get(), mAllImmerseObjects);

	// Change back to GENERIC_READ so we can read the texture in a shader.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(normalMap,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));

}

void MainApp::BuildEditorGUI()
{
	PanelGUI* leftPanel = new PanelGUI(-0.9f, .9f, 0.4f, 1.0f);
	leftPanel->bufferIndex = 0;
	leftPanel->color = DirectX::XMFLOAT4{ 1.0f,1.0f,0.0f,1.0f };
	GUIsubGeometry* leftPanelSubGeo = new GUIsubGeometry();
	leftPanelSubGeo->BaseVertexLocation = 0;
	leftPanelSubGeo->StartIndexLocation = 0;
	leftPanelSubGeo->IndexCount = (UINT)leftPanel->meshData.Indices32.size();
	PanelGUI* tempLeftPanel = leftPanel;
	leftPanelSubGeo->myGUI = static_cast<BaseGUI*>(tempLeftPanel);
	
	PanelGUI* rightPanel = new PanelGUI(0.1f, 0.9f, 0.4f, 1.0f);
	rightPanel->bufferIndex = 1;
	rightPanel->color = DirectX::XMFLOAT4{ 0.0f,1.0f,0.0f,1.0f };
	GUIsubGeometry* rightPanelSubGeo = new GUIsubGeometry();
	rightPanelSubGeo->BaseVertexLocation = (UINT)leftPanel->meshData.Vertices.size();
	rightPanelSubGeo->StartIndexLocation = (UINT)leftPanel->meshData.Indices32.size();
	rightPanelSubGeo->IndexCount = (UINT)rightPanel->meshData.Indices32.size();
	
	PanelGUI* tempRightPanel = rightPanel;
	rightPanelSubGeo->myGUI = static_cast<BaseGUI*>(tempRightPanel);


	

	UINT vertexCount = (UINT)(leftPanel->meshData.Vertices.size() + rightPanel->meshData.Vertices.size());

	std::vector<Vertex> vertices(vertexCount);

	UINT k = 0;
	for (size_t i = 0; i < (UINT)leftPanel->meshData.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = leftPanel->meshData.Vertices[i].Position;
		vertices[k].Normal = leftPanel->meshData.Vertices[i].Normal;
		vertices[k].TexC = leftPanel->meshData.Vertices[i].TexC;
		vertices[k].TangentU = leftPanel->meshData.Vertices[i].TangentU;
	}
	for (size_t i = 0; i < (UINT)rightPanel->meshData.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = rightPanel->meshData.Vertices[i].Position;
		vertices[k].Normal = rightPanel->meshData.Vertices[i].Normal;
		vertices[k].TexC = rightPanel->meshData.Vertices[i].TexC;
		vertices[k].TangentU = rightPanel->meshData.Vertices[i].TangentU;
	}

	std::vector<std::uint16_t> indices;// ((UINT)(leftPanel.get()->meshData.GetIndices16().size() + rightPanel.get()->meshData.GetIndices16().size()));
	indices.insert(indices.end(), std::begin(leftPanel->meshData.GetIndices16()), std::end(leftPanel->meshData.GetIndices16()));
	indices.insert(indices.end(), std::begin(rightPanel->meshData.GetIndices16()), std::end(rightPanel->meshData.GetIndices16()));
	UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	
	mainGUIgeometry->VertexBufferCPU = nullptr;
	//ThrowIfFailed(D3DCreateBlob(vbByteSize, &mainGUIgeometry->VertexBufferCPU));
	//CopyMemory(mainGUIgeometry->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mainGUIgeometry->IndexBufferCPU));
	CopyMemory(mainGUIgeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	mainGUIgeometry->VertexBufferGPU = nullptr;
	//mainGUIgeometry->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		//mCommandList.Get(), vertices.data(), vbByteSize, mainGUIgeometry->VertexBufferUploader);

	mainGUIgeometry->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mainGUIgeometry->IndexBufferUploader);

	mainGUIgeometry->VertexByteStride = sizeof(Vertex);
	mainGUIgeometry->VertexBufferByteSize = vbByteSize;
	mainGUIgeometry->IndexFormat = DXGI_FORMAT_R16_UINT;;
	mainGUIgeometry->IndexBufferByteSize = ibByteSize;

	mainGUIgeometry->subGeos.push_back(std::move(leftPanelSubGeo));
	mainGUIgeometry->subGeos.push_back(std::move(rightPanelSubGeo));
	/*
	std::vector<Vertex> verts = CalculateVerts();
	auto currGUIVB = mCurrFrameResource->EditorGUIVB.get();
	for (UINT i = 0; i < (UINT)verts.size();i++)
	{
		Vertex vert = verts[i];
		currGUIVB->CopyData(i, vert);
	}

	mainGUIgeometry->VertexBufferGPU = currGUIVB->Resource();
	*/
}

void MainApp::CreateMainFont()
{
	
	//std::string p = "Penis";
	//XMFLOAT2 position = XMFLOAT2(-.2f, .2f);
	std::vector<std::uint16_t> indices;
	/*
	float fontSize = 5;
	float xWidth = fontSize / 142.857142857f;
	float yHeight = fontSize / 100.0f;
	
	for (char c : p)
	{
		XMFLOAT4 test = mainFont->MapGlyphQuad(c);

		Vertex vert1;
		float u = test.x / (float)fontTexture->Resource->GetDesc().Width;
		float v = test.y  / (float)fontTexture->Resource->GetDesc().Height;
		float h = test.w / (float)fontTexture->Resource->GetDesc().Height;
		float w = test.z / (float)fontTexture->Resource->GetDesc().Width;
		vert1.TexC = XMFLOAT2(u, v);

		vert1.Pos = XMFLOAT3(position.x, position.y,0.0f);
		/*
		std::string output = "(" + std::to_string(u);
		output += "," + std::to_string(v);
		output += ") ";
		
		std::wstring temp(output.begin(), output.end());
		OutputDebugStringW(temp.c_str());
		*/
	/*
		Vertex vert2;
		vert2.TexC = XMFLOAT2(u + w, v);
		vert2.Pos = XMFLOAT3(position.x + xWidth, position.y, 0.0f);
		Vertex vert3;
		vert3.TexC = XMFLOAT2(u, v + h);
		vert3.Pos = XMFLOAT3(position.x, position.y - yHeight, 0.0f);
		Vertex vert4;
		vert4.TexC = XMFLOAT2(u + w, v + h);
		vert4.Pos = XMFLOAT3(position.x + xWidth, position.y - yHeight, 0.0f);

		fontVertices.push_back(vert1);
		fontVertices.push_back(vert2);
		fontVertices.push_back(vert3);
		fontVertices.push_back(vert4);

		position.x += xWidth;

	}
	*/
	for (int i = 0; i < 800; i++)
	{
		indices.push_back(mainFont->currentStartingIndex + 0);
		indices.push_back(mainFont->currentStartingIndex + 1);
		indices.push_back(mainFont->currentStartingIndex + 2);
		indices.push_back(mainFont->currentStartingIndex + 1);
		indices.push_back(mainFont->currentStartingIndex + 3);
		indices.push_back(mainFont->currentStartingIndex + 2);
		mainFont->currentStartingIndex += 4;
	}
	//mainFont->numCharCreated += (UINT)p.length();
	//mainFont->textLength = mainFont->numCharCreated * 6;
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
	UINT vbByteSize = (UINT)fontVertices.size() * sizeof(Vertex);
	mainFont->VertexBufferCPU = nullptr;


	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mainFont->IndexBufferCPU));
	CopyMemory(mainFont->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	mainFont->VertexBufferGPU = nullptr;


	mainFont->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mainFont->IndexBufferUploader);

	mainFont->VertexByteStride = sizeof(Vertex);
	mainFont->VertexBufferByteSize = vbByteSize;
	mainFont->IndexFormat = DXGI_FORMAT_R16_UINT;;
	mainFont->IndexBufferByteSize = ibByteSize;
	CreateText(std::string("hello"), XMFLOAT2(0.f, 0.f), 7.0f);
}

void MainApp::CreateText(std::string & text, XMFLOAT2 position, float fontSize, XMFLOAT4 color)
{
	float xWidth = fontSize / 142.857142857f;
	float yHeight = fontSize / 100.0f;
	auto parentGui = mainGUIgeometry->subGeos[0]->myGUI;
	ImmerseText* immerseText = new ImmerseText(text, 0, parentGui);
	
	XMFLOAT2 textPosition = XMFLOAT2(parentGui->x + position.x,parentGui->y + position.y); //position is offset from gui position
	for (int i = 0; i < text.length();i++)
	{
		char c = text.at(i);
		XMFLOAT4 test = mainFont->MapGlyphQuad(c);

		Vertex vert1;
		float u = test.x / (float)fontTexture->Resource->GetDesc().Width;
		float v = test.y / (float)fontTexture->Resource->GetDesc().Height;
		float h = test.w / (float)fontTexture->Resource->GetDesc().Height;
		float w = test.z / (float)fontTexture->Resource->GetDesc().Width;
		vert1.TexC = XMFLOAT2(u, v);

		vert1.Pos = XMFLOAT3(textPosition.x, textPosition.y, 0.0f);
		/*
		std::string output = "(" + std::to_string(u);
		output += "," + std::to_string(v);
		output += ") ";

		std::wstring temp(output.begin(), output.end());
		OutputDebugStringW(temp.c_str());
		*/
	
		Vertex vert2;
		vert2.TexC = XMFLOAT2(u + w, v);
		vert2.Pos = XMFLOAT3(textPosition.x + xWidth, textPosition.y, 0.0f);
		Vertex vert3;
		vert3.TexC = XMFLOAT2(u, v + h);
		vert3.Pos = XMFLOAT3(textPosition.x, textPosition.y - yHeight, 0.0f);
		Vertex vert4;
		vert4.TexC = XMFLOAT2(u + w, v + h);
		vert4.Pos = XMFLOAT3(textPosition.x + xWidth, textPosition.y - yHeight, 0.0f);
		
		immerseText->PushBackVertsPos(vert1.Pos);
		immerseText->PushBackVertsTex(vert1.TexC);
		immerseText->PushBackVertsPos(vert2.Pos);
		immerseText->PushBackVertsTex(vert2.TexC);
		immerseText->PushBackVertsPos(vert3.Pos);
		immerseText->PushBackVertsTex(vert3.TexC);
		immerseText->PushBackVertsPos(vert4.Pos);
		immerseText->PushBackVertsTex(vert4.TexC);
		/*
		immerseText->myVerticesPos.push_back(vert1.Pos);
		immerseText->myVerticesTex.push_back(vert1.TexC);
		immerseText->myVerticesPos.push_back(vert2.Pos);
		immerseText->myVerticesTex.push_back(vert2.TexC);
		immerseText->myVerticesPos.push_back(vert3.Pos);
		immerseText->myVerticesTex.push_back(vert3.TexC);
		immerseText->myVerticesPos.push_back(vert4.Pos);
		immerseText->myVerticesTex.push_back(vert4.TexC);
		*/

		fontVertices.push_back(vert1);
		fontVertices.push_back(vert2);
		fontVertices.push_back(vert3);
		fontVertices.push_back(vert4);

		textPosition.x += xWidth;

	}
	
	
	immerseText->vertexBufferIndex = textBufferLastVertexIndex;
	textBufferLastVertexIndex += (UINT)immerseText->myVerticesPos.size();
	immerseText->indexBufferIndex = textBufferLastIndiceIndex;
	immerseText->indexCount = (UINT)text.length() * 6;
	textBufferLastIndiceIndex += (UINT)text.length() * 6;
	UINT vbByteSize = (UINT)fontVertices.size() * sizeof(Vertex);
	mainFont->VertexBufferByteSize = vbByteSize;
	mainFont->numCharCreated += (UINT)text.length();
	mainFont->textLength = mainFont->numCharCreated * 6;
	parentGui->myTitle = immerseText;
	mImmerseTextObjects.push_back(std::move(immerseText));
	
}



void MainApp::DrawTextGUI(ID3D12GraphicsCommandList * cmdList)
{
	

	for (auto immerseText : mImmerseTextObjects)
	{
		cmdList->IASetVertexBuffers(0, 1, &mainFont->VertexBufferView());
		cmdList->IASetIndexBuffer(&mainFont->IndexBufferView());
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawIndexedInstanced(immerseText->indexCount, immerseText->instanceCount, immerseText->indexBufferIndex, (int)immerseText->vertexBufferIndex, 0);
	}
	//cmdList->DrawInstanced(4, 1, 0, 0);
		


	
}


CD3DX12_CPU_DESCRIPTOR_HANDLE MainApp::GetCpuSrv(int index)const
{
	auto srv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	srv.Offset(index, mCbvSrvUavDescriptorSize);
	return srv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE MainApp::GetGpuSrv(int index)const
{
	auto srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	srv.Offset(index, mCbvSrvUavDescriptorSize);
	return srv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE MainApp::GetDsv(int index)const
{
	auto dsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDsvHeap->GetCPUDescriptorHandleForHeapStart());
	dsv.Offset(index, mDsvDescriptorSize);
	return dsv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE MainApp::GetRtv(int index)const
{
	auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtv.Offset(index, mRtvDescriptorSize);
	return rtv;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> MainApp::GetStaticSamplers()
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

    const CD3DX12_STATIC_SAMPLER_DESC shadow(
        6, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return { 
		pointWrap, pointClamp,
		linearWrap, linearClamp, 
		anisotropicWrap, anisotropicClamp,
        shadow 
    };
}

std::vector<Vertex> MainApp::CalculateVerts()
{
	UINT totalVerts = 0;
	for (auto subGeo : mainGUIgeometry->subGeos)
	{
		totalVerts += (UINT)subGeo->myGUI->meshData.Vertices.size();
	}
	std::vector<Vertex> vertices(totalVerts);
	UINT currentVertIndex = 0;
	for (auto subGeo : mainGUIgeometry->subGeos)
	{
		for (UINT i = 0; i < subGeo->myGUI->meshData.Vertices.size(); i++, currentVertIndex++)
		{
			vertices[currentVertIndex].Pos = subGeo->myGUI->meshData.Vertices[i].Position;
			vertices[currentVertIndex].Normal = subGeo->myGUI->meshData.Vertices[i].Normal;
			vertices[currentVertIndex].TexC = subGeo->myGUI->meshData.Vertices[i].TexC;
			vertices[currentVertIndex].TangentU = subGeo->myGUI->meshData.Vertices[i].TangentU;
		}
	}
	return vertices;
	
}

