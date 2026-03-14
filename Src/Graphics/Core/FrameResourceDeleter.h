#pragma once
#include "Includes/CppIncludes.h"

class Graphics;

// purpose of this class is to hang onto the resource as long as frames-in-flight could potentially use the resource
class FrameResourceDeleter
{
	struct FrameIdentifier
	{
		FrameIdentifier(unsigned int _frameIndex, size_t _fenceValue);
		bool operator==(const FrameIdentifier& other) const;

		unsigned int frameIndex;
		size_t fenceValue;
	};

	using ResourceForDeletion = std::move_only_function<void()>;
	using ResourcesPerFrame = std::vector<ResourceForDeletion>;

	struct FrameIdentifierHash
	{
		size_t operator()(const FrameIdentifier& key) const
		{
			return std::hash<unsigned int>{}(key.frameIndex) ^ 
				std::hash<size_t>{}(key.fenceValue);
		}
	};

public:
	template<class T>
	void DeleteResource(Graphics& graphics, T&& resource)
	{
		static_assert(!std::is_trivial<T>());

		FrameIdentifier frameIdentifier = FrameIdentifier(GetFrameIndex(graphics), GetFenceValueForCurrentFrame(graphics));

		m_resources[frameIdentifier].push_back(ResourceForDeletion([res = std::move(resource)]() mutable {}));
	};

	void Update(Graphics& graphics);

private:
	static unsigned int GetFrameIndex(Graphics& graphics);
	static unsigned int GetFenceValueForCurrentFrame(Graphics& graphics);

private:
	std::unordered_map<FrameIdentifier, ResourcesPerFrame, FrameIdentifierHash> m_resources;
};