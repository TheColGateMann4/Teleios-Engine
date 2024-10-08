#include "Fence.h"
#include "Graphics.h"
#include "macros/ErrorMacros.h"

Fence::Fence(Graphics& graphics)
{
	HRESULT hr;

	THROW_ERROR(graphics.GetDevice()->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&pFence)
	));

	m_fenceValue = 0;


	m_fenceEvent = CreateEventA(nullptr, false, false, nullptr);

	if (m_fenceEvent == NULL)
		THROW_LAST_ERROR;
}

Fence::~Fence()
{
	CloseHandle(m_fenceEvent);
}

void Fence::WaitForGPU(Graphics& graphics)
{
	HRESULT hr;

	// if device got removed we don't want to process further since call on commandQueue will not be valid
	if (graphics.GetDevice()->GetDeviceRemovedReason() != S_OK) //  if GetDeviceRemovedReason() returns S_OK, then device was not removed
		return;

	// increasing fence value since we use it as index of event
	m_fenceValue++;

	THROW_ERROR(graphics.GetCommandQueue()->Signal(pFence.Get(), m_fenceValue));

	if(pFence->GetCompletedValue() < m_fenceValue)
	{
		THROW_ERROR(pFence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}