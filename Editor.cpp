#pragma once
#include "Editor.h"

Editor::Editor(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, D3DApp* mainApp)
{
	this->device = device;
	this->cmdList = cmdList;
	this->mainApp = mainApp;
	//Initialize();
}

void Editor::Initialize()
{
	mainGUIgeometry = std::make_unique<GUIgeometry>();
	BuildSubGUI();
	CreateMainFont();
}

void Editor::Draw(const GameTimer & gt)
{
	cmdList->SetPipelineState(GUIPSO);
	DrawEditorGUI();
	cmdList->SetPipelineState(textPSO);
	DrawTextGUI();

}

void Editor::Update(const GameTimer & gt)
{
	auto currentGUIVB = currentFrameResource->EditorGUIVB.get();
	assert(mainGUIgeometry != nullptr);
	for (auto& g : mainGUIgeometry->subGeos)
	{

		assert(g != nullptr);
		int visableGUIcount = 0;
		auto gui = g->myGUI;
		auto currGUIdataBuffer = currentFrameResource->GUIdataBuffers[gui->bufferIndex].get();

		GUIdata tempGUIdata;
		tempGUIdata.color = gui->color;
		currGUIdataBuffer->CopyData(visableGUIcount++, tempGUIdata);
	}
	
	


	for (UINT i = 0; i < (UINT)mAllGUIGeometryVertices.size(); i++)
	{
		Vertex vert = mAllGUIGeometryVertices[i];
		currentGUIVB->CopyData(i, vert);
	}

	mainGUIgeometry->VertexBufferGPU = currentGUIVB->Resource();

	auto currFontVB = currentFrameResource->fontVB.get();
	UINT k = 0;

	
	for (auto immerseText : mImmerseTextObjects)
	{

		for (UINT i = 0; i < (UINT)immerseText->myVerticesPos.size(); i++,k++)
		{

			Vertex vert;

			vert.Pos = immerseText->myVerticesPos[i];

			vert.TexC = immerseText->myVerticesTex[i];
			

			currFontVB->CopyData(immerseText->vertexBufferIndex + i, vert);

		}
	}
	
	mainFont->VertexBufferGPU = currFontVB->Resource();


}

void Editor::SetCurrentFrameResource(FrameResource* frameResource)
{
	currentFrameResource = frameResource;
}

void Editor::SetPSOs(ID3D12PipelineState* GUIPSO, ID3D12PipelineState* textPSO)
{
	this->GUIPSO = GUIPSO;
	this->textPSO = textPSO;
}

void Editor::CheckGUIInteraction(int x, int y)
{
	int chosenIndex = -1;
	UINT closestDepth = 0;
	for (UINT i = 0; i < (UINT)mainGUIgeometry->subGeos.size(); i++)
	{
		
		auto gui = mainGUIgeometry->subGeos[i];
		if (gui->myGUI->bIsVisible == false) { continue; }
		
		float guiX = gui->myGUI->x;
		float guiY = gui->myGUI->y;
		float guiWidth = gui->myGUI->width;
		float guiHeight = gui->myGUI->height;
		float mouseXndc = (2.0f * x) / (mScreenViewport->Width) - 1;
		float mouseYndc = -(2.0f * y) / (mScreenViewport->Height) + 1;
		/*
		std::string pi = "guiX: " + std::to_string(guiX);
		pi += " guiY: " + std::to_string(guiY);
		pi += " guiWidth: " + std::to_string(guiWidth);
		pi += " guiHeight: " + std::to_string(guiHeight);
		pi += " mouseXndc: " + std::to_string(mouseXndc);
		pi += " mouseYndc: " + std::to_string(mouseYndc);
		pi += "  ";
		std::wstring stemp = std::wstring(pi.begin(), pi.end());
		LPCWSTR sw = stemp.c_str();
		OutputDebugString(sw);
		OutputDebugStringW(L"\n");
		*/

		if (guiX < mouseXndc && guiY > mouseYndc && guiX + guiWidth > mouseXndc && guiY - guiHeight < mouseYndc)
		{
			if (gui->myGUI->depth < closestDepth) { continue; }
			else
			{
				chosenIndex = i;
				closestDepth = gui->myGUI->depth;
			}
			
		}

	}
	if (chosenIndex >= 0)
	{
		auto gui = mainGUIgeometry->subGeos[chosenIndex];
		BaseGUI* temp = gui->myGUI;
		PanelGUI* checkPanel = dynamic_cast<PanelGUI*>(temp);
		if (checkPanel != nullptr)
		{
			if (!focusedOnScrollBox)
			{
				checkPanel->ChangeSize(!checkPanel->bIsClosed);



				//update vertex buffer because the panel changed size
				UpdateVertexBuffer(gui);
			}


		}
		ScrollBoxGUI* checkScroll = dynamic_cast<ScrollBoxGUI*>(temp);
		if (checkScroll != nullptr)
		{
			//OutputDebugStringW(L"Scrolling turned off right now due to bug. \n");
			
			if (focusedOnScrollBox == true)
			{
				if (focusedScrollBoxGUI == checkScroll)
				{
					checkScroll->color.x -= .3f;
					checkScroll->color.y -= .3f;
					checkScroll->color.z -= .3f;
					focusedOnScrollBox = false;
					focusedScrollBoxGUI = nullptr;
					std::string pi = "Unfocused On Scrollbox";

					std::wstring stemp = std::wstring(pi.begin(), pi.end());
					LPCWSTR sw = stemp.c_str();
					OutputDebugString(sw);
					OutputDebugStringW(L"\n");
				}
			}
			else
			{
				checkScroll->color.x += .3f;
				checkScroll->color.y += .3f;
				checkScroll->color.z += .3f;
				std::string pi = "Focused On Scrollbox";

				std::wstring stemp = std::wstring(pi.begin(), pi.end());
				LPCWSTR sw = stemp.c_str();
				OutputDebugString(sw);
				OutputDebugStringW(L"\n");
				focusedScrollBoxGUI = checkScroll;
				focusedOnScrollBox = true;
			}
			
		}

		ButtonGUI* checkButton = dynamic_cast<ButtonGUI*>(temp);
		if (checkButton != nullptr)
		{
			if (!focusedOnScrollBox)
			{
				mainApp->OnEditorInteraction("SpawnObject");
				std::string pi = " Clicked A Button ";

				std::wstring stemp = std::wstring(pi.begin(), pi.end());
				LPCWSTR sw = stemp.c_str();
				OutputDebugString(sw);
				OutputDebugStringW(L"\n");
			}
		}


	}

}

void Editor::CheckGUIInteraction(bool scrollUp)
{
	if (focusedOnScrollBox == true && focusedScrollBoxGUI != nullptr)
	{
		if (scrollUp == true)
		{
			focusedScrollBoxGUI->ChangeScrollValue(1);
		}
		else
		{
			focusedScrollBoxGUI->ChangeScrollValue(-1);
		}
		UpdateScrollBox(focusedScrollBoxGUI);
	}
}

void Editor::BuildSubGUI()
{
	UINT totalVertexCount = 0;

	//Create right main panel

	rightMainPanel = std::make_unique<PanelGUI>(.8f, 1.f, .4f, 1.5f);
	rightMainPanel->color = DirectX::XMFLOAT4{ .5f,.5f,.5f,.80f };
	rightMainPanel->bufferIndex = 0;
	rightMainPanel->depth = 1;
	GUIsubGeometry* rightMainPanelSubGeo = new GUIsubGeometry();
	rightMainPanelSubGeo->BaseVertexLocation = GUIVertexBufferOpenIndex;
	rightMainPanelSubGeo->StartIndexLocation = GUIIndexBufferOpenIndex;
	rightMainPanelSubGeo->IndexCount = (UINT)rightMainPanel->meshData.Indices32.size();
	BaseGUI* temp = static_cast<BaseGUI*>(rightMainPanel.get());
	rightMainPanelSubGeo->myGUI = temp;
	GUIVertexBufferOpenIndex += (UINT)rightMainPanel->meshData.Vertices.size();
	GUIIndexBufferOpenIndex += (UINT)rightMainPanel->meshData.Indices32.size();
	totalVertexCount += (UINT)rightMainPanel->meshData.Vertices.size();
	

	//Create scrollbox;

	rMPScroll1 = std::make_unique<ScrollBoxGUI>(.81f, .8f, .06f, .08f);
	rMPScroll1->color = DirectX::XMFLOAT4{ 0.0f,0.0f,1.0f,1.0f };
	rMPScroll1->bufferIndex = 1;
	rMPScroll1->depth = 2;
	rightMainPanel->scrollBoxes.push_back(rMPScroll1.get());
	GUIsubGeometry* rMPScroll1SubGeo = new GUIsubGeometry();
	rMPScroll1SubGeo->BaseVertexLocation = GUIVertexBufferOpenIndex;
	rMPScroll1SubGeo->StartIndexLocation = GUIIndexBufferOpenIndex;
	rMPScroll1SubGeo->IndexCount = (UINT)rMPScroll1->meshData.Indices32.size();
	BaseGUI* temp1 = static_cast<BaseGUI*>(rMPScroll1.get());
	rMPScroll1SubGeo->myGUI = temp1;
	GUIVertexBufferOpenIndex += (UINT)rMPScroll1->meshData.Vertices.size();
	GUIIndexBufferOpenIndex += (UINT)rMPScroll1->meshData.Indices32.size();
	totalVertexCount += (UINT)rMPScroll1->meshData.Vertices.size();

	//Create button
	rMPButton1 = std::make_unique<ButtonGUI>(.81f, .5f, .12f, .05f);
	rMPButton1->color = DirectX::XMFLOAT4(0.0f, .5f, 0.0f, 1.0f);
	rMPButton1->bufferIndex = 2;
	rMPButton1->depth = 2;
	rightMainPanel->buttons.push_back(rMPButton1.get());
	GUIsubGeometry* rMPButton1SubGeo = new GUIsubGeometry();
	rMPButton1SubGeo->BaseVertexLocation = GUIVertexBufferOpenIndex;
	rMPButton1SubGeo->StartIndexLocation = GUIIndexBufferOpenIndex;
	rMPButton1SubGeo->IndexCount = (UINT)rMPButton1->meshData.Indices32.size();
	BaseGUI* temp2 = static_cast<BaseGUI*>(rMPButton1.get());
	rMPButton1SubGeo->myGUI = temp2;
	GUIVertexBufferOpenIndex += (UINT)rMPButton1->meshData.Vertices.size();
	GUIIndexBufferOpenIndex += (UINT)rMPButton1->meshData.Indices32.size();
	totalVertexCount += (UINT)rMPButton1->meshData.Vertices.size();


	mAllGUI.push_back(temp);
	mAllGUI.push_back(temp1);
	mAllGUI.push_back(temp2);

	for (auto n : mAllGUI)
	{
		for (auto v : n->meshData.Vertices)
		{
			Vertex tempVert;
			tempVert.Pos = v.Position;
			tempVert.Normal = v.Normal;
			tempVert.TangentU = v.TangentU;
			tempVert.TexC = v.TexC;
			mAllGUIGeometryVertices.push_back(tempVert);
		}
	}


	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(rightMainPanel->meshData.GetIndices16()), std::end(rightMainPanel->meshData.GetIndices16()));
	indices.insert(indices.end(), std::begin(rMPScroll1->meshData.GetIndices16()), std::end(rMPScroll1->meshData.GetIndices16()));
	indices.insert(indices.end(), std::begin(rMPButton1->meshData.GetIndices16()), std::end(rMPButton1->meshData.GetIndices16()));
	UINT vbByteSize = (UINT)mAllGUIGeometryVertices.size() * sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mainGUIgeometry->VertexBufferCPU = nullptr;
	mainGUIgeometry->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mainGUIgeometry->IndexBufferCPU));
	CopyMemory(mainGUIgeometry->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	mainGUIgeometry->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
		cmdList, indices.data(), ibByteSize, mainGUIgeometry->IndexBufferUploader);

	mainGUIgeometry->VertexByteStride = sizeof(Vertex);
	mainGUIgeometry->VertexBufferByteSize = vbByteSize;
	mainGUIgeometry->IndexFormat = DXGI_FORMAT_R16_UINT;;
	mainGUIgeometry->IndexBufferByteSize = ibByteSize;

	mainGUIgeometry->subGeos.push_back(std::move(rightMainPanelSubGeo));
	mainGUIgeometry->subGeos.push_back(std::move(rMPScroll1SubGeo));
	mainGUIgeometry->subGeos.push_back(std::move(rMPButton1SubGeo));
	mAllGUIsUnorderedMap["MainPanel"] = rightMainPanelSubGeo;
	mAllGUIsUnorderedMap["Scrollbox1"] = rMPScroll1SubGeo;
	mAllGUIsUnorderedMap["Button1"] = rMPButton1SubGeo;
	
	
	
}

void Editor::CreateMainFont()
{

	std::vector<std::uint16_t> indices;

	for (int i = 0; i < 800; i++)
	{
		indices.push_back(mainFont->currentStartingIndex + 0);
		indices.push_back(mainFont->currentStartingIndex + 1);
		indices.push_back(mainFont->currentStartingIndex + 2);
		indices.push_back(mainFont->currentStartingIndex + 1);
		indices.push_back(mainFont->currentStartingIndex + 3);
		indices.push_back(mainFont->currentStartingIndex + 2);
		mainFont->currentStartingIndex += 4;
	}
	UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
	UINT vbByteSize = (UINT)fontVertices.size() * sizeof(Vertex);
	mainFont->VertexBufferCPU = nullptr;


	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mainFont->IndexBufferCPU));
	CopyMemory(mainFont->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	mainFont->VertexBufferGPU = nullptr;


	mainFont->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
		cmdList, indices.data(), ibByteSize, mainFont->IndexBufferUploader);

	mainFont->VertexByteStride = sizeof(Vertex);
	mainFont->VertexBufferByteSize = vbByteSize;
	mainFont->IndexFormat = DXGI_FORMAT_R16_UINT;;
	mainFont->IndexBufferByteSize = ibByteSize;
	mainGUIgeometry->subGeos[0]->myGUI->myTitle = CreateText(std::string("test"), XMFLOAT2(0.f, 0.f), 7.0f, mainGUIgeometry->subGeos[0]->myGUI);
	mainGUIgeometry->subGeos[1]->myGUI->myTitle = CreateText(std::string("0"), XMFLOAT2(0.f, 0.f), 5.0f, mainGUIgeometry->subGeos[1]->myGUI);
	//mainGUIgeometry->subGeos[2]->myGUI->myTitle = CreateText(std::string("poo"), XMFLOAT2(0.f, 0.f), 5.0f, mainGUIgeometry->subGeos[2]->myGUI);
	BaseGUI* temp = mainGUIgeometry->subGeos[0]->myGUI;
	PanelGUI* checkPanel = dynamic_cast<PanelGUI*>(temp);
	checkPanel->ChangeSize(true);
}

void Editor::DrawEditorGUI()
{
	for (UINT i = 0; i < (UINT)mainGUIgeometry->subGeos.size(); i++)
	{

		auto gui = mainGUIgeometry->subGeos[i];
		if (gui->myGUI->bIsVisible == false) { continue; }
		
		auto GUIdataBuffer = currentFrameResource->GUIdataBuffers[gui->myGUI->bufferIndex]->Resource();
		cmdList->IASetVertexBuffers(0, 1, &mainGUIgeometry->VertexBufferView());
		cmdList->IASetIndexBuffer(&mainGUIgeometry->IndexBufferView());
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->SetGraphicsRootShaderResourceView(5, GUIdataBuffer->GetGPUVirtualAddress());


		cmdList->DrawIndexedInstanced(gui->IndexCount, 1, gui->StartIndexLocation, gui->BaseVertexLocation, 0);


	}
}

void Editor::DrawTextGUI()
{
	for (auto immerseText : mImmerseTextObjects)
	{
		if (immerseText->bIsVisible == false) { continue; }
		cmdList->IASetVertexBuffers(0, 1, &mainFont->VertexBufferView());
		cmdList->IASetIndexBuffer(&mainFont->IndexBufferView());
		cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmdList->DrawIndexedInstanced(immerseText->indexCount, 1, 0, (int)immerseText->vertexBufferIndex, 0);
	}
}

ImmerseText* Editor::CreateText(std::string & text, XMFLOAT2 position, float fontSize, BaseGUI* parentGUI, XMFLOAT4 color)
{
	float xWidth = fontSize / 142.857142857f;
	float yHeight = fontSize / 100.0f;

	ImmerseText* immerseText = new ImmerseText(text, textBufferLastVertexIndex, textBufferLastIndiceIndex, parentGUI);

	XMFLOAT2 textPosition = XMFLOAT2(parentGUI->x + position.x, parentGUI->y + position.y); //position is offset from gui position
	for (int i = 0; i < text.length(); i++)
	{
		char c = text.at(i);
		XMFLOAT4 test = mainFont->MapGlyphQuad(c);

		Vertex vert1;
		float u = test.x / (float)fontTexture->Resource->GetDesc().Width;
		float v = test.y / (float)fontTexture->Resource->GetDesc().Height;
		float h = test.w / (float)fontTexture->Resource->GetDesc().Height;
		float w = test.z / (float)fontTexture->Resource->GetDesc().Width;
		vert1.TexC = XMFLOAT2(u, v);

		vert1.Pos = XMFLOAT3(textPosition.x, textPosition.y, 0.0f);


		Vertex vert2;
		vert2.TexC = XMFLOAT2(u + w, v);
		vert2.Pos = XMFLOAT3(textPosition.x + xWidth, textPosition.y, 0.0f);
		Vertex vert3;
		vert3.TexC = XMFLOAT2(u, v + h);
		vert3.Pos = XMFLOAT3(textPosition.x, textPosition.y - yHeight, 0.0f);
		Vertex vert4;
		vert4.TexC = XMFLOAT2(u + w, v + h);
		vert4.Pos = XMFLOAT3(textPosition.x + xWidth, textPosition.y - yHeight, 0.0f);

		immerseText->PushBackVertsPos(vert1.Pos);
		immerseText->PushBackVertsTex(vert1.TexC);
		immerseText->PushBackVertsPos(vert2.Pos);
		immerseText->PushBackVertsTex(vert2.TexC);
		immerseText->PushBackVertsPos(vert3.Pos);
		immerseText->PushBackVertsTex(vert3.TexC);
		immerseText->PushBackVertsPos(vert4.Pos);
		immerseText->PushBackVertsTex(vert4.TexC);

		fontVertices.push_back(vert1);
		fontVertices.push_back(vert2);
		fontVertices.push_back(vert3);
		fontVertices.push_back(vert4);

		textPosition.x += xWidth;
		

	}


	
	textBufferLastVertexIndex += (UINT)immerseText->myVerticesPos.size();

	immerseText->indexCount = (UINT)text.length() * 6;
	textBufferLastIndiceIndex += (UINT)text.length() * 6;
	UINT vbByteSize = (UINT)fontVertices.size() * sizeof(Vertex);
	mainFont->VertexBufferByteSize = vbByteSize;
	mainFont->numCharCreated += (UINT)text.length();
	mainFont->textLength = mainFont->numCharCreated * 6;
	mImmerseTextObjects.push_back(std::move(immerseText));
	return immerseText;
	


}

void Editor::UpdateVertexBuffer(GUIsubGeometry* subGeo)
{
	UINT bufferLocation = subGeo->BaseVertexLocation;
	GeometryGenerator::MeshData meshData = subGeo->myGUI->meshData;
	for (UINT i = bufferLocation; i < bufferLocation + (UINT)meshData.Vertices.size(); i++)
	{
		Vertex meshVert;
		meshVert.Pos = meshData.Vertices[i - bufferLocation].Position;
		meshVert.Normal = meshData.Vertices[i - bufferLocation].Normal;
		meshVert.TangentU = meshData.Vertices[i - bufferLocation].TangentU;
		meshVert.TexC = meshData.Vertices[i - bufferLocation].TexC;
		mAllGUIGeometryVertices[i] = meshVert;
	}
}

void Editor::UpdateScrollBox(ScrollBoxGUI * scrollboxGUI)
{
	UINT vert = 0;
	for (int i = 0; i < scrollboxGUI->stringValue.length(); i++)
	{
		char c = scrollboxGUI->stringValue.at(i);
		XMFLOAT4 test = mainFont->MapGlyphQuad(c);

		
		float u = test.x / (float)fontTexture->Resource->GetDesc().Width;
		float v = test.y / (float)fontTexture->Resource->GetDesc().Height;
		float h = test.w / (float)fontTexture->Resource->GetDesc().Height;
		float w = test.z / (float)fontTexture->Resource->GetDesc().Width;

		scrollboxGUI->myTitle->myVerticesTex[vert] = XMFLOAT2(u, v);
		scrollboxGUI->myTitle->myVerticesTex[vert + 1] = XMFLOAT2(u + w, v);
		scrollboxGUI->myTitle->myVerticesTex[vert + 2] = XMFLOAT2(u, v + h);
		scrollboxGUI->myTitle->myVerticesTex[vert + 3] = XMFLOAT2(u + w, v + h);
		vert += 4;
		
	}
}
