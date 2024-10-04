#pragma once
#include "../ErrorHandler.h"

#define THROW_ERROR(statement) if((hr = statement) != S_OK) throw ErrorHandler::StandardException{ __LINE__, __FILE__, hr }; 

#define THROW_LAST_ERROR throw ErrorHandler::StandardException{ __LINE__, __FILE__, HRESULT_FROM_WIN32(GetLastError()) }; 
#define THROW_NOGFX		 throw ErrorHandler::NoGFXException{__LINE__, __FILE__};

#define THROW_INTERNAL_ERROR(errorString)				 				 throw ErrorHandler::InternalException{ __LINE__ , __FILE__ , errorString};
#define THROW_INTERNAL_ERROR_IF(errorString, statement)	   if(statement) throw ErrorHandler::InternalException{ __LINE__ , __FILE__ , errorString};

#ifdef _DEBUG
	#define THROW_SHADER_ERROR(statement)		     if((hr = statement) != S_OK) throw ErrorHandler::ShaderException{__LINE__ , __FILE__ , pErrorMessages.Get()};
	#define THROW_INFO_ERROR(statement)	  statement; if(graphics.GetInfoQueue()->GetNumMessages() != 0) throw ErrorHandler::InfoException{__LINE__ , __FILE__, graphics.GetInfoQueue()->GetMessages()};
#else
	#define THROW_SHADER_ERROR(statement)	THROW_ERROR
	#define THROW_INFO_ERROR(statement)
#endif