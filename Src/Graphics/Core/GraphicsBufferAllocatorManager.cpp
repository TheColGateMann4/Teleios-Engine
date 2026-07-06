#include "GraphicsBufferAllocatorManager.h"

std::shared_ptr<GraphicsBufferSuballocator> GraphicsBufferAllocatorManager::RequestBufferAllocator(Graphics& graphics, unsigned int numElements, unsigned int stride)
{
	for (auto& allocator : m_allocators)
		if (allocator->GetByteStride() == stride)
			return allocator;

	m_allocators.push_back(std::make_shared<GraphicsBufferSuballocator>(graphics, numElements, stride));

	return m_allocators.back();
}

GraphicsBufferSuballocator* GraphicsBufferAllocatorManager::Get(size_t allocatorIndex)
{
	return m_allocators.at(allocatorIndex).get();
}

void GraphicsBufferAllocatorManager::Update(Graphics& graphics)
{
	for (auto& allocator : m_allocators)
		allocator->Update(graphics);
}