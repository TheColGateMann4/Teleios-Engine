#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindable.h"
#include <d3d12shader.h>
#include <dxcapi.h>

static constexpr const wchar_t* GetDefaultEntryPointName(ShaderType type);

static std::wstring GetShaderVersion(ShaderType type);


class Graphics;
class GraphicsPipelineState;

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

	virtual void BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState) override;

	virtual void BindToComputePipelineState(Graphics& graphics, ComputePipelineState* pipelineState) override;

	ShaderType GetType() const;

private:
	void GetReflection(IDxcUtils* dxUtils, Microsoft::WRL::ComPtr<ID3DBlob>&& pReflectionBlob);

	Microsoft::WRL::ComPtr<IDxcResult> CompileBlob(Graphics& graphics, IDxcCompiler3* pDXCompiler, IDxcUtils* dxUtils, DxcBuffer* mainSourceBuffer, std::vector<const wchar_t*>& pArgs);

	Microsoft::WRL::ComPtr<ID3DBlob> GetResult(Graphics& graphics, IDxcResult* pResult, DXC_OUT_KIND resultKind);

	Microsoft::WRL::ComPtr<ID3DBlob> GetShaderWithEditedDebugName(Graphics& graphics, ID3DBlob* pShaderCode);

	Microsoft::WRL::ComPtr<ID3DBlob> GetShaderWithoutDebugInfo(Graphics& graphics, ID3DBlob* pShaderCode);

	void ThrowErrorMessagesResult(Graphics& graphics, IDxcResult* pResult);

	void DebugBlobToFile(const char* extension, ID3DBlob* blob);

private:
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflectionData;
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	ShaderType m_type;
	std::wstring m_name;
	std::wstring m_path;
	std::wstring m_entryPoint;

	std::vector<DxcDefine> m_shaderMacros;

	std::string m_uniqueName;
};