#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class Fence
{
public:
	Fence(Graphics& graphics);

	~Fence();

public:
	void WaitForGPU(Graphics& graphics);

private:
	HANDLE m_fenceEvent = NULL;
	Microsoft::WRL::ComPtr<ID3D12Fence> pFence;
	size_t m_fenceValue;
};
