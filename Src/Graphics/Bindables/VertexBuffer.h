#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Bindable.h"
#include "Graphics/Resources/GraphicsBuffer.h"
#include "Graphics/Data/DynamicVertex.h"

class Pipeline;
class Graphics;
class CommandList;

class VertexBuffer : public Bindable, public CommandListBindable
{
public:
	VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

	// data has to be aligned in 16 bytes
	VertexBuffer(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);
	VertexBuffer(Graphics& graphics, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);
	VertexBuffer(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	VertexBuffer(VertexBuffer&&) noexcept = default;
	VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

public:
	static std::shared_ptr<VertexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, void* pData, size_t numElements, size_t dataStride);

	static std::shared_ptr<VertexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

	static std::shared_ptr<VertexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, const DynamicVertex::DynamicVertexLayout& layout, size_t numElements);

	static std::shared_ptr<VertexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, void* pData, const DynamicVertex::DynamicVertexLayout& layout, size_t numElements);

public:
	void BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	void Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

	GraphicsBuffer* GetBuffer();

	const DynamicVertex::DynamicVertexLayout& GetLayout() const;

private:
	void UpdateBufferData(Graphics& graphics, void* pData);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	std::shared_ptr<GraphicsBuffer> m_buffer;
	std::shared_ptr<GraphicsBuffer> m_uploadBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	DynamicVertex::DynamicVertexLayout m_layout;
};

