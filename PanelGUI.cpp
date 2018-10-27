#include "PanelGUI.h"
#include "ImmerseText.h"

PanelGUI::PanelGUI(float x, float y, float width, float height)
{


	

	meshData.Vertices.resize(4);
	meshData.Indices32.resize(6);

	// Position coordinates specified in NDC space.
	meshData.Vertices[0] = GeometryGenerator::Vertex(
		x, y - height, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = GeometryGenerator::Vertex(
		x, y, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = GeometryGenerator::Vertex(
		x + width, y, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = GeometryGenerator::Vertex(
		x + width, y - height, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices32[0] = 0;
	meshData.Indices32[1] = 1;
	meshData.Indices32[2] = 2;

	meshData.Indices32[3] = 0;
	meshData.Indices32[4] = 2;
	meshData.Indices32[5] = 3;

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	collapsedHeight = .1f;
	originalHeight = height;
	position.x = x;
	position.y = y;
	ChangeSize(bIsClosed);
	instanceCount = 1;
	
}

void PanelGUI::OnInvisible(bool bVisible)
{
	bIsVisible = bVisible;

	if (myTitle != nullptr)
	{
		myTitle->bIsVisible = bVisible;
	}
}

void PanelGUI::ChangeSize(bool bClose)
{
	if (bClose)
	{
		height = collapsedHeight;
		meshData.Vertices[3].Position = XMFLOAT3(x + width, y - height, 0.0f);
		meshData.Vertices[0].Position = XMFLOAT3(x, y - height, 0.0f);
		bIsClosed = true;
		for (auto n : scrollBoxes)
		{
			
			n->OnInvisible(false);
		}
		for (auto n : buttons)
		{
			n->OnInvisible(false);
		}
		if (myTitle != nullptr)
		{
			//myTitle->bVisible = false;
		}
		
	}
	else
	{
		height = originalHeight;
		meshData.Vertices[3].Position = XMFLOAT3(x + width, y - height, 0.0f);
		meshData.Vertices[0].Position = XMFLOAT3(x, y - height, 0.0f);
		bIsClosed = false;
		for (auto n : scrollBoxes)
		{
			n->OnInvisible(true);
			
		}
		for (auto n : buttons)
		{
			n->OnInvisible(true);
		}
		if (myTitle != nullptr)
		{
			//myTitle->bVisible = true;
		}
	}
}
