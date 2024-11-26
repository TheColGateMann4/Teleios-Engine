#include "Shader.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"
#include "Graphics.h"

#include <d3dcompiler.h>

#include "BindableResourceList.h"

constexpr const wchar_t* GetDefaultEntryPointName(ShaderType type)
{
	switch (type)
	{
		case ShaderType::PixelShader:
			return L"PSMain";
		case ShaderType::VertexShader:
			return L"VSMain";
		case ShaderType::ComputeShader:
			return L"CSMain";
		case ShaderType::HullShader:
			return L"HSMain";
		case ShaderType::DomainShader:
			return L"DSMain";
		case ShaderType::GeometryShader:
			return L"GSMain";
	}
}

std::wstring GetShaderVersion(ShaderType type)
{
	std::wstring result;

	switch (type)
	{
		case ShaderType::PixelShader:
			result = L"ps"; break;
		case ShaderType::VertexShader:
			result = L"vs"; break;
		case ShaderType::ComputeShader:
			result = L"cs"; break;
		case ShaderType::HullShader:
			result = L"hs"; break;
		case ShaderType::DomainShader:
			result = L"ds"; break;
		case ShaderType::GeometryShader:
			result = L"gs"; break;
	}

	result += '_';
	result += USED_SHADER_MODEL;

	return result;
}

/*
			Shader Contructor
*/

Shader::Shader(Graphics& graphics, const wchar_t* name, ShaderType type, std::vector<const wchar_t*> shaderMacros)
	:
	m_type(type),
	m_name(std::wstring(name) + L".hlsl"),
	m_path(L"../../Shaders/" + m_name),
	m_entryPoint(GetDefaultEntryPointName(m_type))
{
	for (auto shaderMacro : shaderMacros)
		m_shaderMacros.push_back(DxcDefine{ shaderMacro, nullptr });

	Reload(graphics);
}

std::shared_ptr<Shader> Shader::GetBindableResource(Graphics& graphics, const wchar_t* name, ShaderType type, std::vector<const wchar_t*> shaderMacros)
{
	return BindableResourceList::GetBindableResource<Shader>(graphics, name, type, shaderMacros);
}

std::string Shader::GetIdentifier(const wchar_t* name, ShaderType type, std::vector<const wchar_t*> shaderMacros)
{
	std::string resultString = "Shader#";

	std::wstring wName = name;

	resultString += std::string(wName.begin(), wName.end());
	resultString += '#';

	resultString += std::to_string(size_t(type));
	resultString += '#';

	for (const auto shaderMacro : shaderMacros)
	{
		std::wstring wArg = name;

		resultString += std::string(wArg.begin(), wArg.end());
		resultString += '#';
	}

	return resultString;
}

void Shader::Reload(Graphics& graphics)
{
	HRESULT hr;

	// dxUtils for Loading file data and building args
	Microsoft::WRL::ComPtr<IDxcUtils> dxUtils;

	THROW_ERROR(DxcCreateInstance(
		CLSID_DxcUtils,
		IID_PPV_ARGS(&dxUtils))
	);

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> pEncodedFileBlob;

	THROW_ERROR(dxUtils->LoadFile(
		m_path.c_str(),
		nullptr,
		&pEncodedFileBlob)
	);

	DxcBuffer buffer{ pEncodedFileBlob->GetBufferPointer(), pEncodedFileBlob->GetBufferSize(), 0 };

#ifdef _DEBUG
	std::vector<const wchar_t*> pArgs = { DXC_ARG_DEBUG, DXC_ARG_SKIP_OPTIMIZATIONS, DXC_ARG_IEEE_STRICTNESS, DXC_ARG_ENABLE_STRICTNESS, DXC_ARG_WARNINGS_ARE_ERRORS, DXC_ARG_ALL_RESOURCES_BOUND, DXC_ARG_DEBUG_NAME_FOR_BINARY };
#else
	std::vector<const wchar_t*> pArgs = { DXC_ARG_OPTIMIZATION_LEVEL3 };
#endif

	Microsoft::WRL::ComPtr<IDxcCompilerArgs> pCompilerArguments;

	THROW_ERROR(dxUtils->BuildArguments(
		m_name.c_str(),
		m_entryPoint.c_str(),
		GetShaderVersion(m_type).c_str(),
		pArgs.data(),
		pArgs.size(),
		m_shaderMacros.data(),
		m_shaderMacros.size(),
		&pCompilerArguments)
	);

	Microsoft::WRL::ComPtr<IDxcCompiler3> pDxCompiler;

	THROW_ERROR(DxcCreateInstance(
		CLSID_DxcCompiler,
		IID_PPV_ARGS(&pDxCompiler))
	);

	Microsoft::WRL::ComPtr<IDxcResult> pCompilerResult;

	THROW_ERROR(pDxCompiler->Compile(
		&buffer,
		pCompilerArguments->GetArguments(),
		pCompilerArguments->GetCount(),
		nullptr,
		IID_PPV_ARGS(&pCompilerResult))
	);

	Microsoft::WRL::ComPtr<IDxcBlobUtf16> blobutf16;

	if(pCompilerResult->HasOutput(DXC_OUT_ERRORS) == TRUE)
	{
		THROW_SHADER_BYTECODE_BLOB_ERROR(pCompilerResult->GetOutput(
			DXC_OUT_ERRORS,
			IID_PPV_ARGS(&pShaderCode),
			&blobutf16)
		);
	}

	if (pCompilerResult->HasOutput(DXC_OUT_OBJECT) == TRUE)
	{
		THROW_ERROR(pCompilerResult->GetOutput(
			DXC_OUT_OBJECT,
			IID_PPV_ARGS(&pShaderCode),
			&blobutf16)
		);
	}
}


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