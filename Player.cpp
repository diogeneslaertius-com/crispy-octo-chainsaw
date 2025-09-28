#include "Player.h"
#include "Terrain.h"
#include <glm/gtc/matrix_transform.hpp>

Player::Player()
	: position(0.0f, 10.0f, 0.0f), velocity(0.0f), inputDir(0.0f) {}

void Player::Update(float deltaTime, const Terrain& terrain) {
	// нормализуем ввод
	if (glm::length(inputDir) > 0.0f)
		inputDir = glm::normalize(inputDir);

	// движение по XZ
	glm::vec3 horizontalVel = inputDir * moveSpeed;
	velocity.x = horizontalVel.x;
	velocity.z = horizontalVel.z;

	// применяем гравитацию
	ApplyGravity(deltaTime, terrain);

	// обновляем позицию
	position += velocity * deltaTime;

	// сбрасываем ввод (чтобы не накапливался)
	inputDir = glm::vec3(0.0f);
}

void Player::ApplyGravity(float deltaTime, const Terrain& terrain) {
	float groundHeight = terrain.GetHeightAt(position.x, position.z);

	if (position.y <= groundHeight) {
		position.y = groundHeight;
		velocity.y = 0.0f;
		isGrounded = true;
	}
	else {
		velocity.y -= gravity * deltaTime;
		isGrounded = false;
	}
}

void Player::MoveForward(float value) {
	inputDir.z += value; // вперёд/назад
}

void Player::MoveRight(float value) {
	inputDir.x += value; // влево/вправо
}

void Player::Jump() {
	if (isGrounded) {
		velocity.y = jumpForce;
		isGrounded = false;
	}
}

const glm::vec3& Player::GetPosition() const {
	return position;
}

void Player::SetPosition(const glm::vec3& pos) {
	position = pos;
}
