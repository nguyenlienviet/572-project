/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include <time.h>
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
#include <random>
#include <math.h>
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;


#define CLOTHRESXY 200
#define OBJECT_SIZE 4
#define NUM_OBJECTS 4

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = rot = glm::vec3(0, 0, -15);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -3*ftime;
		else if(d==1)
			yangle = 3*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;


float randf()
	{	
	return (float)(rand() / (float)RAND_MAX);
	}

class Application : public EventCallbacks
{

public:



	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	GLuint VAO;
	GLuint VBOposvel;

	GLuint object_VAO;
	GLuint object_VBOposvel;

	GLuint object_VAOs[NUM_OBJECTS];
	GLuint object_VBOposvels[NUM_OBJECTS];

	GLuint  IndexBuffer;
	GLuint  ObjectIndexBuffer;
	GLuint  ObjectIndexBuffers[NUM_OBJECTS];

	//texture data
	GLuint Texture;
	//compute stuff
	
	GLuint computeProgram;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
	}

	bool mousepressed = false;
	double prevX, prevY;
	double posX, posY;
	double dx = 0, dy = 0;
	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		if (action == GLFW_PRESS)
		{
			std::cout << "press" << std::endl;
			mousepressed = true;
			glfwGetCursorPos(window, &posX, &posY);
		}
		if (action == GLFW_RELEASE)
		{
			std::cout << "release" << std::endl;
			mousepressed = false;
			dx = 0;
			dy = 0;
		}

	}
	
	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

void init_mesh()
	{

		//generate the VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBOposvel);
		glBindBuffer(GL_ARRAY_BUFFER, VBOposvel);

		vec4 vert[CLOTHRESXY * 3];
		for (int i = 0; i < CLOTHRESXY * 3; i++)
			vert[i] = vec4(0);

		int startx = -CLOTHRESXY / 2 / 10;
		for (int x = 0; x < CLOTHRESXY; x++)
			{
			vert[x] = vec4(startx + float(x) / 10, 0, 0, 1);			
			}

		//vert[CLOTHRESXY / 2 - 1].y = .5;
		//vert[CLOTHRESXY / 2].y = -5;
		//vert[CLOTHRESXY / 2 + 1].y = .5;

		glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * CLOTHRESXY * 3, vert, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VBOposvel);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	
		glGenBuffers(1, &IndexBuffer);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
		unsigned int elements[CLOTHRESXY];
		int ind = 0;
		for (int x = 0; x < CLOTHRESXY; x++)
			{
			elements[ind++] = x;
			}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * ind, elements, GL_STATIC_DRAW);
		glBindVertexArray(0);

		for (int i = 0; i < NUM_OBJECTS; i++)
		{
			glGenVertexArrays(1, &object_VAOs[i]);
			glBindVertexArray(object_VAOs[i]);
			glGenBuffers(1, &object_VBOposvels[i]);
			glBindBuffer(GL_ARRAY_BUFFER, object_VBOposvels[i]);

			vec4 object_vert[OBJECT_SIZE];

			object_vert[0] = vec4(i*.5, 1+i*1.1, 0, 1);
			object_vert[1] = vec4(i*.5+1, 1+i*1.1, 0, 1);
			object_vert[2] = vec4(i*.5+1, 2+i*1.1, 0, 1);
			object_vert[3] = vec4(i*.5, 2+i*1.1, 0, 1);

			glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * OBJECT_SIZE, object_vert, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, object_VBOposvels[i]);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glGenBuffers(1, &ObjectIndexBuffers[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ObjectIndexBuffers[i]);
			unsigned int objectElements[4];
			int objInd = 0;
			for (int x = 0; x < 4; x++)
			{
				objectElements[objInd++] = x;
			}
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * objInd, objectElements, GL_STATIC_DRAW);
			glBindVertexArray(0);
		}
	}
	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		//initialize the net mesh
		init_mesh();

		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		
		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/grid.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
	
		prog->addAttribute("vertPos");


		

		//GPU stuff
		std::string ShaderString = readFileAsString("../resources/compute.glsl");
		const char* shader = ShaderString.c_str();
		GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(computeShader, 1, &shader, nullptr);

		GLint rc;
		CHECKED_GL_CALL(glCompileShader(computeShader));
		CHECKED_GL_CALL(glGetShaderiv(computeShader, GL_COMPILE_STATUS, &rc));
		if (!rc)	//error compiling the shader file
		{
			GLSL::printShaderInfoLog(computeShader);
			std::cout << "Error compiling fragment shader " << std::endl;
			exit(1);
		}

		computeProgram = glCreateProgram();
		glAttachShader(computeProgram, computeShader);
		glLinkProgram(computeProgram);
		glUseProgram(computeProgram);

		GLuint block_index;
		block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "VerticesVelocities");
		GLuint ssbo_binding_point_index = 0;
		glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);
	}


	void update(float dt)
		{		
		compute();
		}

	void compute()
	{

		//print data before compute shader
	
		if (mousepressed)
		{
			std::cout << "hold" << std::endl;
			prevX = posX;
			prevY = posY;
			glfwGetCursorPos(windowManager->getHandle(), &posX, &posY);
			dx = (posX - prevX) * 0.01;
			dy = (posY - prevY) * 0.01;
			std::cout << dx << std::endl;
		}

		GLuint block_index = 0;
		block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, "VerticesVelocities");
		GLuint ssbo_binding_point_index = 0;
		glShaderStorageBlockBinding(computeProgram, block_index, ssbo_binding_point_index);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, VBOposvel);

		char* names[] = { "ObjectPos1", "ObjectPos2", "ObjectPos3", "ObjectPos4" };

		for (int i = 0; i < NUM_OBJECTS; i++)
		{
			GLuint object_block_index = 0;
			object_block_index = glGetProgramResourceIndex(computeProgram, GL_SHADER_STORAGE_BLOCK, names[i]);
			GLuint object_ssbo_binding_point_index = i+1;
			glShaderStorageBlockBinding(computeProgram, object_block_index, object_ssbo_binding_point_index);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i+1, object_VBOposvels[i]);
		}

		glUseProgram(computeProgram);

		GLint loc = glGetUniformLocation(computeProgram, "dx");
		glUniform1f(loc, dx);
		loc = glGetUniformLocation(computeProgram, "dy");
		glUniform1f(loc, -dy);

		glDispatchCompute((GLuint)CLOTHRESXY, 1, 1);				//start compute shader
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		double frametime = get_last_elapsed_time();
		update(frametime);

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
				
		// Draw the box using GLSL.
		prog->bind();

		V = mycam.process(frametime);
		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VAO);
		glDrawElements(GL_LINE_STRIP, CLOTHRESXY, GL_UNSIGNED_INT, 0);

		for (int i = 0; i < NUM_OBJECTS; i++)
		{
			glBindVertexArray(object_VAOs[i]);
			glDrawElements(GL_LINE_LOOP, OBJECT_SIZE, GL_UNSIGNED_INT, 0);
		}
		prog->unbind();

	}
};




//******************************************************************************************
int main(int argc, char **argv)
{


	srand(time(0));

	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
