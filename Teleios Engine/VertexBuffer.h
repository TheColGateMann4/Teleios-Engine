#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "Bindable.h"

class Pipeline;
class Graphics;
class CommandList;

namespace DynamicVertex
{
	class DynamicVertex;
};

class VertexBuffer : public Bindable, public CommandListBindable
{
public:
	// data has to be aligned in 16 bytes
	VertexBuffer(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

	VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

public:
	static std::shared_ptr<VertexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

	static std::shared_ptr<VertexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, void* pData, size_t numElements, size_t dataStride);

public:
	void BindToCopyPipelineIfNeeded(Pipeline& pipeline);
	
	void CopyResources(Graphics& graphics, CommandList* copyCommandList);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	void Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

private:
	void UpdateBufferData(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);
	
	void CreateResource(Graphics& graphics, size_t numElements, size_t dataStride, Microsoft::WRL::ComPtr<ID3D12Resource>& resultResource, bool isUploadResource = false);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadResource;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	size_t m_bufferSize = 0;
};

