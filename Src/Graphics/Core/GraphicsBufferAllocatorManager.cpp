#include "GraphicsBufferAllocatorManager.h"

std::shared_ptr<GraphicsBufferSuballocator> GraphicsBufferAllocatorManager::RequestBufferAllocator(Graphics& graphics, unsigned int numElements, unsigned int stride, D3D12_RESOURCE_STATES bufferState, BufferType type)
{
	SuballocatorIdentifier identifier = {};
	identifier.state = bufferState;
	identifier.type = type;

	auto [iterator, inserted] = m_allocators.try_emplace(identifier);

	if (inserted)
		iterator->second = std::make_shared<GraphicsBufferSuballocator>(graphics, numElements, stride, bufferState, type);

	return iterator->second;
}

void GraphicsBufferAllocatorManager::Update(Graphics& graphics)
{
	for (auto& [key, allocator] : m_allocators)
		allocator->Update(graphics);
}

bool GraphicsBufferAllocatorManager::SuballocatorIdentifier::operator==(const SuballocatorIdentifier& other) const
{
	return state == other.state &&
		type == other.type;
}