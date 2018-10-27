#pragma once
#include "BaseGUI.h"




using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
class ScrollBoxGUI : public BaseGUI
{
public:
	ScrollBoxGUI(float x, float y, float width, float height);
	~ScrollBoxGUI() = default;
	void OnClicked() override;
	void OnInvisible(bool bVisible) override;
	int scrollValue = 0;
	std::string stringValue = "";
	void ChangeScrollValue(int change);
private:

	

};