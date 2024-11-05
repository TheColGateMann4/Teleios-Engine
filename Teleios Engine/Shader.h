#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindable.h"

#ifdef _DEBUG

static constexpr const char* GetDefaultEntryPointName(ShaderType type);

static std::string GetShaderVersion(ShaderType type);

#endif

class Graphics;
class PipelineState;

class Shader : public Bindable, public PipelineStateBindable
{
public:
	Shader(const char* name, ShaderType type, std::vector<const char*> shaderMacros = {});

public:
	static std::shared_ptr<Shader> GetBindableResource(const char* name, ShaderType type, std::vector<const char*> shaderMacros = {});

	static std::string GetIdentifier(const char* name, ShaderType type, std::vector<const char*> shaderMacros = {});

public:
	void Reload();

	D3D12_SHADER_BYTECODE GetShaderByteCode() const;

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

	ShaderType GetType() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	ShaderType m_type;
	std::string m_name;
	std::string m_entryPoint;
	std::vector<D3D_SHADER_MACRO> m_shaderMacros;
};