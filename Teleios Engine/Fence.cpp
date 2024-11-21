#include "Fence.h"
#include "Graphics.h"
#include "macros/ErrorMacros.h"

Fence::Fence(Fence&& other) noexcept
	:
	m_fenceEvent(other.m_fenceEvent),
	pFence(other.pFence),
	m_fenceValue(other.m_fenceValue),
	m_valueSet(other.m_valueSet)
{
	other.m_moved = true;
}

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
	if(!m_moved)
		CloseHandle(m_fenceEvent);
}

ID3D12Fence* Fence::Get() const
{
	return pFence.Get();
}

size_t Fence::GetValue() const
{
	return m_fenceValue;
}

void Fence::WaitForGPU(Graphics& graphics)
{
	SetWaitValue(graphics);

	WaitForValue(graphics);
}

void Fence::SetWaitValue(Graphics& graphics)
{
	HRESULT hr;

	// if device got removed we don't want to process further since call on commandQueue will not be valid
	if (graphics.GetDevice()->GetDeviceRemovedReason() != S_OK) //  if GetDeviceRemovedReason() returns S_OK, then device was not removed
		return;

	m_fenceValue++; // increasing value of fence

	THROW_ERROR(graphics.GetCommandQueue()->Signal(pFence.Get(), m_fenceValue));

	m_valueSet = true;
}

void Fence::WaitForValue(Graphics& graphics)
{
	if (!m_valueSet)
		return;

	HRESULT hr;

	if (graphics.GetDevice()->GetDeviceRemovedReason() != S_OK)
		return;

	if (pFence->GetCompletedValue() < m_fenceValue)
	{
		THROW_ERROR(pFence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}