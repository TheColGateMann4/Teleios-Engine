#include "RootSignatureBindableWrapper.h"

#include "Graphics/Bindables/Binding.h"

#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/CommandList.h"

RootSignatureBindableWrapper::RootSignatureBindableWrapper(RootSignatureBinding* bind)
	:
	RootSignatureBindable(bind->GetTargets()),
	m_rootbind(bind)
{

}

void RootSignatureBindableWrapper::Initialize(Graphics& graphics)
{
	m_rootbind->InternalInitialize(graphics);
}

void RootSignatureBindableWrapper::BindToRootSignature(RootSignature* rootSignature)
{
	auto& targets = GetTargets();

	for (auto& target : targets)
		m_rootbind->BindToRootSignature(rootSignature, target);
}

BindableType RootSignatureBindableWrapper::GetBindableType() const
{
	return BindableType::bindable_rootSignatureWrapper;
}

RootParameterBindableWrapper::RootParameterBindableWrapper(RootParameterBinding* bind)
	:
	RootSignatureBindable(bind->GetTargets()),
	m_paramBind(bind)
{
	
}

void RootParameterBindableWrapper::Initialize(Graphics& graphics)
{
	m_paramBind->InternalInitialize(graphics);
}

void RootParameterBindableWrapper::BindToRootSignature(RootSignature* rootSignature)
{
	auto& targets = GetTargets();

	for (auto& target : targets)
		m_paramBind->BindToRootSignature(rootSignature, target);
}

void RootParameterBindableWrapper::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	auto& targets = GetTargets();

	for (auto& target : targets)
		m_paramBind->BindToCommandList(graphics, commandList, target);
}

BindableType RootParameterBindableWrapper::GetBindableType() const
{
	return BindableType::bindable_rootSignatureWrapper;
}

RootParameterBinding* RootParameterBindableWrapper::GetParameterBind() const
{
	return m_paramBind;
}