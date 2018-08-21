#pragma once
#include "BaseGUI.h"




using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class ButtonGUI : public BaseGUI
{
public:
	ButtonGUI(float width, float height,std::string label = "");
	~ButtonGUI() = default;
	void OnClicked() override;
	

private:
	
	std::string label;

};
