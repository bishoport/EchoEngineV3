#pragma once

#include "../core/Core.h"
#include "../core/lights/Light.h"

namespace libCore
{
	class LightsManager
	{
	public:

		// Método para obtener la instancia de la clase
		static LightsManager& GetInstance() {
			static LightsManager instance;
			return instance;
		}

		static Ref<Light> CreateLight(bool randomColor, LightType lightType, glm::vec3 position)
		{
			auto light = CreateRef<Light>(lightType);

            // Position
			light->transform.position = position;

            float minColorValue = 0.5f;
            float maxColorValue = 1.0f;

			if (randomColor)
			{
				//Genera valores aleatorios para los componentes de color dentro del rango especificado
				light->color.r = static_cast<float>(minColorValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxColorValue - minColorValue))));
				light->color.g = static_cast<float>(minColorValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxColorValue - minColorValue))));
				light->color.b = static_cast<float>(minColorValue + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxColorValue - minColorValue))));
			}

            // Add the light to the global LightsManager
            libCore::LightsManager::AddLight(light);

			return light;
		}

		static void CreateDirectionalLight()
		{
			GetInstance().directionalLight = CreateRef<Light>(LightType::DIRECTIONAL);
			libCore::LightsManager::AddLight(GetInstance().directionalLight);
		}

		static const Ref<Light>& GetDirectionalLight()
		{
			return  GetInstance().directionalLight;
		}

		// Método estático para obtener la lista de luces
		static const std::vector<Ref<Light>>& GetLights() {
			return GetInstance().lights;
		}

		static void SetLightDataInShader(const std::string& shader)
		{
			std::vector<Ref<Light>> lights = GetInstance().GetLights();
			
			int numPointLights = 0;
			int numSpotLights = 0;
			int numAreaLights = 0;

			for (unsigned int i = 0; i < lights.size(); i++) 
			{
				if (lights[i]->type == LightType::POINT)
				{
					numPointLights++;
				}
				else if (lights[i]->type == LightType::SPOT)
				{
					numSpotLights++;
				}
				else if (lights[i]->type == LightType::AREA)
				{
					numAreaLights++;
				}
				lights[i]->SetLightDataToShader(shader);
			}

			libCore::ShaderManager::Get(shader)->setInt("numPointLights", numPointLights);
			libCore::ShaderManager::Get(shader)->setInt("numSpotLights", numSpotLights);
			libCore::ShaderManager::Get(shader)->setInt("numAreaLights", numAreaLights);
		}

		static void DrawDebugLights(const std::string& shader)
		{
			std::vector<Ref<Light>> lights = GetInstance().GetLights();

			for (unsigned int i = 0; i < lights.size(); i++) {
				lights[i]->DrawDebugLight(shader);
			}
		}


	private:
		LightsManager() {}// Constructor privado
		// Eliminar la posibilidad de copiar o asignar instancias
		LightsManager(const LightsManager&) = delete;
		LightsManager& operator=(const LightsManager&) = delete;

		// Método estático para agregar una luz
		static void AddLight(const Ref<Light>& light) {
			light->id = static_cast<int>(GetInstance().lights.size());
			GetInstance().lights.push_back(light);
		}

		std::vector<Ref<Light>> lights;
		Ref<Light> directionalLight;
	};

	// Declaración externa de una referencia a LightsManager
	extern LightsManager& lightsManager;
}