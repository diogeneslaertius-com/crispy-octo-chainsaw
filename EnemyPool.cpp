#include "EnemyPool.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <random>

Enemy* EnemyPool::Spawn(const glm::vec3& pos) {
	for (auto& e : enemies) {
		if (!e.IsActive()) {
			e.Reset(pos);
			return &e;
		}
	}
	return nullptr; // пул заполнен
}

void EnemyPool::UpdateAll(float deltaTime, const glm::vec3& playerPos) {
	for (auto& e : enemies) {
		if (!e.IsActive()) continue;
		e.Update(deltaTime, playerPos);
	}
}

void EnemyPool::SpawnWaveCircle(int count, const glm::vec3& center, float radius, float jitter) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> jitterDis(-jitter, jitter);

	for (int i = 0; i < count; i++) {
		// Равномерно распределяем по кругу
		float angle = (float)i / count * 2.0f * glm::pi<float>();

		// Добавляем случайное отклонение
		float actualRadius = radius + jitterDis(gen);
		angle += jitterDis(gen) * 0.5f;

		glm::vec3 spawnPos = center + glm::vec3(
			cos(angle) * actualRadius,
			0.0f, // враги спавнятся на уровне земли
			sin(angle) * actualRadius
		);

		Spawn(spawnPos);
	}
}