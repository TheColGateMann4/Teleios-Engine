#pragma once
#include "../ErrorHandler.h"

#define THROW_ERROR_NO_MSGS(statement) if((hr = statement) != S_OK)	throw ErrorHandler::StandardException{ __LINE__, __FILE__, __FUNCTION__, hr }; 

#ifdef _DEBUG
	#define DBG_THROW_ERROR(statement, graphics) \
		{\
			hr = statement;\
			if(graphics.GetInfoQueue()->GetNumMessages() != 0) \
				throw ErrorHandler::InfoException{__LINE__ , __FILE__, __FUNCTION__, graphics.GetInfoQueue()->GetMessages()}; \
			if(hr != S_OK) \
				throw ErrorHandler::StandardException{ __LINE__, __FILE__, __FUNCTION__, hr };\
		}

	#define THROW_ERROR(statement) DBG_THROW_ERROR(statement, graphics) // graphics is used name for graphics varible for now

	#define THROW_ERROR_AT_GFX_INIT(statement) DBG_THROW_ERROR(statement, (*this)); // we using *this since we will be inside graphics class initializing interfaces
#else
	#define THROW_ERROR(statement) THROW_ERROR_NO_MSGS(statement)
	#define THROW_ERROR_AT_GFX_INIT(statement) THROW_ERROR_NO_MSGS(statement)
#endif

#define THROW_LAST_ERROR throw ErrorHandler::StandardException{ __LINE__, __FILE__, __FUNCTION__ , HRESULT_FROM_WIN32(GetLastError()) }; 
#define THROW_NOGFX		 throw ErrorHandler::NoGFXException{__LINE__, __FILE__, __FUNCTION__};

#define THROW_INTERNAL_ERROR(errorString)				 				  throw ErrorHandler::InternalException{ __LINE__, __FILE__, __FUNCTION__, errorString};
#define THROW_INTERNAL_ERROR_IF(errorString, statement)		if(statement) throw ErrorHandler::InternalException{ __LINE__, __FILE__, __FUNCTION__, errorString};
#define THROW_OBJECT_STATE_ERROR_IF(errorString, statement) if(statement) throw ErrorHandler::ObjectStateException{ __LINE__, __FILE__, __FUNCTION__, errorString};

#ifdef _DEBUG
	#define THROW_BLOB_ERROR(statement)\
	if ((hr = statement) != S_OK)\
	{\
		if (pErrorMessages.Get() != nullptr)\
			throw ErrorHandler::BlobMsgException{ __LINE__, __FILE__, __FUNCTION__, pErrorMessages.Get() }; \
		else\
			throw ErrorHandler::StandardException{ __LINE__, __FILE__, __FUNCTION__, hr };\
	}

	#define THROW_INFO_ERROR(statement)	  statement; if(graphics.GetInfoQueue()->GetNumMessages() != 0) throw ErrorHandler::InfoException{__LINE__ , __FILE__, __FUNCTION__, graphics.GetInfoQueue()->GetMessages()};
#else
	#define THROW_BLOB_ERROR(statement)	THROW_ERROR(statement)
	#define THROW_INFO_ERROR(statement)
#endif