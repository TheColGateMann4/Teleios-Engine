#pragma once
#include "includes/CppIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

struct ID3D12Resource;

// purpose of this class is to hang onto the resource as long as frames-in-flight could potentially use the resource
class FrameResourceDeleter
{
	// structure for resource scheduled for deletion
	struct FrameResourceForDeletion
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
		unsigned int frameIndex;
		bool firstIteration = true;
	};

public:
	void DeleteResource(Graphics& graphics, Microsoft::WRL::ComPtr<ID3D12Resource> pResource);

	void Update(Graphics& graphics);

private:
	std::vector<FrameResourceForDeletion> m_resources;
};

