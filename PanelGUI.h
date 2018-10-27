#pragma once
#include "BaseGUI.h"
#include "ButtonGUI.h"
#include "ScrollBoxGUI.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class ImmerseText;
class PanelGUI : public BaseGUI
{
public:
	PanelGUI(float x, float y, float width, float height);
	std::vector<ButtonGUI*> buttons;
	std::vector<ScrollBoxGUI*> scrollBoxes;
	bool visible = false;
	float collapsedHeight;
	float originalHeight;
	bool bIsClosed = true;
	void OnClicked()  override {};
	void OnInvisible(bool bVisible) override;
	void ChangeSize(bool bClose);

};