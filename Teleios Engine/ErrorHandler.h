#pragma once
#include "includes/CppIncludes.h"
#include <exception>

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;


namespace ErrorHandler
{
	class Exception
	{
	public:
		Exception(unsigned int line, const char* file);

		virtual ~Exception() = default;

	public:
		virtual std::string what();

		virtual const char* GetErrorType();
		virtual std::string GetErrorString();

		unsigned int GetLine();
		const char* GetFile();

	protected:
		unsigned int m_line;
		const char* m_file;
	};

	class StandardException : public Exception
	{
	public:
		StandardException(unsigned int line, const char* file, HRESULT hr);

	public:
		virtual std::string what() override;

		virtual const char* GetErrorType() override;
		virtual std::string GetErrorString() override;

		HRESULT GetErrorCode();

	private:
		static std::string TranslateErrorCode(HRESULT hr);

	protected:
		HRESULT m_hr;
	};

	class InternalException : public Exception
	{
	public:
		InternalException(unsigned int line, const char* file, std::string errorString);

	public:
		virtual std::string what() override;

		virtual const char* GetErrorType() override;
		virtual std::string GetErrorString() override;

	private:
		std::string m_errorString;
	};

	class NoGFXException : public Exception
	{
	public:
		NoGFXException(unsigned int line, const char* file);

		virtual const char* GetErrorType() override;
	};

#ifdef _DEBUG
	class ShaderException : public Exception
	{
	public:
		ShaderException(unsigned int line, const char* file, ::ID3DBlob* pErrorMessages);

	public:
		virtual std::string what() override;
		virtual const char* GetErrorType() override;

		size_t m_length;
		std::string m_errorMessages;
	};

	class InfoException : public Exception
	{
	public:
		InfoException(unsigned int line, const char* file, std::vector<std::string> messages);

	public:
		virtual std::string what() override;
		virtual const char* GetErrorType() override;

		std::vector<std::string> m_messages;
	};
#endif

	static void ThrowError(const char* title, const char* text) noexcept
	{
		MessageBoxA(NULL, text, title, MB_OK | MB_ICONEXCLAMATION);
	}
};