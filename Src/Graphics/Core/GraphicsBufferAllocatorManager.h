#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Resources/GraphicsBufferSuballocator.h"

class Graphics;

class GraphicsBufferAllocatorManager
{
public:
	std::shared_ptr<GraphicsBufferSuballocator> RequestBufferAllocator(Graphics& graphics, unsigned int numElements, unsigned int stride, D3D12_RESOURCE_STATES bufferState, BufferType type);

	void Update(Graphics& graphics);

private:
	struct SuballocatorIdentifier
	{
		D3D12_RESOURCE_STATES state;
		BufferType type;

		bool operator==(const SuballocatorIdentifier& other) const;
	};

	using ResourceKey = SuballocatorIdentifier;

	struct ResourceKeyHash
	{
		size_t operator()(const ResourceKey& key) const
		{
			return std::hash<D3D12_RESOURCE_STATES>{}(key.state) ^
				std::hash<BufferType>{}(key.type);
		}
	};

	std::unordered_map<ResourceKey, std::shared_ptr<GraphicsBufferSuballocator>, ResourceKeyHash> m_allocators;
};