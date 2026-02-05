#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

class Graphics;

class QueryHeap
{
public:
	QueryHeap(Graphics& graphics, unsigned int numEntries);

public:
	unsigned int GetNumElements() const;

	ID3D12QueryHeap* Get() const;

private:
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_pResource;
	unsigned int m_numElements = 0;
};