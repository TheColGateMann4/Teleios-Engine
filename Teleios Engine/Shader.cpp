#include "Shader.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"
#include <d3dcompiler.h>

#ifdef _DEBUG
constexpr const char* GetDefaultEntryPointName(ShaderType type)
{
	switch (type)
	{
		case ShaderType::PixelShader:
			return "PSMain";
		case ShaderType::VertexShader:
			return "VSMain";
		case ShaderType::ComputeShader:
			return "CSMain";
		case ShaderType::HullShader:
			return "HSMain";
		case ShaderType::DomainShader:
			return "DSMain";
		case ShaderType::GeometryShader:
			return "GSMain";
	}
}

std::string GetShaderVersion(ShaderType type)
{
	std::string result;

	switch (type)
	{
		case ShaderType::PixelShader:
			result = "ps"; break;
		case ShaderType::VertexShader:
			result = "vs"; break;
		case ShaderType::ComputeShader:
			result = "cs"; break;
		case ShaderType::HullShader:
			result = "hs"; break;
		case ShaderType::DomainShader:
			result = "ds"; break;
		case ShaderType::GeometryShader:
			result = "gs"; break;
	}

	result += '_';
	result += USED_SHADER_MODEL;

	return result;
}

/*
			Debug Shader Contructor
*/

// debug shader contructor reads .hlsl files and compiles them
Shader::Shader(const char* name, ShaderType type, std::vector<const char*> shaderMacros)
	:
	m_name(std::string("../../Shaders/") + name + ".hlsl"),
	m_type(type),
	m_entryPoint(GetDefaultEntryPointName(m_type))
{
	for (const auto shaderMacro : shaderMacros)
		m_shaderMacros.push_back(D3D_SHADER_MACRO{ shaderMacro, nullptr });

	m_shaderMacros.push_back(D3D_SHADER_MACRO{ NULL, NULL });

	Reload();
}

void Shader::Reload()
{
	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessages;
	std::wstring wName = std::wstring(m_name.begin(), m_name.end());
	std::string sShaderVersion = GetShaderVersion(m_type);
	UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;


	THROW_BLOB_ERROR(D3DCompileFromFile(
		wName.c_str(),						 // path to shader
		m_shaderMacros.data(),				 // macros that we will use for customizing one shader. This way we avoid making ton of shaders
		nullptr,							 // we are including files that are in relative directory
		m_entryPoint.c_str(),				 // entry point function name in said shader
		sShaderVersion.c_str(),				 // version of shader
		flags,
		0,
		&pShaderCode,						 // blob shader code
		&pErrorMessages						 // blob for error messages
	));
}

#else

/*
			Release Shader Contructor
*/

// release shader contructor reads already compiled shaders from .cso files
Shader::Shader(const char* name, ShaderType type, std::vector<const char*> shaderMacros)
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

#endif


D3D12_SHADER_BYTECODE Shader::GetShaderByteCode() const
{
	D3D12_SHADER_BYTECODE shaderBytecode = {};
	shaderBytecode.pShaderBytecode = pShaderCode->GetBufferPointer();
	shaderBytecode.BytecodeLength = pShaderCode->GetBufferSize();

	return shaderBytecode;
}

void Shader::BindToPipelineState(Graphics& graphics, PipelineState* pipelineState)
{
	pipelineState->SetShader(this);
}

ShaderType Shader::GetType() const
{
	return m_type;
}