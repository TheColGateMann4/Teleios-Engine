#include "PrimitiveTechnology.h"
#include "CommandList.h"
#include "PipelineState.h"
#include "Macros/ErrorMacros.h"

PrimitiveTechnology::PrimitiveTechnology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
	:
	m_d3d12type(type),
	m_d3dtype(GetD3DPrimitiveFromD3D12(m_d3d12type))
{

}

void PrimitiveTechnology::BindToPipelineState(Graphics& graphics, PipelineState* pipelineState)
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