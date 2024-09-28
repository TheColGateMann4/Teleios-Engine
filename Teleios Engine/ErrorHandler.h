#pragma once
#include "includes/CppIncludes.h"
#include <exception>

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

	static void ThrowError(const char* title, const char* text) noexcept
	{
		MessageBoxA(NULL, text, title, MB_OK | MB_ICONEXCLAMATION);
	}
};