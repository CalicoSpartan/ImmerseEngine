#pragma once
#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class ButtonGUI
{
public:
	ButtonGUI(float width, float height,std::string label = "");
	~ButtonGUI() = default;
	void OnClicked();
	

private:
	XMFLOAT4 color;
	float width;
	float height;
	std::string label;

};
