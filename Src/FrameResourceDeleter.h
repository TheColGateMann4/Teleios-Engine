#pragma once
#include "includes/CppIncludes.h"
#include <any>

class Graphics;

// purpose of this class is to hang onto the resource as long as frames-in-flight could potentially use the resource
class FrameResourceDeleter
{
	// structure for resource scheduled for deletion
	struct FrameResourceForDeletion
	{
		std::any pResource;
		unsigned int frameIndex;
		bool firstIteration = true;
	};

public:
	void DeleteResource(Graphics& graphics, std::any&& pResource);

	void Update(Graphics& graphics);

private:
	std::vector<FrameResourceForDeletion> m_resources;
};