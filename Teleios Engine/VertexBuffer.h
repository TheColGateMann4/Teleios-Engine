#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "Bindable.h"

class Graphics;
class CommandList;

class VertexBuffer : public Bindable, public CommandListBindable
{
public:
	// data has to be aligned in 16 bytes
	VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	void Update(Graphics& graphics, void* pData, size_t dataSizeInBytes);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};

