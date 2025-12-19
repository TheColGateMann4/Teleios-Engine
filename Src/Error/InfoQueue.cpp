#include "InfoQueue.h"
#include "Graphics/Core/Graphics.h"
#include "MessageMap.h"
#include "Macros/ErrorMacros.h"

/*
			InfoQueue
*/ 

#ifdef _DEBUG

InfoQueue::InfoQueue(Graphics& graphics)
{
	graphics.GetDevice()->QueryInterface(pInfoQueue.GetAddressOf());
}

size_t InfoQueue::GetNumMessages() const
{
	return static_cast<size_t>(pInfoQueue->GetNumStoredMessages());
}

std::vector<std::string> InfoQueue::GetMessages() const
{
	HRESULT hr;

	std::vector<std::string> result = {};
	size_t numMessages = GetNumMessages();

	for (size_t messageIndex = 0; messageIndex < numMessages; messageIndex++)
	{
		size_t messageLength = 0;

		//getting size of message
		pInfoQueue->GetMessage(messageIndex, NULL, &messageLength);

		D3D12_MESSAGE* pMessage = reinterpret_cast<D3D12_MESSAGE*>(new char[messageLength]);

		THROW_ERROR_NO_MSGS(pInfoQueue->GetMessage(messageIndex, pMessage, &messageLength));

		result.push_back(ProcessMessage(pMessage));
	}

	return result;
}

std::string InfoQueue::ProcessMessage(D3D12_MESSAGE* message) const
{
	std::string result;

	result.reserve(15 + 10 + 15 + 17 + 22 + 11 + 94);
	// 15 + 10 + 15 + 17 - title lengths
	// 22 - longest category
	// 11 - longest servity
	// 94 - longest ID

	//getting message Category and checking return value
	auto messageCategoryResult = messageCategoryNames.find(message->Category);
	if (messageCategoryResult == messageCategoryNames.end())
	{
		std::string resultStr = "Category Call ";
		resultStr += std::to_string(static_cast<int>(message->Category));
		resultStr += " wasn't defined in Category map";
		THROW_INTERNAL_ERROR(resultStr.c_str());
	}

	//getting message Severity and checking return value
	auto messageSeverityResult = messageSeverityNames.find(message->Severity);
	if (messageSeverityResult == messageSeverityNames.end()) {
		std::string resultStr = "Severity Call ";
		resultStr += std::to_string(static_cast<int>(message->Severity));
		resultStr += " wasn't defined in Severity map";
		THROW_INTERNAL_ERROR(resultStr.c_str());
	}

	//getting message ID and checking return value
	auto messageIDResult = messageIDNames.find(message->ID);
	if (messageIDResult == messageIDNames.end())
	{
		std::string resultStr = "ID Call ";
		resultStr += std::to_string(static_cast<int>(message->ID));
		resultStr += " wasn't defined in ID map";
		THROW_INTERNAL_ERROR(resultStr.c_str());
	}

	result += "\n[Category]: ";
	result += messageCategoryResult->second;

	result += "\n[Severity]: ";
	result += messageSeverityResult->second;

	result += "\n[MessageID]: ";
	result += messageIDResult->second;

	if (message->pDescription != nullptr)
	{
		result += "\n\n[Description]: ";

		size_t currSize = result.size(); 
		result.resize(currSize + message->DescriptionByteLength - 1); // -1 since directX included null terminator in length

		memcpy_s(result.data() + currSize, result.size(), message->pDescription, message->DescriptionByteLength - 1);
	}

	return result;
}

#endif