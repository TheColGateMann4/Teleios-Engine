#pragma once
#include "Includes/CppIncludes.h"

class ResourceList
{
public:
	template<class T, class ...Params>
	static std::shared_ptr<T> GetResource(Graphics& graphics, Params&& ...creationParams)
	{
		std::string identifier = T::GetIdentifier(creationParams...);
		return GetResourceByID<T>(identifier, graphics, std::forward<Params>(creationParams)...);
	}

	template<class T, class ...Params>
	static std::shared_ptr<T> GetResource(Params&& ...creationParams)
	{
		std::string identifier = T::GetIdentifier(creationParams...);
		return GetResourceByID<T>(identifier, std::forward<Params>(creationParams)...);
	}

	template<class T, class ...Params>
	static std::shared_ptr<T> GetResourceByID(const std::string& identifier, Params&& ...creationParams)
	{
		auto& map = GetMap();

		auto key = ResourceKey{typeid(T), identifier};

		auto [iterator, inserted] = map.try_emplace(key);

		if (inserted)
			iterator->second = std::make_shared<T>(std::forward<Params>(creationParams)...);

		return std::static_pointer_cast<T>(iterator->second);
	}

private:
	using ResourceKey = std::pair<std::type_index, std::string>;

	struct ResourceKeyHash
	{
		size_t operator()(const ResourceKey& key) const
		{
			return std::hash<std::type_index>{}(key.first) ^
				std::hash<std::string>{}(key.second);
		}
	};

	static auto& GetMap()
	{
		static std::unordered_map<ResourceKey, std::shared_ptr<void>, ResourceKeyHash> resources;
		return resources;
	}
};