#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

class Terrain {
public:
	Terrain(float size = 100.0f, float y = 0.0f);
	~Terrain();

	void Draw() const;

	// ����� ����� ��� ��������� ������ � �����
	float GetHeightAt(float x, float z) const;

private:
	unsigned int VAO, VBO, EBO;
	unsigned int indexCount;

	float baseY; // <-- ��� ����� � ���� �� �������

	void setupMesh(float size, float y);
};
