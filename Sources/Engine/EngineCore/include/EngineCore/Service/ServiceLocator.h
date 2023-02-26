#pragma once

#include <any>
#include <unordered_map>
#include <mutex>

#define service(type) EngineCore::Service::ServiceLocator::instance.get<type>()
#define provideService(type, data) EngineCore::Service::ServiceLocator::instance.provide<type>(data)

namespace EngineCore::Service
{
	class ServiceLocator
	{
		public:
			static ServiceLocator instance;
			std::unordered_map<size_t, std::any> services;
			std::recursive_mutex serviceMutex;

			template<typename T>
			void provide(T& pService)
			{
				std::unique_lock lock(serviceMutex);
				services[typeid(T).hash_code()] = std::any(&pService);
			}
		
		
			template<typename T>
			T& get()
			{
				std::unique_lock lock(serviceMutex);

				size_t hash = typeid(T).hash_code();
				
				if (services[hash].has_value())
					return *std::any_cast<T*>(services[hash]);

				T* service = new T();
				services[hash] = service;
				return *service;
			}

			template<typename T>
			void remove()
			{
				T* service = nullptr;
				size_t hash = typeid(T).hash_code();

				if (!services[hash].has_value())
					return;
					
				service = std::any_cast<T*>(services[hash]);
				services.erase(hash);
				delete service;
			}

			~ServiceLocator();
	};
}