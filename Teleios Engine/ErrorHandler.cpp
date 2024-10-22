#include "ErrorHandler.h"
#include "Includes/DirectXIncludes.h"

ErrorHandler::Exception::Exception(unsigned int line, const char* file, const char* function)
	:
	m_line(line),
	m_file(file),
	m_function(function)
{

}

std::string ErrorHandler::Exception::what()
{
	std::string result = {};

	result += GetErrorType();

	result += "\n\n[File] ";
	result += GetFile();

	result += "\n[Line] ";
	result += std::to_string(GetLine()).c_str();

	result += "\n[Function] ";
	result += GetFunction();

	return result;
}

const char* ErrorHandler::Exception::GetErrorType()
{
	return "EXCEPTION";
}

unsigned int ErrorHandler::Exception::GetLine()
{
	return m_line;
}

const char* ErrorHandler::Exception::GetFile()
{
	return m_file;
}

const char* ErrorHandler::Exception::GetFunction()
{
	return m_function;
}



/*
		STANDARD EXCEPTION
*/

ErrorHandler::StandardException::StandardException(unsigned int line, const char* file, const char* function, HRESULT hr)
	:
	Exception(line, file, function),
	m_hr(hr)
{

}

std::string ErrorHandler::StandardException::what()
{
	std::string result = {};

	result += GetErrorType();

	result += "\n[Error Code]: ";
	result += std::to_string(GetErrorCode());

	result += "\n[Error String]: ";
	result += GetErrorString();

	result += "\n";

	result += "\n[File] ";
	result += GetFile();

	result += "\n[Line] ";
	result += std::to_string(GetLine()).c_str();

	result += "\n[Function] ";
	result += GetFunction();

	return result;
}

const char* ErrorHandler::StandardException::GetErrorType()
{
	return "STANDARD_EXCEPTION";
}

std::string ErrorHandler::StandardException::GetErrorString()
{
	return TranslateErrorCode(m_hr);
}

HRESULT ErrorHandler::StandardException::GetErrorCode()
{
	return m_hr;
}

std::string ErrorHandler::StandardException::TranslateErrorCode(HRESULT hr)
{
	char* msgBuf = NULL;
	DWORD msgLen = FormatMessageA
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		hr,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		reinterpret_cast<LPSTR>(&msgBuf),
		NULL,
		NULL
	);

	if (msgLen == 0)
		return "Undefined error code";

	return msgBuf;
}

/*
		INTERNAL EXCEPTION
*/

ErrorHandler::InternalException::InternalException(unsigned int line, const char* file, const char* function, std::string errorString)
	: 
	Exception(line, file, function),
	m_errorString(errorString)
{

}

std::string ErrorHandler::InternalException::what()
{
	std::string result = {};

	result += GetErrorType();

	result += "\n[Error Name] ";
	result += GetErrorString();

	result += "\n\n[File] ";
	result += GetFile();

	result += "\n[Line] ";
	result += std::to_string(GetLine()).c_str();

	result += "\n[Function] ";
	result += GetFunction();

	return result;
}

const char* ErrorHandler::InternalException::GetErrorType()
{
	return "INTERNAL_EXCEPTION";
}

std::string ErrorHandler::InternalException::GetErrorString()
{
	return m_errorString;
}

/*
		NO GFX EXCEPTION 
*/

ErrorHandler::NoGFXException::NoGFXException(unsigned int line, const char* file, const char* function)
	: 
	Exception(line, file, function)
{

}

const char* ErrorHandler::NoGFXException::GetErrorType()
{
	return "NO_GFX_EXCEPTION";
}

/*
			BLOB MSG EXCEPTION
*/
#ifdef _DEBUG

ErrorHandler::BlobMsgException::BlobMsgException(unsigned int line, const char* file, const char* function, ::ID3DBlob* pErrorMessages)
	:
	Exception(line, file, function),
	m_length(pErrorMessages->GetBufferSize()),
	m_errorMessages(m_length, '\0')
{
	memcpy_s(m_errorMessages.data(), m_length, pErrorMessages->GetBufferPointer(), m_length);
}
 
std::string ErrorHandler::BlobMsgException::what()
{
	std::string result = {};

	result += GetErrorType();

	result += "\n\n[File] ";
	result += GetFile();

	result += "\n[Line] ";
	result += std::to_string(GetLine()).c_str();

	result += "\n[Function] ";
	result += GetFunction();

	result += "\n[Error Messages] ";
	result += m_errorMessages.c_str();

	return result;
}

const char* ErrorHandler::BlobMsgException::GetErrorType()
{
	return "INFO_EXCEPTION";
}

/*
			INFO EXCEPTION
*/

ErrorHandler::InfoException::InfoException(unsigned int line, const char* file, const char* function, std::vector<std::string> messages)
	:
	Exception(line, file, function),
	m_messages(messages)
{

}

std::string ErrorHandler::InfoException::what()
{
	std::string result = {};

	result += GetErrorType();

	result += "\n\n[File] ";
	result += GetFile();

	result += "\n[Line] ";
	result += std::to_string(GetLine()).c_str();

	result += "\n[Function] ";
	result += GetFunction();

	result += "\n[Messages]: \n\n";
	for (const auto& message : m_messages)
		result += message + "\n\n\n";

	return result;
}

const char* ErrorHandler::InfoException::GetErrorType()
{
	return "INFO_EXCEPTION";
}

#endif