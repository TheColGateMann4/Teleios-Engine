#include "Shader.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/Graphics.h"

#include <d3dcompiler.h>

#include "Graphics/Core/BindableResourceList.h"

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
#ifdef _DEBUG
	m_path(L"../../Src/Shaders/" + m_name),
#else
	m_path(L"Shaders/" + m_name),
#endif
	m_entryPoint(GetDefaultEntryPointName(m_type)),
	m_uniqueName(GetIdentifier(name, type, shaderMacros))
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

	std::wstring_view wName = name;

	resultString += std::string(wName.begin(), wName.end());
	resultString += '#';

	resultString += std::to_string(size_t(type));
	resultString += '#';

	if(!shaderMacros.empty())
	{
		std::wstring allMacros = {};

		for (const auto shaderMacro : shaderMacros)
			allMacros += shaderMacro;

		resultString += std::to_string(std::hash<std::wstring>{}(allMacros));
		resultString += '#';
	}

	return resultString;
}

struct DxilShaderDebugName {
	uint16_t Flags;      // Reserved, must be set to zero.
	uint16_t NameLength; // Length of the debug name, without null terminator.
	// Followed by NameLength bytes of the UTF-8-encoded name.
	// Followed by a null terminator.
	// Followed by [0-3] zero bytes to align to a 4-byte boundary.
};

void Shader::Reload(Graphics& graphics)
{
	HRESULT hr;

	Microsoft::WRL::ComPtr<IDxcUtils> dxUtils; // dxUtils for Loading file data and building args
	Microsoft::WRL::ComPtr<IDxcCompiler3> pDXCompiler;

	// creating dxUtils instance
	THROW_ERROR(DxcCreateInstance(
		CLSID_DxcUtils,
		IID_PPV_ARGS(&dxUtils))
	);

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> pEncodedFileBlob;

	// loading main file
	THROW_ERROR(dxUtils->LoadFile(
		m_path.c_str(),
		nullptr,
		&pEncodedFileBlob)
	);


	// creating compiler instace
	THROW_ERROR(DxcCreateInstance(
		CLSID_DxcCompiler,
		IID_PPV_ARGS(&pDXCompiler))
	);

	Microsoft::WRL::ComPtr<IDxcResult> pCompilerResult;

#ifdef _DEBUG

	std::vector<const wchar_t*> pBinaryArgs = { DXC_ARG_DEBUG, DXC_ARG_SKIP_OPTIMIZATIONS, DXC_ARG_IEEE_STRICTNESS, DXC_ARG_ENABLE_STRICTNESS, DXC_ARG_WARNINGS_ARE_ERRORS, DXC_ARG_ALL_RESOURCES_BOUND, DXC_ARG_DEBUG_NAME_FOR_SOURCE };
	std::vector<const wchar_t*> pSourceArgs = { L"-P" };

	Microsoft::WRL::ComPtr<ID3DBlob> pSourceWithoutMacros;

	// getting main source without macros
	{
		DxcBuffer mainFileBuffer{ pEncodedFileBlob->GetBufferPointer(), pEncodedFileBlob->GetBufferSize(), 0 };

		pCompilerResult = CompileBlob(graphics, pDXCompiler.Get(), dxUtils.Get(), &mainFileBuffer, pSourceArgs);

		pSourceWithoutMacros = GetResult(graphics, pCompilerResult.Get(), DXC_OUT_HLSL);
	}

	DxcBuffer bufferWithoutMacros{ pSourceWithoutMacros->GetBufferPointer(), pSourceWithoutMacros->GetBufferSize(), 0 };

#else
	std::vector<const wchar_t*> pBinaryArgs = { DXC_ARG_OPTIMIZATION_LEVEL3 };

	// just using blob from read .hlsl file
	DxcBuffer bufferWithoutMacros{ pEncodedFileBlob->GetBufferPointer(), pEncodedFileBlob->GetBufferSize(), 0 };
#endif


	pCompilerResult = CompileBlob(graphics, pDXCompiler.Get(), dxUtils.Get(), &bufferWithoutMacros, pBinaryArgs);

	ThrowErrorMessagesResult(graphics, pCompilerResult.Get());

	pShaderCode = GetResult(graphics, pCompilerResult.Get(), DXC_OUT_OBJECT);

	// getting shader reflection data
	{
		GetReflection(dxUtils.Get(), GetResult(graphics, pCompilerResult.Get(), DXC_OUT_REFLECTION));
	}

#ifdef _DEBUG

	// saving .pdb files for graphic debugger
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pPDB = GetResult(graphics, pCompilerResult.Get(), DXC_OUT_PDB);

		DebugBlobToFile(".pdb", pPDB.Get());
	}

	// editing debug name
	pShaderCode = GetShaderWithEditedDebugName(graphics, pShaderCode.Get());

	// removing debug info from out shader binary
	pShaderCode = GetShaderWithoutDebugInfo(graphics, pShaderCode.Get());

#endif
}

D3D12_SHADER_BYTECODE Shader::GetShaderByteCode() const
{
	D3D12_SHADER_BYTECODE shaderBytecode = {};
	shaderBytecode.pShaderBytecode = pShaderCode->GetBufferPointer();
	shaderBytecode.BytecodeLength = pShaderCode->GetBufferSize();

	return shaderBytecode;
}

void Shader::BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState)
{
	pipelineState->SetShader(this);
}

void Shader::BindToComputePipelineState(Graphics& graphics, ComputePipelineState* pipelineState)
{
	pipelineState->SetShader(this);
}

BindableType Shader::GetBindableType() const
{
	return BindableType::bindable_shader;
}

ShaderType Shader::GetShaderType() const
{
	return m_type;
}

DirectX::XMUINT3 Shader::GetNumThreads() const
{
	THROW_INTERNAL_ERROR_IF("Tried to get number of threads of non compute shader", m_type != ShaderType::ComputeShader);
	
	DirectX::XMUINT3 result;

	pReflectionData->GetThreadGroupSize(&result.x, &result.y, &result.z);

	return result;
}

void Shader::GetReflection(IDxcUtils* dxUtils, Microsoft::WRL::ComPtr<ID3DBlob>&& pReflectionBlob)
{
	DxcBuffer reflectionIDxcBlob =
	{
		.Ptr = pReflectionBlob->GetBufferPointer(),
		.Size = pReflectionBlob->GetBufferSize(),
		.Encoding = 0
	};

	dxUtils->CreateReflection(&reflectionIDxcBlob, IID_PPV_ARGS(&pReflectionData));
}

Microsoft::WRL::ComPtr<IDxcResult> Shader::CompileBlob(Graphics& graphics, IDxcCompiler3* pDXCompiler, IDxcUtils* dxUtils, DxcBuffer* mainSourceBuffer, std::vector<const wchar_t*>& pArgs)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<IDxcCompilerArgs> pCompilerArgs;

	THROW_ERROR(dxUtils->BuildArguments(
		m_name.c_str(),
		m_entryPoint.c_str(),
		GetShaderVersion(m_type).c_str(),
		pArgs.data(),
		pArgs.size(),
		m_shaderMacros.data(),
		m_shaderMacros.size(),
		&pCompilerArgs)
	);


	Microsoft::WRL::ComPtr<IDxcResult> pCompilerResult;

	THROW_ERROR(pDXCompiler->Compile(
		mainSourceBuffer,
		pCompilerArgs->GetArguments(),
		pCompilerArgs->GetCount(),
		nullptr,
		IID_PPV_ARGS(&pCompilerResult))
	);

	return pCompilerResult;
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::GetResult(Graphics& graphics, IDxcResult* pResult, DXC_OUT_KIND resultKind)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob = {};

	if (pResult->HasOutput(resultKind) == TRUE)
	{
		THROW_ERROR(pResult->GetOutput(
			resultKind,
			IID_PPV_ARGS(&pBlob),
			nullptr)
		);
	}

	return pBlob;
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::GetShaderWithEditedDebugName(Graphics& graphics, ID3DBlob* pShaderCode)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> pEditedDebugName;

	std::string newDebugName = "Shaders/" + m_uniqueName + ".pdb";
	size_t lengthOfNameStorage = (newDebugName.size() + 0x3) & ~0x3;
	size_t nameBlobPartSize = sizeof(DxilShaderDebugName) + lengthOfNameStorage;

	newDebugName.resize(lengthOfNameStorage, '\0');

	THROW_ERROR(D3DSetBlobPart(
		pShaderCode->GetBufferPointer(),
		pShaderCode->GetBufferSize(),
		D3D_BLOB_DEBUG_NAME,
		0,
		newDebugName.c_str(),
		nameBlobPartSize,
		&pEditedDebugName
	));

	return pEditedDebugName;
}

Microsoft::WRL::ComPtr<ID3DBlob> Shader::GetShaderWithoutDebugInfo(Graphics& graphics, ID3DBlob* pShaderCode)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> pStrippedBlob;

	THROW_ERROR(D3DStripShader(
		pShaderCode->GetBufferPointer(),
		pShaderCode->GetBufferSize(),
		D3DCOMPILER_STRIP_DEBUG_INFO,
		&pStrippedBlob
	));

	return pStrippedBlob;
}

void Shader::ThrowErrorMessagesResult(Graphics& graphics, IDxcResult* pResult)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> pShaderCode;

	if (pResult->HasOutput(DXC_OUT_ERRORS) == TRUE)
	{
		THROW_SHADER_BYTECODE_BLOB_ERROR(pResult->GetOutput(
			DXC_OUT_ERRORS,
			IID_PPV_ARGS(&pShaderCode),
			nullptr)
		);
	}
}

void Shader::DebugBlobToFile(const char* extension, ID3DBlob* blob)
{
	const char* targetFolder = "Shaders/";

	if (!CreateDirectoryA(targetFolder, nullptr))
	{
		DWORD lastError = GetLastError();

		THROW_INTERNAL_ERROR_IF((std::string("Failed to create folder") + targetFolder).c_str(), lastError != ERROR_ALREADY_EXISTS);
	}


	std::string filePath = targetFolder + m_uniqueName + extension;

	HANDLE hFile = CreateFileA(
		filePath.c_str(),
		GENERIC_WRITE,
		NULL,
		nullptr,
		CREATE_ALWAYS,
		NULL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
		THROW_LAST_ERROR;

	if (WriteFile(
		hFile,
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		nullptr
	) != TRUE)
		THROW_LAST_ERROR;


	CloseHandle(hFile);
}