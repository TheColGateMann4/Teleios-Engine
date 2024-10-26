#pragma once
#include "TargetShaders.h"

class Graphics;
class CommandList;
class RootSignature;
class PipelineState;


struct TargetSlotAndShader
{
	ShaderVisibilityGraphic target;
	UINT slot;
	UINT rootIndex = 0;
};


class Bindable
{
	virtual void VTableRequirement() // for now we will keep it this way. Later on we are gonna do some bindable list type system
	{

	}
};


class CommandListBindable
{
public:
	virtual ~CommandListBindable() = default;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) = 0;
};

class DirectCommandListBindable
{
public:
	virtual ~DirectCommandListBindable() = default;

	virtual void BindToDirectCommandList(Graphics& graphics, CommandList* commandList) = 0;
};

class RootSignatureBindable
{
public:
	virtual ~RootSignatureBindable() = default;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) = 0;

	virtual std::vector<TargetSlotAndShader>& GetTargets() = 0;
};

class PipelineStateBindable
{
public:
	virtual ~PipelineStateBindable() = default;

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) = 0;
};