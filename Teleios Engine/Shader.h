#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

enum ShaderType
{
	PixelShader,
	VertexShader,
	ComputeShader,
	HullShader,
	DomainShader,
	GeometryShader
};

#ifdef _DEBUG

static constexpr const char* GetDefaultEntryPointName(ShaderType type);

static std::string GetShaderVersion(ShaderType type);

#endif

class Shader
{
public:
	// leaving entryPointName as default will make class select proper entryPoint name for a shader
	Shader(const char* name, ShaderType type, const char* entryPointName = nullptr);

public:
	void Reload();

	ID3DBlob* GetShaderBlob() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	ShaderType m_type;
	std::string m_name;
	std::string m_entryPoint;
};