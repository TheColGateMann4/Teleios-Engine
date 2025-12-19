#include "PrimitiveTechnology.h"
#include "CommandList.h"
#include "PipelineState.h"
#include "Macros/ErrorMacros.h"

#include "BindableResourceList.h"

PrimitiveTechnology::PrimitiveTechnology(Graphics& graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
	:
	m_d3d12type(type),
	m_d3dtype(GetD3DPrimitiveFromD3D12(m_d3d12type))
{

}

std::shared_ptr<PrimitiveTechnology> PrimitiveTechnology::GetBindableResource(Graphics& graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
	return BindableResourceList::GetBindableResource<PrimitiveTechnology>(graphics, type);
}

std::string PrimitiveTechnology::GetIdentifier(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
	std::string resultString = "PrimitiveTechnology#";

	resultString += std::to_string(size_t(type));
	resultString += '#';

	return resultString;
}

void PrimitiveTechnology::BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState)
{
	pipelineState->SetPrimitiveTechnologyType(m_d3d12type);
}

void PrimitiveTechnology::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetPrimitiveTopology(graphics, m_d3dtype);
}

constexpr D3D_PRIMITIVE_TOPOLOGY PrimitiveTechnology::GetD3DPrimitiveFromD3D12(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
	switch (type)
	{
		case D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED:
			return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

		case D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

		case D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;

		case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		case D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH:
			THROW_INTERNAL_ERROR("Not supporting patch list yet");
	}
}