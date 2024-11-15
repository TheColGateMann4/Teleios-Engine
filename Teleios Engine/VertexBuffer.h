#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "Bindable.h"

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
	static std::shared_ptr<VertexBuffer> GetBindableResource(std::string identifier, Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

	static std::shared_ptr<VertexBuffer> GetBindableResource(std::string identifier, Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	void Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

private:
	void UpdateBufferData(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);
	
	void CreateResource(Graphics& graphics, size_t dataSize, size_t dataStride);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	size_t m_bufferSize = 0;
};

