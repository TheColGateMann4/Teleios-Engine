#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class Fence
{
public:
	Fence(Fence&& other) noexcept;

	Fence(Graphics& graphics);

	~Fence();

public:
	void WaitForGPU(Graphics& graphics);


	void SetWaitValue(Graphics& graphics);

	void WaitForValue(Graphics& graphics);

private:
	Microsoft::WRL::ComPtr<ID3D12Fence> pFence;
	HANDLE m_fenceEvent = NULL;
	size_t m_fenceValue = 0;
	bool m_valueSet = false;
	bool m_moved = false;
};

