#pragma once
#include "Includes/CppIncludes.h"

class Graphics;

// purpose of this class is to hang onto the resource as long as frames-in-flight could potentially use the resource
class FrameResourceDeleter
{
	// structure for resource scheduled for deletion
	struct FrameResourceForDeletion
	{
		std::function<void()> destroy;
		unsigned int frameIndex;
		bool firstIteration = true;
	};

public:
	template<class T>
	void DeleteResource(Graphics& graphics, T&& resource)
	{
		static_assert(!std::is_trivial<T>());

		m_resources.push_back(FrameResourceForDeletion{
			[res = std::move(resource)]() mutable {}, // creating lambda that owns the resource
			GetFrameIndex(graphics)
		});
	};

	void Update(Graphics& graphics);

private:
	static unsigned int GetFrameIndex(Graphics& graphics);

private:
	std::vector<FrameResourceForDeletion> m_resources;
};