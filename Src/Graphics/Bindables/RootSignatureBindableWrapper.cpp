#include "RootSignatureBindableWrapper.h"

#include "Graphics/Bindables/Binding.h"

#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/CommandList.h"

#include "Macros/ErrorMacros.h"

RootSignatureBindableWrapper::RootSignatureBindableWrapper(RootSignatureBinding* bind)
	:
	RootSignatureBindable(bind->GetTargets()),
	m_rootbind(bind)
{

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

RootSignatureBindableType RootSignatureBindableWrapper::GetRootSignatureBindableType() const
{
	THROW_INTERNAL_ERROR_IF("Resource type was not set", m_rootbind == nullptr);

	return m_rootbind->GetRootSignatureBindableType();
}

RootParameterBindableWrapper::RootParameterBindableWrapper(RootParameterBinding* bind)
	:
	RootSignatureBindable(bind->GetTargets()),
	m_paramBind(bind)
{
	
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

RootSignatureBindableType RootParameterBindableWrapper::GetRootSignatureBindableType() const
{
	THROW_INTERNAL_ERROR_IF("Resource type was not set", m_paramBind == nullptr);

	return m_paramBind->GetRootSignatureBindableType();
}

RootParameterBinding* RootParameterBindableWrapper::GetParameterBind() const
{
	return m_paramBind;
}