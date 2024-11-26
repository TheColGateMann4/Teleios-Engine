#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindable.h"
#include <dxcapi.h>

static constexpr const wchar_t* GetDefaultEntryPointName(ShaderType type);

static std::wstring GetShaderVersion(ShaderType type);


class Graphics;
class PipelineState;

class Shader : public Bindable, public PipelineStateBindable
{
public:
	// should think of some cool way to ship the engine
	Shader(Graphics& graphics, const wchar_t* name, ShaderType type, std::vector<const wchar_t*> shaderMacros = {});

public:
	static std::shared_ptr<Shader> GetBindableResource(Graphics& graphics, const wchar_t* name, ShaderType type, std::vector<const wchar_t*> shaderMacros = {});

	static std::string GetIdentifier(const wchar_t* name, ShaderType type, std::vector<const wchar_t*> shaderMacros = {});

public:
	void Reload(Graphics& graphics);

	D3D12_SHADER_BYTECODE GetShaderByteCode() const;

	virtual void BindToPipelineState(Graphics& graphics, PipelineState* pipelineState) override;

	ShaderType GetType() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	ShaderType m_type;
	std::wstring m_name;
	std::wstring m_path;
	std::wstring m_entryPoint;

	std::vector<DxcDefine> m_shaderMacros;
};