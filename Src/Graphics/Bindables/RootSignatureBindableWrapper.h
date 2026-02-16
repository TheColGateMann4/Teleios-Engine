#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Bindables/Bindable.h"

class Graphics;

class RootSignature;
class CommandList;

class RootParameterBinding;
class RootSignatureBinding;

class RootSignatureBindableWrapper : public Bindable, public RootSignatureBindable
{
public:
	RootSignatureBindableWrapper(RootSignatureBinding* bind);

	virtual void BindToRootSignature(RootSignature* rootSignature) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

private:
	RootSignatureBinding* m_rootbind;
};

class RootParameterBindableWrapper : public Bindable, public RootSignatureBindable, public CommandListBindable
{
public:
	RootParameterBindableWrapper(RootParameterBinding* bind);

public:
	virtual void BindToRootSignature(RootSignature* rootSignature) override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

	RootParameterBinding* GetParameterBind() const;

private:
	RootParameterBinding* m_paramBind;
};