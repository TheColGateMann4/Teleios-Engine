#pragma once
#include "includes/CppIncludes.h"
#include "TargetShaders.h"

struct TargetSlotAndShader
{
	ShaderVisibilityGraphic target;
	UINT slot;
	UINT rootIndex = 0;
};

enum class RootSignatureResourceType
{
	type_constBufferView,
	type_texture,
	// srv
	// uav
	// constants
	// static sampler
};

class RootSignatureResource
{
protected:
	RootSignatureResource() = default;

public:
	virtual ~RootSignatureResource() = default;

public:
	virtual std::vector<TargetSlotAndShader>& GetTargets() = 0;
};

struct RootSignatureTypedResource
{
	RootSignatureResourceType type;
	RootSignatureResource* resource;
};