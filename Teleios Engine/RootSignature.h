#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class RootSignature
{
public:
	RootSignature(class Graphics& graphics);

public:
	ID3D12RootSignature* Get() const;

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
};

