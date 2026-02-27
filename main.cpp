#include <Windows.h>
#include <gl/glew.h>
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include <stdio.h>
#include "tiny_obj_loader.h"
#include <iostream>
#include "Turtle.cpp"

GLuint plantProgram;			// shader programs
GLuint plantVAO;			// the data to be displayed
GLuint plantBuffer;
int triangles;			// number of triangles
int window;

char *vertexName;
char *fragmentName;

double theta, phi;
double r;

float cx, cy, cz;

glm::mat4 projection;	// projection matrix
float eyex, eyey, eyez;	// eye position

float deltaT = 0.0f, currentTime = 0.0f;
float lastMouseX = NULL, lastMouseY = NULL;
float yaw = -90.0f, pitch = 0.0f;
float speed = 50.0f;
float fov = 45.0f;
boolean keyUp = false, keyDown = false, keyLeft = false, keyRight = false, keySpace = false, keyShift = false;

glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 right;
glm::vec3 up;

std::vector<GLfloat> plant;
Turtle t = Turtle();
int generations;
float angle;
float animateT = 0;
bool animate = true;

void init() {
	GLuint vbuffer;
	GLuint ibuffer;
	GLint vPosition;
	GLint vNormal;
	int vs;
	int fs;
	int i;
	float xmin, xmax, ymin, ymax;
	float step = 1;
	std::string axiom;

	glGenVertexArrays(1, &plantVAO);
	glBindVertexArray(plantVAO);
	
	axiom = "X";
	angle = 22.5f;
	generations = 6;

	t.addRule("X -> F-[[X]+X]+F[+FX]-X");
	t.addRule("F -> FF");
	
	t.setAngle(angle);
	t.setAxiom(axiom);
	t.setStep(step);
	plant = *t.interpret(t.iterate(generations));

	ymin = 1000000.0;
	ymax = -1000000.0;
	for (i = 0; i < plant.size() / 3; i++) {
		if (plant[3.0 * i + 1] < ymin)
			ymin = plant[3.0 * i + 1];
		if (plant[3.0 * i + 1] > ymax)
			ymax = plant[3.0 * i + 1];
	}
	eyex = 0;
	eyey = (ymin + ymax) / 2;
	eyez = 150.0;

	std::cout << "Ready" << std::endl;

	/*
	 *  compile and build the shader program
	 */
	vs = buildShader(GL_VERTEX_SHADER, (char*) "LSystem.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, (char*) "LSystem.fs");
	plantProgram = buildProgram(vs, fs, 0);

	glGenVertexArrays(1, &plantVAO);
	glBindVertexArray(plantVAO);
	glGenBuffers(1, &plantBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, plantBuffer);
	glBufferData(GL_ARRAY_BUFFER, plant.size() * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glUseProgram(plantProgram);

	vPosition = glGetAttribLocation(plantProgram, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
}

/*
 *  Executed each time the window is resized,
 *  usually once at the start of the program.
 */
void framebufferSizeCallback(GLFWwindow *window, int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0f * w / h;

	glfwMakeContextCurrent(window);

	glViewport(0, 0, w, h);

	projection = glm::perspective(0.7f, ratio, 1.0f, 800.0f);
}

/*
 *  This procedure is called each time the screen needs
 *  to be redisplayed
 */
void display() {
	glm::mat4 view;
	int viewLoc;
	int projLoc;

	glm::vec3 position = glm::vec3(eyex, eyey, eyez);

	if (keyLeft)
		position += right * deltaT * speed;
	if (keyRight)
		position -= right * deltaT * speed;
	if (keyUp)
		position += direction * deltaT * speed;
	if (keyDown)
		position -= direction * deltaT * speed;
	if (keySpace)
		position.y += deltaT * speed;
	if (keyShift)
		position.y -= deltaT * speed;

	eyex = position.x;
	eyey = position.y;
	eyez = position.z;

	view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
			glm::vec3(eyex, eyey, eyez) + direction,
			up);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(plantProgram);

	glBindVertexArray(plantVAO);
	glDrawArrays(GL_LINE_STRIP, 0, 1);
	glBindBuffer(GL_ARRAY_BUFFER, plantBuffer);
	glBufferData(GL_ARRAY_BUFFER, plant.size() * sizeof(GLfloat), plant.data(), GL_STATIC_DRAW);

	glLineWidth(2);

	if (animate) {
		t.setAngle((2 * sin(0.8 * animateT) + angle));
		plant.clear();

		plant = *t.interpret(t.getCondition());
	}
	viewLoc = glGetUniformLocation(plantProgram, "modelView");
	glUniformMatrix4fv(viewLoc, 1, 0, glm::value_ptr(view));
	projLoc = glGetUniformLocation(plantProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, 0, glm::value_ptr(projection));
	glBindVertexArray(plantVAO);
	glDrawArrays(GL_LINE_STRIP, 0, plant.size() / 3);
}

void updateCamera() {
	glm::vec3 newDirection;
	newDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newDirection.y = sin(glm::radians(pitch));
	newDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction = glm::normalize(newDirection);

	glm::vec3 upY = glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(upY, direction));
	up = glm::cross(direction, right);
}

/*
 *  Called each time a key is pressed on
 *  the keyboard.
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_A)
		keyLeft = action == GLFW_PRESS || action == GLFW_REPEAT;
	if (key == GLFW_KEY_D)
		keyRight = action == GLFW_PRESS || action == GLFW_REPEAT;
	if (key == GLFW_KEY_W)
		keyUp = action == GLFW_PRESS || action == GLFW_REPEAT;
	if (key == GLFW_KEY_S)
		keyDown = action == GLFW_PRESS || action == GLFW_REPEAT;
	if (key == GLFW_KEY_SPACE)
		keySpace = action == GLFW_PRESS || action == GLFW_REPEAT;
	if (key == GLFW_KEY_LEFT_SHIFT)
		keyShift = action == GLFW_PRESS || action == GLFW_REPEAT;

	switch (key) {
		/****************
		  2D L - system
		*****************/
		case GLFW_KEY_1:
			angle = 25.7f;
			generations = 4;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("F");
			t.setStep(1);

			t.addRule("F -> F[+F]F[-F]F");
			plant = *t.interpret(t.iterate(generations));
			break;
		case GLFW_KEY_2:
			angle = 20.0f;
			generations = 5;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("F");
			t.setStep(1);

			t.addRule("F -> F[+F]F[-F][F]");
			plant = *t.interpret(t.iterate(generations));
			break;
		case GLFW_KEY_3:
			angle = 22.5f;
			generations = 4;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("F");
			t.setStep(1);

			t.addRule("F -> FF-[-F+F+F]+[+F-F-F]");
			plant = *t.interpret(t.iterate(generations));
			break;
		case GLFW_KEY_4:
			angle = 20.0f;
			generations = 7;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("X");
			t.setStep(1);

			t.addRule("X -> F[+X]F[-X]+X");
			t.addRule("F -> FF");
			plant = *t.interpret(t.iterate(generations));
			break;
		case GLFW_KEY_5:
			angle = 25.7f;
			generations = 7;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("X");
			t.setStep(1);

			t.addRule("X -> F[+X][-X]FX");
			t.addRule("F -> FF");
			plant = *t.interpret(t.iterate(generations));
			break;
		case GLFW_KEY_6:
			angle = 22.5f;
			generations = 6;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("X");
			t.setStep(1);

			t.addRule("X -> F-[[X]+X]+F[+FX]-X");
			t.addRule("F -> FF");
			plant = *t.interpret(t.iterate(generations));
			break;
		/****************
		   3D L - system
		*****************/
		//3D Hilbert Curve
		case GLFW_KEY_7:
			angle = 90.0f;
			generations = 4;
			animate = false;
			animateT = 0;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("X");
			t.setStep(1);

			t.addRule("X -> ^\\XF^\\XFX-F^//XFX&F+//XFX-F/X-/");
			plant = *t.interpret(t.iterate(generations));
			break;
		//3D Fern
		case GLFW_KEY_8:
			angle = 4.0f;
			generations = 12;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("EEEA");
			t.setStep(1);

			t.addRule("A -> [++++++++++++++EC]B^+B[--------------ED]B+BA");
			t.addRule("C -> [---------EE][+++++++++EE]B&&+C");
			t.addRule("D -> [---------EE][+++++++++EE]B&&-D");
			plant = *t.interpret(t.iterate(generations));
			break;
		// 3D Vine
		case GLFW_KEY_9:
			angle = 18.0f;
			generations = 4;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("A");
			t.setStep(0.5);

			t.addRule("A -> /A[++A]-\\A[--A]+//A");
			plant = *t.interpret(t.iterate(generations));
			break;
		// 3D tree
		case GLFW_KEY_0:
			angle = 18.0f;
			generations = 6;
			animate = true;

			t.clearRules();
			t.setAngle(angle);
			t.setAxiom("BBBBBA");
			t.setStep(3);

			t.addRule("A -> [++BB[--C][++C][&&C][^^C]A]/////+BBB[--C][++C][&&C][^^C]A");
			t.addRule("C -> ");
			plant = *t.interpret(t.iterate(generations));
			break;
		default:
			break;
	}

	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		plant.clear();
		generations++;
		t.iterate(generations);
		plant = *t.interpret(t.getCondition());
		std::cout << "Generations: " << generations << std::endl;
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		plant.clear();
		generations = generations == 0 ? 0 : generations - 1;
		t.iterate(generations);
		plant = *t.interpret(t.getCondition());
		std::cout << "Generations: " << generations << std::endl;
	}

	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		animate = !animate;
	}
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!lastMouseX || !lastMouseY) {
		lastMouseX = xpos;
		lastMouseY = ypos;
	}

	float xoffset = xpos - lastMouseX;
	float yoffset = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	updateCamera();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;

	projection = glm::perspective(glm::radians(fov), 1.0f, 1.0f, 800.0f);

}

void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char **argv) {
	GLFWwindow *window;

	if(argc > 1) {
		vertexName = argv[1];
	} else {
		vertexName = (char*) "a";
	}
	if(argc > 2) {
		fragmentName = argv[2];
	} else {
		fragmentName = (char*) "a";
	}

	// start by setting error callback in case something goes wrong

	glfwSetErrorCallback(error_callback);

	// initialize glfw

	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
	}

	// create the window used by our application

	window = glfwCreateWindow(800, 800, "Final", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// establish framebuffer size change and input callbacks

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetScrollCallback(window, scroll_callback);
	/*
	 *  initialize glew
	 */
	glfwMakeContextCurrent(window);
	GLenum error = glewInit();
	if(error != GLEW_OK) {
		printf("Error starting GLEW: %s\n",glewGetErrorString(error));
		exit(0);
	}
	
	init();
	updateCamera();

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0,1.0,1.0,1.0);
	glViewport(0, 0, 800, 800);

	projection = glm::perspective(glm::radians(fov), 1.0f, 1.0f, 800.0f);

	glfwSwapInterval(1);

	// GLFW main loop, display model, swapbuffer and check for input
	float lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		currentTime = glfwGetTime();
		deltaT = currentTime - lastTime;
		lastTime = currentTime;

		if (animate)
			animateT += deltaT;

		if (animateT > 1000)
			animate -= 1000;

		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
}