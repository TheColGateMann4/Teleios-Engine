#include "FrameResourceDeleter.h"
#include "Graphics.h"

void FrameResourceDeleter::DeleteResource(Graphics& graphics, Microsoft::WRL::ComPtr<ID3D12Resource> pResource)
{
	m_resources.push_back(FrameResourceForDeletion{ pResource, graphics.GetCurrentBufferIndex() });
}

void FrameResourceDeleter::Update(Graphics& graphics)
{
	for (size_t resourceIndex = 0; resourceIndex < m_resources.size(); resourceIndex++)
	{
		FrameResourceForDeletion& resourceForDeletion = m_resources.at(resourceIndex);

		if(resourceForDeletion.frameIndex == graphics.GetCurrentBufferIndex())
		{
			if (resourceForDeletion.firstIteration)
				resourceForDeletion.firstIteration = false;
			else
			{
				// if same frame index is hit twice then resource is no longer needed by pipeline
				m_resources.erase(m_resources.begin() + resourceIndex);
				resourceIndex--; // we are substracting one from index since next resource is going to be at the same index since we deleted this one
			}
		}
	}
}