#include <GL/glew.h>
#include "Beam.h"
#include <glm/gtc/matrix_transform.hpp>

// �����������
Beam::Beam(const glm::vec3& start, const glm::vec3& dir, float length, float lifeTime, const glm::vec3& color)
	: m_Start(start), m_Dir(glm::normalize(dir)), m_Length(length), m_LifeTime(lifeTime), m_Color(color)
{}

// ���������� ������� �����
void Beam::Update(float deltaTime) {
	m_LifeTime -= deltaTime;
}

glm::mat4 Beam::GetModelMatrix() const {
	// ������� ��� ������/��������
	glm::mat4 model = glm::mat4(1.0f);

	// ��������� � ������ ����
	model = glm::translate(model, m_Start);

	// ����������� �� ����������� (���� ����� � ����� �������� lookAt)
	// ���� ������ ������������ �� �����
	model = glm::scale(model, glm::vec3(m_Length, 0.05f, 0.05f));

	return model;
}
// ��������� (���������� ������� ����� �����)
void Beam::Draw() {
	glBegin(GL_LINES);
	glColor3f(m_Color.r, m_Color.g, m_Color.b);
	glVertex3f(m_Start.x, m_Start.y, m_Start.z);
	glm::vec3 end = m_Start + m_Dir * m_Length;
	glVertex3f(end.x, end.y, end.z);
	glEnd();
}
