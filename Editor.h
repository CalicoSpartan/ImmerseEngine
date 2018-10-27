#pragma once
#include "BaseGUI.h"
#include "ImmerseFont.h"
#include "ImmerseText.h"
#include "PanelGUI.h"
#include "Common/d3dApp.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
struct FrameResource;
struct Vertex;
struct GUIgeometry;
struct GUIsubGeometry;
class Editor
{
public:
	Editor(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,D3DApp* mainApp);
	std::unique_ptr<PanelGUI> rightMainPanel;
	std::unique_ptr<ScrollBoxGUI> rMPScroll1;
	std::unique_ptr<ButtonGUI> rMPButton1;
	ID3D12Device* device;
	FrameResource* currentFrameResource;
	ID3D12GraphicsCommandList* cmdList;
	std::vector<Vertex> mAllGUIGeometryVertices;
	std::vector<Vertex> fontVertices;
	std::vector<BaseGUI*> mAllGUI;
	std::unique_ptr<ImmerseFont> mainFont;
	std::unique_ptr<Texture> fontTexture;
	std::unique_ptr<GUIgeometry> mainGUIgeometry;
	std::vector<ImmerseText*> mImmerseTextObjects;
	D3D12_VIEWPORT* mScreenViewport;
	ID3D12PipelineState* GUIPSO;
	ID3D12PipelineState* textPSO;
	UINT textBufferLastIndiceIndex = 0;
	UINT textBufferLastVertexIndex = 0;
	UINT GUIVertexBufferOpenIndex = 0;
	UINT GUIIndexBufferOpenIndex = 0;
	UINT TextVertexBufferOpenIndex = 0;
	bool focusedOnScrollBox = false;
	ScrollBoxGUI* focusedScrollBoxGUI = nullptr;
	D3DApp* mainApp = nullptr;
	
private:

public:
	void Draw(const GameTimer& gt);
	void Update(const GameTimer& gt);
	void Initialize();
	void BuildSubGUI();
	void SetCurrentFrameResource(FrameResource* frameResource);
	void CreateMainFont();
	void DrawTextGUI();
	void DrawEditorGUI();
	std::unordered_map<std::string, GUIsubGeometry*> mAllGUIsUnorderedMap;
	ImmerseText* CreateText(std::string & text, XMFLOAT2 position, float fontSize,BaseGUI* parentGUI = nullptr, XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	void SetPSOs(ID3D12PipelineState* GUIPSO, ID3D12PipelineState* textPSO);
	void CheckGUIInteraction(int x,int y);
	void CheckGUIInteraction(bool scrollUp);
	void UpdateVertexBuffer(GUIsubGeometry* subGeo);
	void UpdateScrollBox(ScrollBoxGUI* scrollboxGUI);
};