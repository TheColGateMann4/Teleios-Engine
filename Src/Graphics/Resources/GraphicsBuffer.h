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
	void Update(Graphics& graphics, const void* data, size_t size);
	void Update(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size);

	template<class T>
	void Update(Graphics& graphics, std::initializer_list<T> list)
	{
		Update(graphics, list.begin(), list.size() * sizeof(T));
	}

	template<class T>
	void Update(Graphics& graphics, Pipeline& pipeline, std::initializer_list<T> list)
	{
		Update(graphics, pipeline, list.begin(), list.size() * sizeof(T));
	}

	size_t GetByteSize() const;
	size_t GetNumElements() const;
	size_t GetByteStride() const;

private:
	void UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size);
	void UpdateLocalResource(Graphics& graphics, const void* data, size_t size);

private:
	size_t m_byteSize;
	size_t m_byteStride;
	size_t m_numElements;
};