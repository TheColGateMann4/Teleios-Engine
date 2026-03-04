#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Bindable.h"
#include "Graphics/Resources/GraphicsBuffer.h"
#include "Graphics/Data/DynamicVertex.h"

class Pipeline;
class Graphics;
class CommandList;

struct VertexBufferEntryInfo
{
	unsigned int offset;
	unsigned int size;
};

class VertexBuffer : public Bindable, public CommandListBindable
{
	struct VertexBufferUploadData
	{
		unsigned int offset;
		std::unique_ptr<GraphicsBuffer> uploadBuffer;
	};

public:
	VertexBuffer(const DynamicVertex::DynamicVertexLayout& layout);

	VertexBuffer(VertexBuffer&&) noexcept = default;
	VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

	static std::shared_ptr<VertexBuffer> GetResource(const DynamicVertex::DynamicVertexLayout& layout);

public:
	// data has to be aligned in 16 bytes
	VertexBufferEntryInfo PushData(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);
	VertexBufferEntryInfo PushData(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	void Build(Graphics& graphics);

public:
	void BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	GraphicsBuffer* GetBuffer();

	const DynamicVertex::DynamicVertexLayout& GetLayout() const;

private:
	void AddUploadBuffer(Graphics& graphics, unsigned int numElements, unsigned int dataStride, void* pData);
	void UpdateBufferData(Graphics& graphics, void* pData);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	std::shared_ptr<GraphicsBuffer> m_buffer;
	std::vector<VertexBufferUploadData> m_toUpload;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	DynamicVertex::DynamicVertexLayout m_layout;

	unsigned int m_accumulatedElements = {};
};

class VertexBufferEntry : public Bindable, public CommandListBindable
{
public:
	VertexBufferEntry(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	VertexBufferEntry(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

	static std::shared_ptr<VertexBufferEntry> GetResource(Graphics& graphics, const std::string& identifier, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	static std::shared_ptr<VertexBufferEntry> GetResource(Graphics& graphics, const std::string& identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	VertexBuffer* GetVertexBuffer() const;

	VertexBufferEntryInfo GetEntryInfo() const;

private:
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	VertexBufferEntryInfo m_entryInfo;
};