#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#ifdef _DEBUG
class InfoQueue
{
public:
	InfoQueue(class Graphics& graphics);

public:
	size_t GetNumMessages() const;

 	std::vector<std::string> GetMessages() const;

private:
	std::string ProcessMessage(struct D3D12_MESSAGE* message) const;

private:
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;

	static const std::map<D3D12_MESSAGE_CATEGORY, const char*> messageCategoryNames;
	static const std::map<D3D12_MESSAGE_SEVERITY, const char*> messageSeverityNames;
	static const std::map<D3D12_MESSAGE_ID, const char*> messageIDNames;
};
#endif

