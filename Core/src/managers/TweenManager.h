#pragma once

#include "../core/Core.h"

namespace libCore
{
	enum class TweenAction {
		MOVE,
		SCALE,
		ROTATE
	};

	struct Tween {
		using Callback = std::function<void(const glm::vec3&)>;

		glm::vec3 startValue;
		glm::vec3 endValue;
		float duration; // Duración en segundos
		TweenType type;
		TweenAction action; // Agregar el tipo de acción (Move, Scale, Rotate)
		Callback onUpdate;
		Callback onComplete;

		// Tiempo inicial del tween
		std::chrono::steady_clock::time_point startTime;

		// Constructor actualizado para incluir TweenAction
		Tween(const glm::vec3& start, const glm::vec3& end, float duration, TweenType type, TweenAction action, Callback update, Callback complete = nullptr)
			: startValue(start), endValue(end), duration(duration), type(type), action(action), onUpdate(update), onComplete(complete)
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
