#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "Bindable.h"
#include "Buffer.h"

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
	void BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	void Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

	Buffer* GetBuffer();

private:
	void UpdateBufferData(Graphics& graphics, void* pData);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	std::shared_ptr<Buffer> m_buffer;
	std::shared_ptr<Buffer> m_uploadBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};

