#include "Window.h"

const char* modelPath = "Spencer mansion hall.obj";
const char* window_title = "Assignment 4";
GLint shaderProgramPhong, shaderProgramSkybox, shaderProgramSSAO, shaderProgramDOF;

// Default camera parameters
glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;

bool enableDOF;
bool keyHeld[1024];

GLuint FBO, FBO2, FBO3, FBO4;
GLuint texColor, texDepth, texNormal, texColor2, texColor3, texColor4;
GLuint VBO, VAO, EBO;
std::vector<GLfloat> Window::vertices;

int Window::time = 0;

glm::mat4 Window::P;
glm::mat4 Window::V;

glm::mat4 toWorld;

float planeInFocus = 1.0f;
float yaw = -3.14f * 0.5f;
float pitch = 0.0f;

OBJObject* skybox;
Model* scene;

Transform* sceneGraphRoot;
Transform* armyTransform;

unsigned int cubeTextureID;


/** Load a ppm file from disk.
@input filename The location of the PPM file.  If the file is not found, an error message
will be printed and this function will return 0
@input width This will be modified to contain the width of the loaded image, or 0 if file not found
@input height This will be modified to contain the height of the loaded image, or 0 if file not found
@return Returns the RGB pixel data as interleaved unsigned chars (R0 G0 B0 R1 G1 B1 R2 G2 B2 .... etc) or 0 if an error ocured
**/
unsigned char* loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;
}

void Window::initialize_objects()
{

	skybox = new OBJObject(NULL, 0.0f, 0.0f, 0.0f, 0.3f, glm::vec3(0.0), glm::vec3(0.0), 1.0f);
	scene = new Model(modelPath);
	//dragon = new OBJObject("dragon.obj", 0.0f, 0.0f, 0.0f, 10.0f, glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.0), 16.0f);
	//dragon2 = new OBJObject("dragon.obj", 0.0f, 00.0f, 50.0f, 10.0f, glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.0), 16.0f);
	//dragon3 = new OBJObject("dragon.obj", 20.0f, 0.0f, 0.0f, 10.0f, glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.0), 16.0f);

	shaderProgramPhong = LoadShaders("phong.vert", "phong.frag");
	shaderProgramSkybox = LoadShaders("skybox.vert", "skybox.frag");
	shaderProgramSSAO = LoadShaders("SSAO.vert", "SSAO.frag");
	shaderProgramDOF = LoadShaders("DOF.vert", "DOF.frag");

	int twidth, theight;   // texture width/height [pixels]
	unsigned char* tdata;  // texture pixel data

	enableDOF = false;

	// Cube texture
	glGenTextures(1, &cubeTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureID);

	char* cubeFiles[] = {"right.ppm", "left.ppm", "top.ppm", "bottom.ppm" , "front.ppm" , "back.ppm" };

	for (GLuint i = 0; i < 6; i++)
	{
		// Load image file
		tdata = loadPPM(cubeFiles[i], twidth, theight);
		if (tdata == NULL) return;

		// Generate the texture
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 3, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	vertices.push_back(-1.0f); vertices.push_back(1.0f);
	vertices.push_back(-1.0f); vertices.push_back(-1.0f);
	vertices.push_back(1.0f); vertices.push_back(1.0f);
	vertices.push_back(1.0f); vertices.push_back(-1.0f);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		2, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		0, // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);

	// CREATE FBO 1

	// Creates a framebuffer object.
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Creates a color texture and binds it to the framebuffer.
	glGenTextures(1, &texColor);
	glBindTexture(GL_TEXTURE_2D, texColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);

	// Creates a normal texture and binds it to the same framebuffer.
	glGenTextures(1, &texNormal);
	glBindTexture(GL_TEXTURE_2D, texNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texNormal, 0);

	// Creates a depth texture and binds it to the same framebuffer.
	glGenTextures(1, &texDepth);
	glBindTexture(GL_TEXTURE_2D, texDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0);

	// CREATE FBO_2

	// Creates a framebuffer object.
	glGenFramebuffers(1, &FBO2);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO2);

	// Creates a color texture and binds it to the framebuffer.
	glGenTextures(1, &texColor2);
	glBindTexture(GL_TEXTURE_2D, texColor2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor2, 0);

	// FBO 3
	glGenFramebuffers(1, &FBO3);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO3);

	// Creates a color texture and binds it to the framebuffer.
	glGenTextures(1, &texColor3);
	glBindTexture(GL_TEXTURE_2D, texColor3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor3, 0);

	// FBO 4

	glGenFramebuffers(1, &FBO4);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO4);

	// Creates a color texture and binds it to the framebuffer.
	glGenTextures(1, &texColor4);
	glBindTexture(GL_TEXTURE_2D, texColor4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor4, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer is incomplete.\n");
	}
	else
	{
		printf("Framebuffer is complete.\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_GREATER, 0.5f)
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
	delete(skybox);
	glDeleteProgram(shaderProgramPhong);
	glDeleteProgram(shaderProgramSkybox);
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1030.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}

	glfwSetCursorPos(window, width / 2, height / 2);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	for (int i = 0; i < 1024; ++i)
	{
		keyHeld[i] = false;
	}
}

void Window::idle_callback()
{
	time++;

	float speed = 0.1f;

	if (keyHeld[GLFW_KEY_W])
	{
		cam_pos.x += glm::cos(yaw) * speed;
		cam_pos.z += glm::sin(yaw) * speed;
	}

	if (keyHeld[GLFW_KEY_A])
	{
		cam_pos.x -= glm::cos(yaw + 3.14 * 0.5) * speed;
		cam_pos.z -= glm::sin(yaw + 3.14 * 0.5) * speed;
	}

	if (keyHeld[GLFW_KEY_S])
	{
		cam_pos.x -= glm::cos(yaw) * speed;
		cam_pos.z -= glm::sin(yaw) * speed;
	}

	if (keyHeld[GLFW_KEY_D])
	{
		cam_pos.x += glm::cos(yaw + 3.14 * 0.5) * speed;
		cam_pos.z += glm::sin(yaw + 3.14 * 0.5) * speed;
	}

	if (keyHeld[GLFW_KEY_SPACE])
	{
		cam_pos.y += speed;
	}

	if (keyHeld[GLFW_KEY_LEFT_SHIFT])
	{
		cam_pos.y -= speed;
	}
}

bool print = false;

void Window::display_callback(GLFWwindow* window)
{
	cam_look_at = cam_pos + glm::vec3(glm::cos(yaw) * glm::cos(pitch), glm::sin(pitch), glm::sin(yaw) * glm::cos(pitch));
	V = glm::lookAt(cam_pos, cam_look_at, cam_up);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	
		// Tells OpenGL what buffers we want to draw to.
		GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, buffers);

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgramSkybox);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTextureID);
		skybox->draw(V);

		glUseProgram(shaderProgramPhong);


		glUniform1i(glGetUniformLocation(shaderProgramPhong, "directionalLight.enabled"), false);
		glUniform1i(glGetUniformLocation(shaderProgramPhong, "pointLight.enabled"), false);
		glUniform1i(glGetUniformLocation(shaderProgramPhong, "spotLight.enabled"), false);

		GLuint uCameraPosition = glGetUniformLocation(shaderProgramPhong, "cameraPosition");
		glUniform3f(uCameraPosition, cam_pos.x, cam_pos.y, cam_pos.z);

		GLuint uProjection = glGetUniformLocation(shaderProgramPhong, "projection");
		GLuint uView = glGetUniformLocation(shaderProgramPhong, "view");
		glUniformMatrix4fv(uProjection, 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(uView, 1, GL_FALSE, &V[0][0]);

		scene->draw(shaderProgramPhong);
		//dragon->draw(V);
		//dragon2->draw(glm::translate(V, glm::vec3(0.0f, 0.0f, 5.0f)));

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
/*
	// SSAO effect ----------------------------------------------------------------------

	glUseProgram(shaderProgramSSAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "texColor"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texNormal);
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "texNormal"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texDepth);
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "texDepth"), 2);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO2);

		// Tells OpenGL what buffers we want to draw to.
		GLenum buffers2[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers2);

		glUniform1i(glGetUniformLocation(shaderProgramSSAO, "debugMode"), 0);
		toWorld = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramSSAO, "modelMatrix"), 1, GL_FALSE, &toWorld[0][0]);
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
*/
	// DOF effect ----------------------------------------------------------------------

	glUseProgram(shaderProgramDOF);

	glUniform1i(glGetUniformLocation(shaderProgramDOF, "enabled"), enableDOF);

	//horizontal blur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glUniform1i(glGetUniformLocation(shaderProgramDOF, "texColor"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texDepth);
	glUniform1i(glGetUniformLocation(shaderProgramDOF, "texDepth"), 1);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO3);

		// Renders DOF result
		glUniform1f(glGetUniformLocation(shaderProgramDOF, "textureWidth"), width);
		glUniform1f(glGetUniformLocation(shaderProgramDOF, "textureHeight"), height);
		glUniform1f(glGetUniformLocation(shaderProgramDOF, "aperture"), 10.0);
		glUniform1f(glGetUniformLocation(shaderProgramDOF, "plane"), planeInFocus);
		glUniform1f(glGetUniformLocation(shaderProgramDOF, "near"), 0.1f);
		glUniform1f(glGetUniformLocation(shaderProgramDOF, "far"), 1030.0f);
		glUniform1i(glGetUniformLocation(shaderProgramDOF, "horizontal"), 1);


		GLenum buffers3[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers3);

		toWorld = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramDOF, "modelMatrix"), 1, GL_FALSE, &toWorld[0][0]);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);


	//vertcal blur

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColor3);
	glUniform1i(glGetUniformLocation(shaderProgramDOF, "texColor"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texDepth);
	glUniform1i(glGetUniformLocation(shaderProgramDOF, "texDepth"), 1);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO4);


	glUniform1i(glGetUniformLocation(shaderProgramDOF, "horizontal"), 0);

	glDrawBuffers(1, buffers3);

	toWorld = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramDOF, "modelMatrix"), 1, GL_FALSE, &toWorld[0][0]);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Debug info ----------------------------------------------------------------------

	glUseProgram(shaderProgramSSAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texColor4);
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "texColor"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texNormal);
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "texNormal"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texDepth);
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "texDepth"), 2);

	float scale = 0.2f;

	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "debugMode"), 0);
	toWorld = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramSSAO, "modelMatrix"), 1, GL_FALSE, &toWorld[0][0]);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	// Renders normals
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "debugMode"), 1);
	toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f - scale, 1.0f - scale, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramSSAO, "modelMatrix"), 1, GL_FALSE, &toWorld[0][0]);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	// Renders depth
	glUniform1i(glGetUniformLocation(shaderProgramSSAO, "debugMode"), 2);
	toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f - scale, 1.0f - scale - scale * 2.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramSSAO, "modelMatrix"), 1, GL_FALSE, &toWorld[0][0]);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glEnable(GL_DEPTH_TEST);

	if (glGetError() != GL_NO_ERROR)
	{
		//printf("Error\n");
	}

	if (print)
	{
		print = false;
		float* data = new float[width * height];

		/*data[5] = 50.0f;
		for (int y = 0; y < 10; y++)
		{
			printf("Row %d:\n", y);
			for (int x = 0; x < width; x++)
			{
				printf("%f, ", data[y * width + x]);
			}
			printf("\n");
		}*/

		glBindTexture(GL_TEXTURE_2D, texDepth);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid*)data);
		glBindTexture(GL_TEXTURE_2D, 0);

		printf("Started reading depth texture.\n");
		
		for (int y = 0; y < height; y++)
		{
			//printf("Row %d:\n", y);
			for (int x = 0; x < width; x++)
			{
				if (data[y * width + x] != 1.0f)
				{
					printf("%f\n", data[y * width + x]);
				}
			}
		}

		printf("Done reading depth texture.\n");
	}

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		keyHeld[key] = true;

		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_P) {
			if (mods&GLFW_MOD_CONTROL) {
				if (planeInFocus < 1000.0)
					planeInFocus++;
			}
			else {
				if (planeInFocus > 1.0) 
					planeInFocus--;
			}
		}

		if (key == GLFW_KEY_F3) {
			enableDOF = !enableDOF;
		}
	}

	if (action == GLFW_RELEASE)
	{
		keyHeld[key] = false;
	}
}

void Window::cursorPos_callback(GLFWwindow* window, double xpos, double ypos)
{
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	
	float sensitivity = 0.003f;
	yaw += (cursorX - width / 2) * sensitivity;
	pitch = glm::clamp(pitch + (float)(height / 2 - cursorY) * sensitivity, -3.14f * 0.5f, 3.14f * 0.5f);

	glfwSetCursorPos(window, width / 2, height / 2);
}

void Window::mouseButton_callback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		break;

	case GLFW_MOUSE_BUTTON_RIGHT:
		break;
	}
}

void Window::scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
}