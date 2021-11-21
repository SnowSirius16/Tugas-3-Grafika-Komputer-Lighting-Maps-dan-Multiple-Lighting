#pragma once
#pragma once

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class RenderEngine
{
public:
	RenderEngine();
	~RenderEngine();
	void Start(const char* title, unsigned int width, unsigned int height, bool vsync, bool fullscreen);
protected:
	unsigned int screenWidth, screenHeight, last = 0, _fps = 0, fps = 0;
	double lastFrame = 0;
	GLFWwindow* window;

	virtual void Init() = 0;
	virtual void DeInit() = 0;
	virtual void Update(double deltaTime) = 0;
	virtual void Render() = 0;
	virtual void ProcessInput(GLFWwindow *window) = 0;

	double GetDeltaTime();
	void GetFPS();
	void Err(std::string errorString);
	void PrintFrameRate();
	void CheckShaderErrors(GLuint shader, std::string type);
	GLuint BuildShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);
	void UseShader(GLuint program);
};

