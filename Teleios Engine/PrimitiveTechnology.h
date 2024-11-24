#pragma once
#include "Bindable.h"
#include "includes/DirectXIncludes.h"

class Graphics;
class CommandList;
class RootSignature;

class PrimitiveTechnology : public Bindable, public PipelineStateBindable, public CommandListBindable
{
public:
	PrimitiveTechnology(Graphics& graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

public:
	static std::shared_ptr<PrimitiveTechnology> GetBindableResource(Graphics& graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

	static std::string GetIdentifier(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

public:
	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

private:
	static constexpr D3D_PRIMITIVE_TOPOLOGY GetD3DPrimitiveFromD3D12(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);

private:
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_d3d12type;
	D3D_PRIMITIVE_TOPOLOGY m_d3dtype;
};