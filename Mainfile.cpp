#include <algorithm>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Мои заголовки
#include "Shader.h"
#include "Camera.h"
#include "Render.h"
#include "Mechanics.h"
#include "Model.h"
#include "Terrain.h"
#include "Beam.h"
#include "EnemyPool.h"
#include "Player.h"

// Заголовки ImGui
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h> 


void InitImGui(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Темная тема (идеально для киберпанк игры)
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}


int main(void)
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	// Настраиваем контекст OpenGL для нужной версии
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(800, 600, "Time Shooter", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "GLEW initialization failed!\n";
		glfwTerminate();
		return -1;
	}

	std::cout << "OpenGL ver " << glGetString(GL_VERSION) << std::endl;
	glfwSwapInterval(1);
	Shader shader("vertex.glsl", "fragment.glsl");

	Renderer::InitGLSettings();
	Renderer::CheckError();

	const float FOV = 45.0f;
	const float NEAR_PLANE = 0.1f;
	const float FAR_PLANE = 100.0f;

	const int HIT = 0;
	const int DEATH = 1;
	const int PLAYER_MOVE = 2;
	const int BULLET_SPAWN = 3;

	// Переменная Time Energy объявлена здесь
	float timeEnergy = 100.0f;

	// Инициализация камеры
	glm::vec3 cameraPos(0.0f, 2.0f, 3.0f);
	Camera camera(cameraPos);
	Mechanics mechanicsHandler(&camera);

	// === НОВОЕ: СИСТЕМА ВРАГОВ ===
	EnemyPool enemyPool(20); // пул на 20 врагов
	float waveTimer = 0.0f;
	int currentWave = 1;
	bool waveActive = false;

	// Настройка ввода
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(window, &mechanicsHandler);
	glfwSetCursorPosCallback(window, Mechanics::MouseCallback);

	// === ИНИЦИАЛИЗАЦИЯ IMGUI ===
	InitImGui(window);

	std::vector<Beam> beams;
	Model enemyModel("assets/cube.obj"); // используем тот же куб для врагов
	Terrain terrain(100.0f, 0.0f);
	Player player;

	// в Update:

	float lastTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		float currentTime = (float)glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// === ЛОГИКА ИГРЫ ===
		mechanicsHandler.Update(deltaTime);
		mechanicsHandler.AddPlayerMoveEvent(camera.GetPosition());

		// --- обновление игрока ---
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) player.MoveForward(1.0f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) player.MoveForward(-1.0f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) player.MoveRight(-1.0f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) player.MoveRight(1.0f);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) player.Jump();

		player.Update(deltaTime, terrain);
		camera.SetPosition(player.GetPosition() + glm::vec3(0.0f, 1.8f, 0.0f));


		// === СИСТЕМА ВОЛН ===
		waveTimer += deltaTime;
		if (!waveActive && waveTimer > 3.0f) { // новая волна каждые 3 секунды
			// Спавним врагов по кругу вокруг игрока
			glm::vec3 playerPos = camera.GetPosition();
			for (int i = 0; i < currentWave + 2; i++) {
				float angle = (float)i / (currentWave + 2) * 2.0f * 3.14159f;
				glm::vec3 spawnPos = playerPos + glm::vec3(
					cos(angle) * 8.0f,  // радиус 8 метров
					0.0f,
					sin(angle) * 8.0f
				);
				enemyPool.Spawn(spawnPos);
			}

			std::cout << "=== WAVE " << currentWave << " SPAWNED ===\n";
			currentWave++;
			waveActive = true;
			waveTimer = 0.0f;
		}

		// Обновляем врагов
		enemyPool.UpdateAll(deltaTime, camera.GetPosition());

		// Проверяем, закончилась ли волна
		bool hasActiveEnemies = false;
		for (const auto& enemy : enemyPool.GetEnemies()) {
			if (enemy.IsActive()) {
				hasActiveEnemies = true;
				break;
			}
		}
		if (waveActive && !hasActiveEnemies) {
			waveActive = false;
			std::cout << "Wave cleared! Next wave in 3 seconds...\n";
		}

		// === ОБНОВЛЕНИЕ КАМЕРЫ ===
		Renderer::UpdateViewport(window);
		glm::mat4 view = camera.ProcessInput(window, deltaTime);
		glm::mat4 projection = camera.GetProjectionMatrix(window, FOV, NEAR_PLANE, FAR_PLANE);

		// === ЛОГИКА СТРЕЛЬБЫ ===
		if (mechanicsHandler.CheckForShot(window)) {
			glm::vec3 rayOrigin = camera.GetPosition();
			glm::vec3 rayDir = camera.GetFront();

			// Проверяем попадание по врагам
			for (auto& enemy : enemyPool.GetEnemies()) {
				if (!enemy.IsActive()) continue;

				// ... (Ваша логика рейкаста) ...
				glm::vec3 enemyPos = enemy.GetPosition();
				glm::vec3 toEnemy = enemyPos - rayOrigin;
				float projectionHit = glm::dot(toEnemy, rayDir);

				if (projectionHit > 0 && projectionHit < 50.0f) { // максимальная дистанция
					glm::vec3 closestPoint = rayOrigin + rayDir * projectionHit;
					float distanceToRay = glm::length(enemyPos - closestPoint);

					if (distanceToRay < 1.0f) { // радиус попадания
						enemy.TakeDamage(50.0f);
						// Добавляем событие попадания в историю времени
						mechanicsHandler.AddEvent(HIT, enemyPos, 0);
						std::cout << "HIT! Enemy HP: " << enemy.GetHP() << "\n";
						break; // один выстрел = одно попадание
					}
				}
			}

			// Создаем визуальный луч
			beams.emplace_back(rayOrigin, rayDir, 20.0f, 0.2f, glm::vec3(1.0f, 0.2f, 1.0f));
		}

		// === ЛОГИКА ОТМЕНЫ ВРЕМЕНИ (REWIND) ===
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && timeEnergy > 0.0f)
		{
			// Уменьшаем энергию (зависит от deltaTime)
			float energyCost = 50.0f * deltaTime; // Например, 50 единиц в секунду

			// Проверяем, осталось ли достаточно энергии
			if (timeEnergy - energyCost > 0.0f) 
			{
				mechanicsHandler.RewindTime(deltaTime); // Отматываем на маленький шаг
				timeEnergy -= energyCost;

				// =========================================================================
				// === TODO: ВОССТАНОВЛЕНИЕ СОСТОЯНИЯ ВРАГОВ (Crucial for game state) ===
				// =========================================================================

				// 1. Получаем состояние мира из механики времени

				// 2. Обновляем врагов: проходим по активным врагам и устанавливаем их
				//    позиции, HP и состояния на основе данных из rewindState.
				// enemyPool.RestoreState(rewindState.enemyStates);

			}
			else {
				// Если энергии недостаточно, отматываем только на остаток и обнуляем
				mechanicsHandler.RewindTime(timeEnergy / 50.0f); // Отмотка на оставшееся время
				timeEnergy = 0.0f;
			}

		}
		else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
			// Восстановление энергии, если клавиша R не нажата
			timeEnergy = glm::min(timeEnergy + (10.0f * deltaTime), 100.0f); // Восстановление 10/сек
		}

		// =========================================
		// === ImGui и РЕНДЕРИНГ OpenGL ===
		// =========================================

		// === ImGui начало фрейма ===
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// === HUD через ImGui ===
		ImGui::Begin("HUD", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
		ImGui::SetWindowPos(ImVec2(10, 10)); // Устанавливаем положение
		ImGui::Text("Wave: %d", currentWave);
		int activeEnemies = 0;
		for (const auto& e : enemyPool.GetEnemies()) if (e.IsActive()) activeEnemies++;
		ImGui::Text("Active Enemies: %d", activeEnemies);
		ImGui::Text("Combo: %d", mechanicsHandler.GetHitsWithoutRewind());

		// Индикатор энергии отмены времени
		ImGui::Text("Time Energy");
		// Используем красное/зеленое смещение для энергии
		ImVec4 progressColor = ImVec4(1.0f - (timeEnergy / 100.0f), (timeEnergy / 100.0f), 0.0f, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progressColor);
		ImGui::ProgressBar(timeEnergy / 100.0f, ImVec2(200.0f, 0.0f), "");
		ImGui::PopStyleColor();

		// Дополнительный индикатор глитча
		float glitchIntensity = mechanicsHandler.GetGlitchIntensity();
		if (glitchIntensity > 0.01f) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "REWIND ACTIVE!");
		}

		ImGui::End();

		// === РЕНДЕРИНГ OpenGL ===
		Renderer::Clear(0.1f, 0.1f, 0.2f, 1.0f); // темный фон

		shader.Bind();
		shader.SetUniformMat4f("projection", projection);
		shader.SetUniformMat4f("view", view);
		shader.SetUniform1f("time", currentTime);

		// Глитч эффект (интенсивность зависит от механики времени)
		shader.SetUniform1f("glitchIntensity", glitchIntensity);

		// 1. Рендерим террейн
		glm::mat4 terrain_model = glm::mat4(1.0f);
		shader.SetUniformMat4f("model", terrain_model);
		shader.SetUniform4f("u_Color", 0.1f, 0.3f, 0.1f, 1.0f);
		terrain.Draw();

		// 2. Рендерим врагов
		for (const auto& enemy : enemyPool.GetEnemies()) {
			if (!enemy.IsActive()) continue;

			shader.SetUniformMat4f("model", enemy.GetModelMatrix());
			glm::vec4 color = enemy.GetColor();
			shader.SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
			enemyModel.Draw();
		}

		// 3. Рендерим лучи
		for (auto it = beams.begin(); it != beams.end();) {
			it->Update(deltaTime);
			if (!it->IsAlive()) {
				it = beams.erase(it);
			}
			else {
				shader.SetUniformMat4f("model", it->GetModelMatrix());
				glm::vec3 color = it->GetColor();
				shader.SetUniform4f("u_Color", color.r, color.g, color.b, 1.0f);
				it->Draw();
				++it;
			}
		}

		// === ImGui рендер ===
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// === КОНЕЦ ЦИКЛА ===
		Renderer::CheckError();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// =========================================
	// === ОЧИСТКА РЕСУРСОВ IMGUI ===
	// =========================================
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Очистка GLFW
	glfwTerminate();
	return 0;
}