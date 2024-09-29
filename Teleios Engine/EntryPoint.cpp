#include "Includes/CppIncludes.h"
#include "Application.h"
#include "ErrorHandler.h"

int WINAPI WinMain
(
	_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPSTR,
	_In_ int
)
{
	CONSOLE_HANDLE;
	INITIALIZE_CONSOLE;

	try
	{
		unsigned int screenWidth = unsigned int(std::round(float(GetSystemMetrics(SM_CXSCREEN)) * 0.625f));
		unsigned int screenHeight = unsigned int(std::round(float(GetSystemMetrics(SM_CYSCREEN)) * 0.83333333333f));

		Application app(screenWidth, screenHeight, "Teleios Engine");

		return app.Run();
	}
	catch (ErrorHandler::Exception& except)
	{
		ErrorHandler::ThrowError("Internal Error", except.what().c_str());
	}
	catch (std::exception& except)
	{
		ErrorHandler::ThrowError("Standard Error", except.what());
	}
	catch (...)
	{
		ErrorHandler::ThrowError("Unknown Error", "No Details Available");
	}

	CLEANUP_CONSOLE;

	return EXIT_FAILURE;
}