#pragma once

#include "../core/Core.h"

namespace libCore
{
	struct Tween {
		using Callback = std::function<void(const glm::vec3&)>;

		glm::vec3 startValue;
		glm::vec3 endValue;
		float duration; // Duración en segundos
		TweenType type;
		Callback onUpdate;
		Callback onComplete;

		// Tiempo inicial del tween
		std::chrono::steady_clock::time_point startTime;

		Tween(glm::vec3 start, glm::vec3 end, float duration, TweenType type, Callback update, Callback complete = nullptr)
			: startValue(start), endValue(end), duration(duration), type(type), onUpdate(update), onComplete(complete)
		{
			startTime = std::chrono::steady_clock::now();
		}
	};


	class TweenManager
	{
	public:
		//--INSTANCE ACCESS
		static TweenManager& GetInstance()
		{
			static TweenManager instance;
			return instance;
		}


		void AddTween(const Tween& tween);
		void Update(float deltaTime);

	private:
		// Constructor privado
		TweenManager() {}
		TweenManager(const TweenManager&) = delete;
		TweenManager& operator=(const TweenManager&) = delete;
		~TweenManager() {}

		std::vector<Tween> tweens;

		float CalculateTweenFactor(float t, TweenType type);
	};
}