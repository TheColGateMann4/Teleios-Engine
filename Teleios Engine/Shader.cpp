#include "Shader.h"
#include "Macros/ErrorMacros.h"

#ifdef _DEBUG
    #include <d3dcompiler.h>
#endif

constexpr const char* GetDefaultEntryPointName(ShaderType type)
{
	switch (type)
	{
		case PixelShader:
			return "PSMain";
		case VertexShader:
			return "VSMain";
		case ComputeShader:
			return "CSMain";
		case HullShader:
			return "HSMain";
		case DomainShader:
			return "DSMain";
		case GeometryShader:
			return "GSMain";
	}
}

std::string GetShaderVersion(ShaderType type)
{
	std::string result;

	switch (type)
	{
		case PixelShader:
			result = "ps"; break;
		case VertexShader:
			result = "vs"; break;
		case ComputeShader:
			result = "cs"; break;
		case HullShader:
			result = "hs"; break;
		case DomainShader:
			result = "ds"; break;
		case GeometryShader:
			result = "gs"; break;
	}

	result += '_';
	result += USED_SHADER_MODEL;

	return result;
}

#ifdef _DEBUG

/*
			Debug Shader
*/

Shader::Shader(const char* name, ShaderType type, const char* entryPointName)
	:
	m_name(std::string("../../Shaders/") + name + ".hlsl"),
	m_type(type),
	m_entryPoint(entryPointName == "default" ? GetDefaultEntryPointName(m_type) : entryPointName)
{
	Reload();
}

void Shader::Reload()
{
	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessages;
	std::wstring wName = std::wstring(m_name.begin(), m_name.end());
	std::string sShaderVersion = GetShaderVersion(m_type);
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;


	THROW_BLOB_ERROR(D3DCompileFromFile(
		wName.c_str(),						 // path to shader
		nullptr,							 // we don't use macros yet but I got bright plans with this
		nullptr,							 // we are including files that are in relative directory
		m_entryPoint.c_str(),				 // entry point function name in said shader
		sShaderVersion.c_str(),				 // version of shader
		flags,
		0,
		&pShaderCode,						 // blob shader code
		&pErrorMessages						 // blob for error messages
	));
}

ID3DBlob* Shader::GetShaderBlob() const
{
	return pShaderCode.Get();
}

#else

/*
			Release Shader
*/

Shader::Shader(const char* name, ShaderType type, const char*)
	:
	m_name(std::string("Shaders/") + name + ".cso"),
	m_type(type)
{
	Reload();
}

void Shader::Reload()
{
	HRESULT hr;

	std::wstring wName = std::wstring(m_name.begin(), m_name.end());

	THROW_BLOB_ERROR(D3DReadFileToBlob(
		wName.c_str(),
		&pShaderCode
	));
}

ID3DBlob* Shader::GetShaderBlob() const
{
	return pShaderCode.Get();
}

#endif