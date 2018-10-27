#pragma once
#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "BaseGUI.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class ImmerseText
{
public:
	ImmerseText(std::string& text = std::string("NULL"), UINT vertexBufferIndex = 0, UINT indexBufferIndex = 0, BaseGUI* ParentGUI = nullptr);
	bool bIsVisible = true;
	BaseGUI* ParentGUI = nullptr;
	std::string myText = "NULL";
	UINT vertexBufferIndex = 0;
	UINT vertexBufferEndIndex = 0;
	UINT indexBufferIndex = 0;
	UINT instanceCount = 1;
	UINT indexCount = 0;
	std::vector<XMFLOAT3> myVerticesPos;
	std::vector<XMFLOAT2> myVerticesTex;
	void PushBackVertsPos(XMFLOAT3 vert);
	void PushBackVertsTex(XMFLOAT2 vert);
	
};