#pragma once
#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct InstanceData
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT MaterialIndex;
	UINT InstancePad0;
	UINT InstancePad1;
	UINT InstancePad2;
};

class ImmerseObject
{


public:
	ImmerseObject(std::string newName,DirectX::XMMATRIX world,DirectX::XMMATRIX texTransform,Material* mat,MeshGeometry* geo,UINT indexCount,UINT startIndexLocation,int baseVertexLocation,UINT matIndex, UINT instanceCount);
	std::string name;
	XMFLOAT4X4 World;
	bool bIs2D;
	XMFLOAT4X4 TexTransform;
	UINT instanceBufferIndex;
	BoundingBox Bounds;
	UINT MatIndex;
	std::vector<InstanceData> Instances;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT InstanceCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;


private:




};