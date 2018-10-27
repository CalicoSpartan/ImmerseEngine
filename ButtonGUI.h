#pragma once
#include "BaseGUI.h"




using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class ButtonGUI : public BaseGUI
{
public:
	ButtonGUI(float x, float y, float width, float height);
	~ButtonGUI() = default;
	void OnClicked() override;
	void OnInvisible(bool bVisible) override;


private:
	


};
