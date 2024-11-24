#include "FrameResourceDeleter.h"
#include "Graphics.h"

void FrameResourceDeleter::DeleteResource(Graphics& graphics, Microsoft::WRL::ComPtr<ID3D12Resource> pResource)
{
	m_resources.push_back(FrameResourceForDeletion{ pResource, graphics.GetCurrentBufferIndex() });
}

void FrameResourceDeleter::Update()
{
	for (size_t resourceIndex = 0; resourceIndex < m_resources.size(); resourceIndex++)
	{
		FrameResourceForDeletion& resourceForDeletion = m_resources.at(resourceIndex);

		if (resourceForDeletion.firstIteration)
			resourceForDeletion.firstIteration = false;
		else
		{
			// if same frame index is hit twice then resource is no longer needed by pipeline
			m_resources.erase(m_resources.begin() + resourceIndex);
		}
	}
}