#pragma once
#include "Editor.h"

Editor::Editor(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	this->device = device;
	this->cmdList = cmdList;
}

void Editor::Draw(const GameTimer & gt)
{

}

void Editor::Update(const GameTimer & gt)
{

}
