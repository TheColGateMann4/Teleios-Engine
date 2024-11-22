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
public:
	virtual ~Bindable() = default;
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
	RootSignatureBindable(std::vector<TargetSlotAndShader> targets);

public:
	virtual ~RootSignatureBindable() = default;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) = 0;

	std::vector<TargetSlotAndShader>& GetTargets();

private:
	std::vector<TargetSlotAndShader> m_targets;
};

class PipelineStateBindable
{
public:
	virtual ~PipelineStateBindable() = default;

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) = 0;
};