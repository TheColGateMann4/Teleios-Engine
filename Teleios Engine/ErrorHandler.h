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
		Exception(unsigned int line, const char* file, const char* function);

		virtual ~Exception() = default;

		Exception(const Exception&) = default;

	public:
		virtual std::string what() const;

		virtual const char* GetErrorType() const;

		unsigned int GetLine() const;
		const char* GetFile() const;
		const char* GetFunction() const;

	protected:
		unsigned int m_line;
		const char* m_file;
		const char* m_function;
	};

	class StandardException : public Exception
	{
	public:
		StandardException(unsigned int line, const char* file, const char* function, HRESULT hr);

	public:
		virtual std::string what() const override;

		virtual const char* GetErrorType() const override;
		std::string GetErrorString() const;

		HRESULT GetErrorCode() const;

	private:
		static std::string TranslateErrorCode(HRESULT hr);

	protected:
		HRESULT m_hr;
	};

	class InternalException : public Exception
	{
	public:
		InternalException(unsigned int line, const char* file, const char* function, const char* errorMessage);

	public:
		virtual std::string what() const override;

		virtual const char* GetErrorType() const override;
		std::string GetErrorString() const;

	private:
		std::string m_errorString;
	};

	class NoGFXException : public Exception
	{
	public:
		NoGFXException(unsigned int line, const char* file, const char* function);

		virtual const char* GetErrorType() const override;
	};

#ifdef _DEBUG
	class BlobMsgException : public Exception
	{
	public:
		BlobMsgException(unsigned int line, const char* file, const char* function, ::ID3DBlob* pErrorMessages);

	public:
		virtual std::string what() const override;
		virtual const char* GetErrorType() const override;

		size_t m_length;
		std::string m_errorMessages;
	};

	class InfoException : public Exception
	{
	public:
		InfoException(unsigned int line, const char* file, const char* function, std::vector<std::string> messages);

	public:
		virtual std::string what() const override;
		virtual const char* GetErrorType() const override;

		std::vector<std::string> m_messages;
	};
#endif

	class ObjectStateException : public InternalException
	{
	public:
		ObjectStateException(unsigned int line, const char* file, const char* function, const char* errorMessage);

	public:
		virtual std::string what() const override;

		virtual const char* GetErrorType() const override;
	};

	static void ThrowError(const char* title, const char* text) noexcept
	{
		MessageBoxA(NULL, text, title, MB_OK | MB_ICONEXCLAMATION);
	}
};