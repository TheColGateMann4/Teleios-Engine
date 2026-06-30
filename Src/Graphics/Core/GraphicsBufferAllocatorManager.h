#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Resources/GraphicsBufferSuballocator.h"

class Graphics;


class GraphicsBufferAllocatorManager
{
public:
	std::shared_ptr<GraphicsBufferSuballocator> RequestBufferAllocator(Graphics& graphics, unsigned int numElements, unsigned int stride);

	GraphicsBufferSuballocator* Get(size_t allocatorIndex);

	void Update(Graphics& graphics);

private:
	std::vector<std::shared_ptr<GraphicsBufferSuballocator>> m_allocators;
};