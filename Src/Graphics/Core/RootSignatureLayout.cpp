#include "RootSignatureLayout.h"

void RootSignatureLayout::AddParam(RootSignatureBindable* bindable, RootBinding binding)
{
	m_bindings.push_back(RootSignatureLayoutBinding(bindable, binding));
}

void RootSignatureLayout::BindToCommandList(Graphics& graphics, CommandList* commandList) const
{
	for (const auto& binding : m_bindings)
		binding.bindable->BindToCommandListAsRootParam(graphics, commandList, binding.binding);
}