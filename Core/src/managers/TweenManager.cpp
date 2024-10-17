#include "TweenManager.h"

namespace libCore
{
	void TweenManager::AddTween(const Tween& tween)
	{
		tweens.push_back(tween);
	}

	void TweenManager::Update(float deltaTime)
	{
		auto currentTime = std::chrono::steady_clock::now();

		for (auto it = tweens.begin(); it != tweens.end(); ) {
			Tween& tween = *it;

			// Calcular el tiempo transcurrido
			float elapsed = std::chrono::duration<float>(currentTime - tween.startTime).count();

			// Calcular la interpolación
			float t = glm::clamp(elapsed / tween.duration, 0.0f, 1.0f);
			float tweenFactor = CalculateTweenFactor(t, tween.type);

			// Interpolar entre el valor inicial y el final
			glm::vec3 newValue = glm::mix(tween.startValue, tween.endValue, tweenFactor);

			// Llamar al callback de actualización
			tween.onUpdate(newValue);

			// Comprobar si el tween ha terminado
			if (elapsed >= tween.duration) {
				if (tween.onComplete) {
					tween.onComplete(tween.endValue);
				}
				it = tweens.erase(it); // Eliminar el tween que ha terminado
			}
			else {
				++it;
			}
		}
	}

	float TweenManager::CalculateTweenFactor(float t, TweenType type)
	{
		switch (type) {
		case TweenType::LINEAR:
			return t;
		case TweenType::EASE_IN:
			return t * t;
		case TweenType::EASE_OUT:
			return t * (2.0f - t);
		case TweenType::EASE_IN_OUT:
			return (t < 0.5f) ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
		default:
			return t;
		}
	}
}
