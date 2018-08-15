#include "ImmerseObject.h"

ImmerseObject::ImmerseObject(std::string newName, DirectX::XMMATRIX world, DirectX::XMMATRIX texTransform, Material* mat, MeshGeometry* geo, UINT indexCount, UINT startIndexLocation, int baseVertexLocation,UINT matIndex, UINT instanceCount)
{
	name = newName;
	XMStoreFloat4x4(&World, world);
	XMStoreFloat4x4(&TexTransform, texTransform);
	Mat = mat;
	Geo = geo;
	IndexCount = indexCount;
	StartIndexLocation = startIndexLocation;
	BaseVertexLocation = baseVertexLocation;
	InstanceCount = instanceCount;
	MatIndex = matIndex;
	bIs2D = false;
	instanceBufferIndex = 0;
	Instances.resize(1);
	XMStoreFloat4x4(&Instances[0].World, world);
	XMStoreFloat4x4(&Instances[0].TexTransform, texTransform);

}

