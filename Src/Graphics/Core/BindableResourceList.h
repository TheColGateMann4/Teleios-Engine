#pragma once
#include "Includes/CppIncludes.h"
#include "Bindable.h"

class Graphics;

class BindableResourceList
{
public:
	template<class T, class ...Params>
	static std::shared_ptr<T> GetBindableResource(Graphics& graphics, Params&& ...creationParams)
	{
		std::string identifier = T::GetIdentifier(creationParams...);
		return GetBindableResourceByID<T>(graphics, identifier, std::forward<Params&&>(creationParams)...);
	}

	template<class T, class ...Params>
	static std::shared_ptr<T> GetBindableResourceByID(Graphics& graphics, std::string identifier, Params&& ...creationParams) // making identifier argument first so we don't mistakely take other argument as identifier when we were meant to call GetBindableResource
	{
		auto& bindableResourceMap = GetStaticMap();

		auto foundItem = bindableResourceMap.find(identifier);

		if (foundItem == bindableResourceMap.end()) // item not found?
		{
			std::shared_ptr<T> bindable = std::make_shared<T>(graphics, std::forward<Params&&>(creationParams)...);

			bindableResourceMap[identifier] = bindable; // pushing newly created bindable to our map

			return bindable; // returning newly created bindable
		}

		return std::static_pointer_cast<T>(foundItem->second); // casting shader pointer of type bindable to shader ptr of desired type
	}

private:
	static std::map<std::string, std::shared_ptr<Bindable>>& GetStaticMap()
	{
		static std::map<std::string, std::shared_ptr<Bindable>> m_bindableResources;
		return m_bindableResources;
	}
};