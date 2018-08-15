#pragma once
#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "ButtonGUI.h"
#include "Common/GeometryGenerator.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class PanelGUI
{
public:
	PanelGUI(float x, float y, float width, float height);
	std::vector<ButtonGUI*> buttons;
	bool visible = false;
	float x_pos;
	float y_pos;
	float width;
	float height;
	GeometryGenerator::MeshData meshData;

};