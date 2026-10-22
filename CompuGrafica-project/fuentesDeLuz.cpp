#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera (cerca de la escena)
Camera camera(glm::vec3(0.0f, 2.5f, 7.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

// Luces en las 4 esquinas (cerca del suelo)
glm::vec3 pointLightPositions[] = {
	glm::vec3(-3.0f, 0.5f, -3.0f), // Esquina 1 (Fondo Izquierda)
	glm::vec3(3.0f, 0.5f, -3.0f),  // Esquina 2 (Fondo Derecha)
	glm::vec3(3.0f, 0.5f,  3.0f),  // Esquina 3 (Frente Derecha)
	glm::vec3(-3.0f, 0.5f,  3.0f)  // Esquina 4 (Frente Izquierda)
};

// Luz seleccionada para mover (0=Rojo, 1=Verde, 2=Azul, 3=Amarillo)
int selectedLight = 0;

// Colores de las luces y de los cubos
glm::vec3 pointLightColors[] = {
	glm::vec3(1.0f, 0.0f, 0.0f), // Rojo
	glm::vec3(0.0f, 1.0f, 0.0f), // Verde
	glm::vec3(0.0f, 0.0f, 1.0f), // Azul
	glm::vec3(1.0f, 1.0f, 0.0f)  // Amarillo
};

// Intensidad de las luces puntuales
const float POINT_AMBIENT = 0.3f;
const float POINT_DIFFUSE = 8.0f;
const float POINT_SPECULAR = 6.0f;

// Modo de prueba de luces puntuales
bool lightOn[4] = { true, true, true, true }; // encendido de cada luz puntual
bool onlyPoints = false;                       // true = apaga luz direccional y linterna

// ---------- Ciclo dia/noche ----------
const float NIGHT_TIME = 4.71f;          // medianoche (sin = -1)
const float DAY_TIME = 1.57f;            // mediodia   (sin = +1)
float dayTime = DAY_TIME;                // empieza de dia

// Cuanto dura el dia antes de pasar a la noche (en segundos)
const float DAY_DURATION = 10.0f;
float holdTimer = 0.0f;                  // segundos transcurridos de dia
bool waitingForNight = true;             // true = contando los 10 segundos de dia
bool holdAfterArrival = false;           // true = al llegar al dia, empezar a contar

// Transicion suave hacia una hora objetivo (con aceleracion y frenado)
bool goingToTarget = false;
float targetTime = 0.0f;
float transSpeed = 0.0f;                 // velocidad actual de la transicion
const float TRANSITION_MAX = 0.6f;       // velocidad maxima (rad/s). Baja para algo mas lento
const float TRANSITION_ACCEL = 0.8f;     // que tan suave arranca y frena
const float TWO_PI = 6.2831853f;

// Funcion smoothstep propia (curva suave entre 0 y 1)
float Smooth(float edge0, float edge1, float x)
{
	float t = glm::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

float vertices[] = {
	-0.5f, -0.5f, -0.5f,   0.0f,   0.0f, -1.0f,
	 0.5f, -0.5f, -0.5f,   0.0f,   0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,   0.0f,   0.0f, -1.0f,
	 0.5f,  0.5f, -0.5f,   0.0f,   0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,   0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,   0.0f,   0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,   0.0f,   0.0f,  1.0f,
	 0.5f, -0.5f,  0.5f,   0.0f,   0.0f,  1.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,   0.0f,  1.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,   0.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,   0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,   0.0f,   0.0f,  1.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,   0.0f,   0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,   0.0f,   0.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,   0.0f,   0.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,   0.0f,   0.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,   0.0f,   0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,   0.0f,   0.0f,

	 0.5f,  0.5f,  0.5f,   1.0f,   0.0f,   0.0f,
	 0.5f,  0.5f, -0.5f,   1.0f,   0.0f,   0.0f,
	 0.5f, -0.5f, -0.5f,   1.0f,   0.0f,   0.0f,
	 0.5f, -0.5f, -0.5f,   1.0f,   0.0f,   0.0f,
	 0.5f, -0.5f,  0.5f,   1.0f,   0.0f,   0.0f,
	 0.5f,  0.5f,  0.5f,   1.0f,   0.0f,   0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f,  -1.0f,   0.0f,
	 0.5f, -0.5f, -0.5f,   0.0f,  -1.0f,   0.0f,
	 0.5f, -0.5f,  0.5f,   0.0f,  -1.0f,   0.0f,
	 0.5f, -0.5f,  0.5f,   0.0f,  -1.0f,   0.0f,
	-0.5f, -0.5f,  0.5f,   0.0f,  -1.0f,   0.0f,
	-0.5f, -0.5f, -0.5f,   0.0f,  -1.0f,   0.0f,

	-0.5f,  0.5f, -0.5f,   0.0f,   1.0f,   0.0f,
	 0.5f,  0.5f, -0.5f,   0.0f,   1.0f,   0.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,   1.0f,   0.0f,
	 0.5f,  0.5f,  0.5f,   0.0f,   1.0f,   0.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,   1.0f,   0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,   1.0f,   0.0f
};

glm::vec3 Light1 = glm::vec3(0);

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
	// Init GLFW
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Fuentes de luz", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

	Model Dog((char*)"Models/ball.obj");
	Model Piso((char*)"Models/piso.obj");

	// VAO/VBO del cubo
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();

		// ---------- Temporizador: 10 segundos de dia y luego noche ----------
		if (waitingForNight)
		{
			holdTimer += deltaTime;
			if (holdTimer >= DAY_DURATION)
			{
				waitingForNight = false;
				targetTime = NIGHT_TIME;
				goingToTarget = true;
			}
		}

		// ---------- Avance gradual hacia la hora objetivo ----------
		if (goingToTarget)
		{
			// Distancia hacia adelante hasta la hora objetivo (siempre avanza)
			float diff = fmod(targetTime - dayTime, TWO_PI);
			if (diff < 0.0f) diff += TWO_PI;

			// Acelera poco a poco y frena antes de llegar
			transSpeed = std::min(transSpeed + TRANSITION_ACCEL * deltaTime, TRANSITION_MAX);
			float allowed = std::max(sqrt(2.0f * TRANSITION_ACCEL * diff), 0.05f);
			float step = std::min(transSpeed, allowed) * deltaTime;

			if (diff <= step)
			{
				dayTime = targetTime;
				goingToTarget = false;
				transSpeed = 0.0f;

				// Si llegamos al dia tras un clic, empezar a contar los 10 segundos
				if (holdAfterArrival)
				{
					holdAfterArrival = false;
					holdTimer = 0.0f;
					waitingForNight = true;
				}
			}
			else
			{
				dayTime += step;
			}
		}

		float sunHeight = sin(dayTime);   // >0 dia, <0 noche

		// 0 = noche completa, 1 = dia completo (rango ancho y curva suave)
		float dayFactor = Smooth(-0.3f, 0.5f, sunHeight);

		// Atardecer / amanecer: mas largo y suave
		float sunsetFactor = Smooth(0.0f, 1.0f, glm::clamp(1.0f - fabs(sunHeight) / 0.5f, 0.0f, 1.0f));

		// Misma luz direccional: de dia es el sol, de noche la luna.
		// Cada uno se apaga poco a poco al llegar al horizonte (lightPower = 0),
		// asi el cambio de direccion entre sol y luna no se nota.
		glm::vec3 lightSrc;
		float lightPower;
		if (sunHeight >= 0.0f)
		{
			lightSrc = glm::vec3(cos(dayTime), sunHeight + 0.05f, 0.3f);
			lightPower = Smooth(0.0f, 0.5f, sunHeight);
		}
		else
		{
			lightSrc = glm::vec3(-cos(dayTime), -sunHeight + 0.05f, 0.3f);
			lightPower = Smooth(0.0f, 0.5f, -sunHeight);
		}
		glm::vec3 sunDir = -glm::normalize(lightSrc);      // direccion de la luz

		glm::vec3 dayAmbient(0.30f, 0.30f, 0.30f);
		glm::vec3 nightAmbient(0.01f, 0.01f, 0.03f);   // noche oscura
		glm::vec3 dayDiffuse(0.9f, 0.85f, 0.75f);
		glm::vec3 nightDiffuse(0.04f, 0.05f, 0.12f);   // luz de luna azulada tenue
		glm::vec3 sunsetColor(1.0f, 0.45f, 0.15f);

		// Ambiente: mezcla suave + resplandor naranja en el atardecer
		glm::vec3 dirAmbient = glm::mix(nightAmbient, dayAmbient, dayFactor);
		dirAmbient += sunsetColor * 0.08f * sunsetFactor;

		// Difusa: color segun la hora, con tinte naranja cerca del horizonte,
		// y su intensidad sube/baja suavemente con lightPower
		glm::vec3 dirDiffuse = glm::mix(nightDiffuse, dayDiffuse, dayFactor);
		dirDiffuse = glm::mix(dirDiffuse, sunsetColor * 0.8f, sunsetFactor * 0.6f);
		dirDiffuse *= lightPower;

		glm::vec3 dirSpecular = glm::vec3(0.5f) * dayFactor * lightPower;

		// Modo prueba: solo las luces puntuales
		if (onlyPoints)
		{
			dirAmbient = glm::vec3(0.0f);
			dirDiffuse = glm::vec3(0.0f);
			dirSpecular = glm::vec3(0.0f);
		}

		// Fondo negro fijo (sin color de cielo): el dia/noche se ve en la iluminacion
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);

		lightingShader.Use();

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// Directional light (sol / luna)
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), sunDir.x, sunDir.y, sunDir.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), dirAmbient.r, dirAmbient.g, dirAmbient.b);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), dirDiffuse.r, dirDiffuse.g, dirDiffuse.b);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), dirSpecular.r, dirSpecular.g, dirSpecular.b);

		// Point lights: una por esquina, cada una de un color, muy intensas
		for (int i = 0; i < 4; i++)
		{
			std::string base = "pointLights[" + std::to_string(i) + "].";
			glm::vec3 c = pointLightColors[i] * (lightOn[i] ? 1.0f : 0.0f);

			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "position").c_str()),
				pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "ambient").c_str()),
				c.r * POINT_AMBIENT, c.g * POINT_AMBIENT, c.b * POINT_AMBIENT);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "diffuse").c_str()),
				c.r * POINT_DIFFUSE, c.g * POINT_DIFFUSE, c.b * POINT_DIFFUSE);
			glUniform3f(glGetUniformLocation(lightingShader.Program, (base + "specular").c_str()),
				c.r * POINT_SPECULAR, c.g * POINT_SPECULAR, c.b * POINT_SPECULAR);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "constant").c_str()), 1.0f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "linear").c_str()), 0.014f);
			glUniform1f(glGetUniformLocation(lightingShader.Program, (base + "quadratic").c_str()), 0.0007f);
		}

		// SpotLight (se apaga en el modo prueba)
		float spotOn = onlyPoints ? 0.0f : 1.0f;
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.3f * spotOn, 0.3f * spotOn, 0.3f * spotOn);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.2f * spotOn, 0.2f * spotOn, 0.2f * spotOn);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));

		// Material
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

		glm::mat4 view = camera.GetViewMatrix();

		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model(1);

		// Piso (sin transparencia)
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
		Piso.Draw(lightingShader);

		// Dog / Ball
		model = glm::mat4(1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
		Dog.Draw(lightingShader);
		glDisable(GL_BLEND);
		glBindVertexArray(0);

		// Cubos de las lamparas, cada uno con el color de su luz
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");
		GLint lightColorLoc = glGetUniformLocation(lampShader.Program, "lightColor");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		for (GLuint i = 0; i < 4; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.3f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			// Cubo gris oscuro cuando su luz esta apagada
			glm::vec3 cubeColor = lightOn[i] ? pointLightColors[i] : glm::vec3(0.15f);
			glUniform3f(lightColorLoc, cubeColor.r, cubeColor.g, cubeColor.b);

			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	// Camara
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	// Elegir que luz mover
	if (keys[GLFW_KEY_1]) selectedLight = 0; // Rojo
	if (keys[GLFW_KEY_2]) selectedLight = 1; // Verde
	if (keys[GLFW_KEY_3]) selectedLight = 2; // Azul
	if (keys[GLFW_KEY_4]) selectedLight = 3; // Amarillo

	// Mover la luz seleccionada
	float speed = 3.0f * deltaTime;

	if (keys[GLFW_KEY_T]) pointLightPositions[selectedLight].x += speed;
	if (keys[GLFW_KEY_G]) pointLightPositions[selectedLight].x -= speed;

	if (keys[GLFW_KEY_Y]) pointLightPositions[selectedLight].y += speed;
	if (keys[GLFW_KEY_H]) pointLightPositions[selectedLight].y -= speed;

	if (keys[GLFW_KEY_U]) pointLightPositions[selectedLight].z -= speed;
	if (keys[GLFW_KEY_J]) pointLightPositions[selectedLight].z += speed;
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Prueba de luces puntuales
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_5) lightOn[0] = !lightOn[0]; // Rojo
		if (key == GLFW_KEY_6) lightOn[1] = !lightOn[1]; // Verde
		if (key == GLFW_KEY_7) lightOn[2] = !lightOn[2]; // Azul
		if (key == GLFW_KEY_8) lightOn[3] = !lightOn[3]; // Amarillo
		if (key == GLFW_KEY_P) onlyPoints = !onlyPoints;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
		{
			Light1 = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		else
		{
			Light1 = glm::vec3(0.0f);
		}
	}
}

// Clic izquierdo: reinicia el ciclo (vuelve a dia, espera 10 s y pasa a noche)
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		waitingForNight = false;     // detener la cuenta actual
		holdTimer = 0.0f;
		holdAfterArrival = true;     // al llegar al dia, contar 10 segundos
		targetTime = DAY_TIME;       // volver al dia de forma gradual
		goingToTarget = true;
	}
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}