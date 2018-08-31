#pragma once
#include "EditorGUIincludes.h"
#include "Common/d3dApp.h"

class Editor
{
public:
	Editor(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
	PanelGUI* rightMainPanel;
	ID3D12Device* device;
	ID3D12GraphicsCommandList* cmdList;
	void Draw(const GameTimer& gt);
	void Update(const GameTimer& gt);

};