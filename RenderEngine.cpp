#include "RenderEngine.h"
RenderEngine::RenderEngine() {
}


RenderEngine::~RenderEngine() {
	glfwDestroyWindow(window);
}



void RenderEngine::Start(const char* title, unsigned int width, unsigned int height, bool vsync, bool fullscreen) {

	// set app configuration
	this->screenHeight = height;
	this->screenWidth = width;

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	window = glfwCreateWindow(this->screenWidth, this->screenHeight, title, fullscreen ? monitor : NULL, NULL);
	if (window == NULL)
	{
		Err("Failed to create GLFW window");
	}

	// set window position on center of screen
	// ---------------------------------------
	if (!fullscreen) {
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowPos(window, mode->width / 4, mode->height / 4);
	}

	// set opengl context
	// ------------------
	glfwMakeContextCurrent(window);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		Err("Failed to initialize GLAD");
	}

	// set vsync
	// ---------
	glfwSwapInterval(vsync ? 1 : 0);

	// user defined function
	// ---------------------
	Init();

	lastFrame = glfwGetTime() * 1000;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// Calculate framerate and frametime
		double deltaTime = GetDeltaTime();
		GetFPS();

		// user defined function
		// ---------------------
		ProcessInput(window);
		Update(deltaTime);
		Render();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

		//Debug print framerate
		PrintFrameRate();
	}

	// user defined function
	// ---------------------
	DeInit();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
}

double RenderEngine::GetDeltaTime()
{

	double time = glfwGetTime() * 1000;
	double delta = time - lastFrame;
	lastFrame = time;

	return delta;
}

void RenderEngine::GetFPS()
{
	if (glfwGetTime() * 1000 - last > 1000) {
		fps = _fps;
		_fps = 0;
		last += 1000;
	}
	_fps++;
}


//Prints out an error message and exits the game
void RenderEngine::Err(std::string errorString)
{
	std::cout << errorString << std::endl;
	glfwTerminate();
	exit(1);
}

static int frameCounter = 0;
void RenderEngine::PrintFrameRate() {
	//print only once every 60 frames
	frameCounter++;
	if (frameCounter == 60) {
		std::cout << "FPS: " << fps << std::endl;
		frameCounter = 0;
	}
}

void RenderEngine::CheckShaderErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			Err("| ERROR::::SHADER-COMPILATION-ERROR of type: " + type + "|\n" + infoLog + "\n| -- --------------------------------------------------- -- |");
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			Err("| ERROR::::PROGRAM-LINKING-ERROR of type: " + type + "|\n" + infoLog + "\n| -- --------------------------------------------------- -- |");
		}
	}
}

GLuint RenderEngine::BuildShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	// 1. Retrieve the vertex/fragment source code from filePath
	std::string vertexCode, fragmentCode, geometryCode;
	std::ifstream vShaderFile, fShaderFile, gShaderFile;
	// ensures ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// Open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// Read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// Convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// If geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		Err("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
	}
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar * fShaderCode = fragmentCode.c_str();
	// 2. Compile shaders
	GLuint vertex, fragment;

	// Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	CheckShaderErrors(vertex, "VERTEX");
	// Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	CheckShaderErrors(fragment, "FRAGMENT");
	// If geometry shader is given, compile geometry shader
	GLuint geometry;
	if (geometryPath != nullptr)
	{
		const GLchar * gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		CheckShaderErrors(geometry, "GEOMETRY");
	}
	// Shader Program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	if (geometryPath != nullptr)
		glAttachShader(program, geometry);
	glLinkProgram(program);
	CheckShaderErrors(program, "PROGRAM");
	// Delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);
	return program;

}

void RenderEngine::UseShader(GLuint program)
{
	// Uses the current shader
	glUseProgram(program);
}




