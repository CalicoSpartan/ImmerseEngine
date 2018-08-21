#include "FrameResource.h"


FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT maxInstanceCount, UINT materialCount, UINT numRenderItems)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));
	mainDevice = device;
	mainPassCount = passCount;
	mainMaxInstanceCount = maxInstanceCount;
	mainMaterialCount = materialCount;
	mainNumRenderItems = numRenderItems;

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	SsaoCB = std::make_unique<UploadBuffer<SsaoConstants>>(device, 1, true);
	ImmerseObjectBuffer = std::make_unique<UploadBuffer<InstanceData>>(device, 4, false);
	EditorGUIBuffer = std::make_unique<UploadBuffer<InstanceData>>(device, 4, false);
	MaterialBuffer = std::make_unique<UploadBuffer<MaterialData>>(device, materialCount, false);
	for (UINT i = 0; i < numRenderItems; i++)
	{
		auto tempInstanceBuffer = std::make_unique<UploadBuffer<InstanceData>>(device, maxInstanceCount, false);
		renderItemBuffers.push_back(std::move(tempInstanceBuffer));
	}
	for (UINT i = 0; i < 2; i++)
	{
		auto tempGUIdataBuffer = std::make_unique<UploadBuffer<GUIdata>>(device, 1, false);
		GUIdataBuffers.push_back(std::move(tempGUIdataBuffer));
	}
	
	EditorGUIVB = std::make_unique<UploadBuffer<Vertex>>(device, 15, false);
}


FrameResource::~FrameResource()
{

}