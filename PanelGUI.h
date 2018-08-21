#pragma once
#include "BaseGUI.h"
#include "ButtonGUI.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

class PanelGUI : public BaseGUI
{
public:
	PanelGUI(float x, float y, float width, float height);
	std::vector<ButtonGUI*> buttons;
	bool visible = false;
	float collapsedHeight;
	float originalHeight;
	bool bIsClosed = true;
	void OnClicked()  override {};
	void ChangeSize(bool bClose);

};