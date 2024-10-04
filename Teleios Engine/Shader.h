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

static constexpr const char* GetDefaultEntryPointName(ShaderType type);

static std::string GetShaderVersion(ShaderType type);


#ifdef _DEBUG

class Shader
{
public:
	// debug shader that reads .hlsl files and compiles them
	Shader(const char* name, ShaderType type, const char* entryPointName = "default"); // leaving entryPointName as default will make class select proper entryPoint name for a shader

public:
	void Reload();

	ID3DBlob* GetShaderBlob() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	ShaderType m_type;
	std::string m_name;
	std::string m_entryPoint;
};

#else

class Shader
{
public:
	// release shader that reads already compiled shaders
	Shader(const char* name, ShaderType type, const char* entryPointName = nullptr);

public:
	void Reload();

	ID3DBlob* GetShaderBlob() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	ShaderType m_type;
	std::string m_name;
};

#endif