#pragma once
#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"

#include "Common/GeometryGenerator.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class BaseGUI
{
public:
	virtual void OnClicked() {};
	//set x and y as the meshData.vertices[1] for top right corner origin
	float x;
	float y;
	float width;
	float height;
	UINT bufferIndex;
	XMFLOAT2 position;
	DirectX::XMFLOAT4 color = { 0.0f,0.0f,0.0f,1.0f };
	GeometryGenerator::MeshData meshData;

};