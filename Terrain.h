#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

class Terrain {
public:
	Terrain(float size = 100.0f, float y = 0.0f);
	~Terrain();

	void Draw() const;

	// Новый метод для получения высоты в точке
	float GetHeightAt(float x, float z) const;

private:
	unsigned int VAO, VBO, EBO;
	unsigned int indexCount;

	float baseY; // <-- вот этого у тебя не хватало

	void setupMesh(float size, float y);
};
