#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "GraphicsResource.h"

#include "Graphics/Bindables/Bindable.h"

class Graphics;
class Pipeline;

class GraphicsBuffer : public GraphicsResource
{	
public:
	GraphicsBuffer(Graphics& graphics, unsigned int numElements, unsigned int byteStride, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

public:
	virtual void CopyResourcesToTexture(Graphics& graphics, CommandList* copyCommandList, GraphicsResource* dst, int targetMip = 0) override;

	virtual void CopyPartiallyTo(Graphics& graphics, CommandList* copyCommandList, size_t srcOffset, size_t srcSize, GraphicsResource* dst, size_t dstOffset);

	virtual GraphicsResourceType GetResourceType() override;

	void Read(Graphics& graphics, void* data, unsigned int size, unsigned int offset);

	void Update(Graphics& graphics, const void* data, size_t size, size_t offset = 0);
	void Update(Graphics& graphics, const void* data, size_t rowSize, size_t rows, size_t dataRowPitch, size_t targetRowPitch, size_t offset = 0);

	template<class T>
	void Update(Graphics& graphics, std::initializer_list<T> list)
	{
		Update(graphics, list.begin(), list.size() * sizeof(T));
	}

	void* Map(Graphics& graphics, SIZE_T readStart = 0, SIZE_T readEnd = 0);
	void UnMap(SIZE_T writeStart = 0, SIZE_T writeEnd = 0);

	size_t GetByteSize() const;
	size_t GetNumElements() const;
	size_t GetByteStride() const;

private:
	void UpdateUsingTempResource(Graphics& graphics, const void* data, size_t size, size_t offset = 0);
	void UpdateLocalResource(Graphics& graphics, const void* data, size_t rowSize, size_t rows, size_t dataRowPitch, size_t targetRowPitch, size_t offset = 0);

protected:
	size_t m_byteSize;
	size_t m_byteStride;
	size_t m_numElements;
	bool m_mapped = false;
};