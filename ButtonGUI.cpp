#include "ButtonGUI.h"
#include "ImmerseText.h"

ButtonGUI::ButtonGUI(float x, float y, float width, float height)
{
	this->width = width;
	this->height = height;
	this->x = x;
	this->y = y;

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
	instanceCount = 1;
}

void ButtonGUI::OnClicked()
{

}

void ButtonGUI::OnInvisible(bool bVisible)
{
	bIsVisible = bVisible;

	if (myTitle != nullptr)
	{
		myTitle->bIsVisible = bVisible;
	}
}
