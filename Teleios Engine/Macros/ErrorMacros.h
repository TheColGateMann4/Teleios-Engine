#pragma once
#include "../ErrorHandler.h"

#define THROW_ERROR(statement) if((hr = statement) != S_OK) throw ErrorHandler::StandardException{ (UINT32)__LINE__, __FILE__, hr }; 

#define THROW_LAST_ERROR throw ErrorHandler::StandardException{ (UINT32)__LINE__, __FILE__, HRESULT_FROM_WIN32(GetLastError()) }; 
#define THROW_NOGFX		 throw ErrorHandler::NoGFXException{(UINT32)__LINE__, __FILE__};

#define THROW_INTERNAL_ERROR(errorString)				 				 throw ErrorHandler::InternalException{ (UINT32)__LINE__ , __FILE__ , errorString};
#define THROW_INTERNAL_ERROR_IF(errorString, statement)	   if(statement) throw ErrorHandler::InternalException{ (UINT32)__LINE__ , __FILE__ , errorString};