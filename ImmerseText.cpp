#include "ImmerseText.h"

ImmerseText::ImmerseText(std::string & text, UINT vertexBufferIndex, BaseGUI * ParentGUI)
{
	myText = text;
	this->ParentGUI = ParentGUI;
	this->vertexBufferIndex = vertexBufferIndex;
}

void ImmerseText::PushBackVertsPos(XMFLOAT3 vert)
{
	myVerticesPos.push_back(vert);
}

void ImmerseText::PushBackVertsTex(XMFLOAT2 vert)
{
	myVerticesTex.push_back(vert);
}
