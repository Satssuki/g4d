#include <g4d/ShaderProgram.hpp>
#include <g4d/Transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

using namespace g4d;

GLFWwindow* window;

std::string readFile(const char* name)
{
	std::ifstream ifs(name);

	return std::string(
	    std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

std::unique_ptr<ShaderProgram> program;

void initProgram()
{
	Shader vertex(Shader::Type::Vertex);
	Shader geometry(Shader::Type::Geometry);
	Shader fragment(Shader::Type::Fragment);

	vertex.compile(readFile("../shaders/basic/vert.glsl"));
	geometry.compile(readFile("../shaders/basic/geom.glsl"));
	fragment.compile(readFile("../shaders/basic/frag.glsl"));

	assert(vertex.isCompiled());
	if(!geometry.isCompiled())
		std::cout << geometry.getLog() << std::endl;
	assert(geometry.isCompiled());
	assert(fragment.isCompiled());

	program = std::make_unique<ShaderProgram>();
	program->link(vertex, geometry, fragment);
	
	assert(program->isLinked());
}

float angle1;
float angle2;
float angle3;

void setUniforms()
{
	float invert = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) ? -1.0 : 1.0;

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		angle1 += invert * 0.011;
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		angle2 += invert * 0.011;
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		angle3 += invert * 0.011;

/*
	float x = glm::cos(angle1);
	float y = glm::sin(angle1) * glm::cos(angle2);
	float z = glm::sin(angle1) * glm::sin(angle2) * glm::cos(angle3);
	float w = glm::sin(angle1) * glm::sin(angle2) * glm::sin(angle3);
*/

	Transform view;
	view.lookAt(glm::dvec4(0, 0, 0, -3), glm::dvec4(),
	            glm::dvec4(0, 1, 0, 0), glm::dvec4(0, 0, 1, 0));

	Transform model;
	model.translate(.5, 0, 0, -1.5);
	model.rotate(angle1, glm::dvec4(1, 0, 0, 0), glm::dvec4(0, 0, 0, 1));
	model.rotate(angle2, glm::dvec4(0, 1, 0, 0), glm::dvec4(0, 0, 1, 0));
	model.rotate(angle3, glm::dvec4(0, 1, 1, 0), glm::dvec4(1, 0, 0, 1));

	Transform model_view = view * model;

	glm::mat4 projection = glm::perspective(45.0f, 1.33f, 0.1f, 100.0f);

	program->bind();

	glm::mat4 model_view_linear_map = model_view.getLinearMap();
	glm::vec4 model_view_translation = model_view.getTranslation();

	glUniformMatrix4fv(program->getUniformLocation("ModelViewLinearMap"),
	                   1, GL_FALSE, glm::value_ptr(model_view_linear_map));
	glUniform4fv(program->getUniformLocation("ModelViewTranslation"),
	             1, glm::value_ptr(model_view_translation));
	glUniformMatrix4fv(program->getUniformLocation("Projection"),
	                   1, GL_FALSE, glm::value_ptr(projection));

	program->release();
}

GLuint vbo[3];
GLuint vao;

void initModel()
{
	static const float hypercube[] = {
		  -0.5,  -0.5,  -0.5,  -0.5,
		  -0.5,  -0.5,  -0.5,   0.5,
		  -0.5,  -0.5,   0.5,  -0.5,
		  -0.5,  -0.5,   0.5,   0.5,
		  -0.5,   0.5,  -0.5,  -0.5,
		  -0.5,   0.5,  -0.5,   0.5,
		  -0.5,   0.5,   0.5,  -0.5,
		  -0.5,   0.5,   0.5,   0.5,
		   0.5,  -0.5,  -0.5,  -0.5,
		   0.5,  -0.5,  -0.5,   0.5,
		   0.5,  -0.5,   0.5,  -0.5,
		   0.5,  -0.5,   0.5,   0.5,
		   0.5,   0.5,  -0.5,  -0.5,
		   0.5,   0.5,  -0.5,   0.5,
		   0.5,   0.5,   0.5,  -0.5,
		   0.5,   0.5,   0.5,   0.5
	};
	
	static const float colors[] = {
		  0,  0,  0, 1,  
		  0,  0,  1, 1, 
		  0,  1,  0, 1, 
		  0,  1,  1, 1, 
		  1,  0,  0, 1, 
		  1,  0,  1, 1, 
		  1,  1,  0, 1, 
		  1,  1,  1, 1, 
		  0,  0,  0, 1,  
		  0,  0,  1, 1, 
		  0,  1,  0, 1, 
		  0,  1,  1, 1, 
		  1,  0,  0, 1, 
		  1,  0,  1, 1, 
		  1,  1,  0, 1, 
		  1,  1,  1, 1  
	};

	static const unsigned int indices[] = {
		9 , 13 , 14 , 15 ,
		12 , 9 , 13 , 14 ,
		9 , 11 , 13 , 15 ,
		11 , 9 , 14 , 15 ,
		9 , 10 , 11 , 14 ,
		12 , 9 , 14 , 8 ,
		9 , 12 , 13 , 8 ,
		9 , 10 , 14 , 8 ,
		10 , 9 , 11 , 8 ,
		6 , 2 , 4 , 0 ,
		6 , 4 , 8 , 0 ,
		2 , 6 , 8 , 0 ,
		6 , 10 , 14 , 2 ,
		12 , 6 , 14 , 4 ,
		12 , 6 , 4 , 8 ,
		6 , 12 , 14 , 8 ,
		6 , 10 , 2 , 8 ,
		10 , 6 , 14 , 8 ,
		4 , 5 , 8 , 0 ,
		1 , 5 , 4 , 0 ,
		5 , 1 , 8 , 0 ,
		9 , 5 , 13 , 1 ,
		5 , 12 , 4 , 8 ,
		12 , 5 , 13 , 8 ,
		9 , 5 , 1 , 8 ,
		5 , 9 , 13 , 8 ,
		13 , 5 , 14 , 15 ,
		7 , 5 , 13 , 15 ,
		5 , 12 , 13 , 14 ,
		5 , 7 , 14 , 15 ,
		6 , 5 , 7 , 14 ,
		5 , 12 , 14 , 4 ,
		6 , 5 , 14 , 4 ,
		5 , 6 , 7 , 4 ,
		3 , 2 , 8 , 0 ,
		1 , 3 , 8 , 0 ,
		10 , 3 , 2 , 8 ,
		3 , 10 , 11 , 8 ,
		3 , 9 , 1 , 8 ,
		9 , 3 , 11 , 8 ,
		2 , 3 , 4 , 0 ,
		3 , 1 , 4 , 0 ,
		3 , 6 , 2 , 4 ,
		6 , 3 , 7 , 4 ,
		5 , 3 , 1 , 4 ,
		3 , 5 , 7 , 4 ,
		3 , 11 , 14 , 15 ,
		7 , 3 , 14 , 15 ,
		10 , 3 , 11 , 14 ,
		3 , 6 , 7 , 14 ,
		10 , 3 , 14 , 2 ,
		3 , 6 , 14 , 2 ,
		11 , 3 , 13 , 15 ,
		3 , 7 , 13 , 15 ,
		3 , 9 , 11 , 13 ,
		5 , 3 , 7 , 13 ,
		3 , 9 , 13 , 1 ,
		5 , 3 , 13 , 1
	};

	glGenBuffers(3, &vbo[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(float), hypercube, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 64 * sizeof(float), colors, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 58 * 4 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void drawModel()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

	glBindVertexArray(vao);
	glDrawElements(GL_LINES_ADJACENCY, 58 * 4, GL_UNSIGNED_INT, nullptr);
}

double last_time;
int frames;

void showFPS(GLFWwindow *window)
{
	double current_time = glfwGetTime();
	double delta = current_time - last_time;

	frames++;
	if (delta >= 1.0)
	{
		auto fps = frames / delta;

		std::stringstream ss;
		ss << "G4D Demo [" << fps << " FPS]";

		glfwSetWindowTitle(window, ss.str().c_str());

		frames = 0;
		last_time = current_time;
	}
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "G4D Demo", nullptr, nullptr);
	glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

	initProgram();
	initModel();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setUniforms();

		program->bind();

		drawModel();

		program->release();

        glfwSwapBuffers(window);
		showFPS(window);
    }

    glfwTerminate();

    return 0;
}
