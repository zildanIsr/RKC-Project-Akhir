#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	BuildShaders();
	BuildDepthMap();
	BuildTexturedCube();
	BuildTexturedPlane();
	InitCamera();

}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);
	glDeleteBuffers(1, &depthMapFBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// zoom camera
	// -----------
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (fovy < 90) {
			fovy += 0.0001f;
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (fovy > 0) {
			fovy -= 0.0001f;
		}
	}

	// update camera movement 
	// -------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		MoveCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		MoveCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		StrafeCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		StrafeCamera(CAMERA_SPEED);
	}

	// update camera rotation
	// ----------------------
	double mouseX, mouseY;
	double midX = screenWidth / 2;
	double midY = screenHeight / 2;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	// Get mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);
	if ((mouseX == midX) && (mouseY == midY)) {
		return;
	}

	// Set mouse position
	glfwSetCursorPos(window, midX, midY);

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angleY = (float)((midX - mouseX)) / 500;
	angleZ = (float)((midY - mouseY)) / 500;

	// The higher the value is the faster the camera looks around.
	viewCamY += angleZ * 2;

	// limit the rotation around the x-axis
	if ((viewCamY - posCamY) > 8) {
		viewCamY = posCamY + 8;
	}
	if ((viewCamY - posCamY) < -8) {
		viewCamY = posCamY - 8;
	}
	RotateCamera(-angleY);
}

void Demo::Update(double deltaTime) {
	angle += (float)((deltaTime * 1.5f) / 1500);
}

void Demo::Render() {
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	
	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(10.0f, -10.0f, -10.0f, 5.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(0.05f, 2.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawTexturedCube(this->depthmapShader);
	DrawTexturedPlane(this->depthmapShader);
	DrawTexturedChair(this->depthmapShader);
	DrawTexturedSideWhiteboard(this->depthmapShader);
	DrawTexturedWhiteboard(this->depthmapShader);
	DrawTexturedTableDosen(this->depthmapShader);
	DrawTexturedWall(this->depthmapShader);
	DrawTexturedDoor(this->depthmapShader);
	DrawTextureChairDosen(this->depthmapShader);
	DrawTextureChairDosenAtas(this->depthmapShader);
	DrawTextureProjector(this->depthmapShader);
	DrawTexturedScreen(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	

	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(fovy, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	// LookAt camera (position, target/direction, up)
	glm::mat4 view = glm::lookAt(glm::vec3(posCamX, posCamY, posCamZ), glm::vec3(viewCamX, viewCamY, viewCamZ), glm::vec3(upCamX, upCamY, upCamZ));
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	
	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), posCamX, posCamY, posCamZ);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	// Render floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedPlane(this->shadowmapShader);
	
	// Render cube
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedCube(this->shadowmapShader);

	//Render chair
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture4);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedChair(this->shadowmapShader);

	//Render side whiteboard
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture5);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedSideWhiteboard(this->shadowmapShader);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedWhiteboard(this->shadowmapShader);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture11);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedTableDosen(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture6);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedWall(this->shadowmapShader);

	//door
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture12);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedDoor(this->shadowmapShader);

	//kursiDosen
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture11);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTextureChairDosen(this->shadowmapShader);

	//kursiDosen atas
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture10);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTextureChairDosenAtas(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture11);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTextureProjector(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture13);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedScreen(this->shadowmapShader);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}

void Demo::BuildTexturedCube()
{

	int width, height;
	unsigned char* image;
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &cube_texture);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("crate.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("whiteboard.jpeg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("table.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture4);
	glBindTexture(GL_TEXTURE_2D, texture4);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("chair.jpeg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture5);
	glBindTexture(GL_TEXTURE_2D, texture5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("black_tex.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture6);
	glBindTexture(GL_TEXTURE_2D, texture6);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("wall.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture7);
	glBindTexture(GL_TEXTURE_2D, texture7);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("white.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture8);
	glBindTexture(GL_TEXTURE_2D, texture8);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("camera.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture9);
	glBindTexture(GL_TEXTURE_2D, texture9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("tirai.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture10);
	glBindTexture(GL_TEXTURE_2D, texture10);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("cloth.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture11);
	glBindTexture(GL_TEXTURE_2D, texture11);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("black_wood.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture12);
	glBindTexture(GL_TEXTURE_2D, texture12);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("glass-door.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &texture13);
	glBindTexture(GL_TEXTURE_2D, texture13);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	width, height;
	image = SOIL_load_image("lesson.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords, normal
		// front
		-1.0, -1.0, 1.0, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		1.0, -1.0, 1.0, 1, 0,  0.0f,  0.0f,  1.0f, // 1
		1.0,  1.0, 1.0, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		-1.0,  1.0, 1.0, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		 // right
		 1.0,  1.0,  1.0, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		 1.0,  1.0, -1.0, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		 1.0, -1.0, -1.0, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		 1.0, -1.0,  1.0, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// back
		-1.0, -1.0, -1.0, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		1.0,  -1.0, -1.0, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		1.0,   1.0, -1.0, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		-1.0,  1.0, -1.0, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		 // left
		 -1.0, -1.0, -1.0, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		 -1.0, -1.0,  1.0, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		 -1.0,  1.0,  1.0, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		 -1.0,  1.0, -1.0, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		// upper
		1.0, 1.0,  1.0, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		-1.0, 1.0, 1.0, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		-1.0, 1.0, -1.0, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		1.0, 1.0, -1.0, 0, 1,   0.0f,  1.0f,  0.0f, // 19

		// bottom
		-1.0, -1.0, -1.0, 0, 0, 0.0f,  -1.0f,  0.0f, // 20
		1.0, -1.0, -1.0, 1, 0,  0.0f,  -1.0f,  0.0f, // 21
		1.0, -1.0,  1.0, 1, 1,  0.0f,  -1.0f,  0.0f, // 22
		-1.0, -1.0,  1.0, 0, 1, 0.0f,  -1.0f,  0.0f, // 23
	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22   // bottom
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::BuildTexturedPlane()
{
	// Load and create a texture 
	glGenTextures(1, &plane_texture);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("marmer_grey.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-25.0f,	-0.5f, -25.0f,  0,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f, -25.0f, 25,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f,  25.0f, 25, 25, 0.0f,  1.0f,  0.0f,
		-25.0f,	-0.5f,  25.0f,  0, 25, 0.0f,  1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawTexturedCube(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	int t = -6;
	int tz = 0;
	for (int i = 0; i < 18; i++) {

		glm::mat4 model;
		model = glm::translate(model, glm::vec3(t, 0, tz));
		model = glm::translate(model, glm::vec3(0.65, 0, -0.25));

		model = glm::rotate(model, 0.0f, glm::vec3(0, 0, 1));

		model = glm::scale(model, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model2;
		model2 = glm::translate(model2, glm::vec3(t, 0, tz));
		model2 = glm::translate(model2, glm::vec3(-0.875, 0, -0.25));

		model2 = glm::rotate(model2, 0.0f, glm::vec3(0, 0, 1));

		model2 = glm::scale(model2, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model3;
		model3 = glm::translate(model3, glm::vec3(t, 0, tz));
		model3 = glm::translate(model3, glm::vec3(-0.875, 0, 0.75));

		model3 = glm::rotate(model3, 0.0f, glm::vec3(0, 0, 1));

		model3 = glm::scale(model3, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model4;
		model4 = glm::translate(model4, glm::vec3(t, 0, tz));
		model4 = glm::translate(model4, glm::vec3(0.65, 0, 0.75));

		model4 = glm::rotate(model4, 0.0f, glm::vec3(0, 0, 1));

		model4 = glm::scale(model4, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model5;
		model5 = glm::translate(model5, glm::vec3(t, 0, tz));
		model5 = glm::translate(model5, glm::vec3(-0.115, 0.55, 0.25));

		model5 = glm::rotate(model5, 0.0f, glm::vec3(0, 0, 1));

		model5 = glm::scale(model5, glm::vec3(0.875, 0.1, 0.6));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model5));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		
		if (i == 5 || i == 11) {
			tz += 3;
			t = (-8);
		}

		if (i == 2 || i == 8 || i == 14) {
			t += 4;
		}
		else {
			t += 2;
		}


	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedChair(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	int t = -6;
	int tz = 0;
	for (int i = 0; i < 18; i++) {
		glm::mat4 model6;
		model6 = glm::translate(model6, glm::vec3(t, 0, tz + 0.5));
		model6 = glm::translate(model6, glm::vec3(0.325, -0.2, 0.55));

		model6 = glm::rotate(model6, 0.0f, glm::vec3(0, 0, 1));

		model6 = glm::scale(model6, glm::vec3(0.1, 0.3, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model7;
		model7 = glm::translate(model7, glm::vec3(t, 0, tz + 0.5));
		model7 = glm::translate(model7, glm::vec3(-0.425, -0.2, 0.55));

		model7 = glm::rotate(model7, 0.0f, glm::vec3(0, 0, 1));

		model7 = glm::scale(model7, glm::vec3(0.1, 0.3, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model7));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model8;
		model8 = glm::translate(model8, glm::vec3(t, 0, tz + 0.5));
		model8 = glm::translate(model8, glm::vec3(-0.425, 0, 1.15));

		model8 = glm::rotate(model8, 0.0f, glm::vec3(0, 0, 1));

		model8 = glm::scale(model8, glm::vec3(0.1, 0.825, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model8));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model9;
		model9 = glm::translate(model9, glm::vec3(t, 0, tz + 0.5));
		model9 = glm::translate(model9, glm::vec3(0.325, 0, 1.15));

		model9 = glm::rotate(model9, 0.0f, glm::vec3(0, 0, 1));

		model9 = glm::scale(model9, glm::vec3(0.1, 0.825, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model9));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model10;
		model10 = glm::translate(model10, glm::vec3(t, 0, tz + 0.5));
		model10 = glm::translate(model10, glm::vec3(-0.05, 0.1, 0.85));

		model10 = glm::rotate(model10, 0.0f, glm::vec3(0, 0, 1));

		model10 = glm::scale(model10, glm::vec3(0.475, 0.075, 0.4));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model10));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model11;
		model11 = glm::translate(model11, glm::vec3(t, 0.475, tz + 0.5));
		model11 = glm::translate(model11, glm::vec3(-0.05, 0.2, 1.1));

		model11 = glm::rotate(model11, 0.0f, glm::vec3(0, 0, 1));

		model11 = glm::scale(model11, glm::vec3(0.475, 0.25, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model11));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


		if (i == 5 || i == 11) {
			tz += 3;
			t = (-8);
		}

		if (i == 2 || i == 8 || i == 14) {
			t += 4;
		}
		else {
			t += 2;
		}


	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedWhiteboard(GLuint shader) {

	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelPt;
	modelPt = glm::translate(modelPt, glm::vec3(-0.45 + 0.3, 1.275, -2.25 - 0.6));

	modelPt = glm::rotate(modelPt, glm::radians(180.0f), glm::vec3(1, 0, 0));

	modelPt = glm::scale(modelPt, glm::vec3(2, 1, 0.05));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedSideWhiteboard(GLuint shader) {

	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelPt2;
	modelPt2 = glm::translate(modelPt2, glm::vec3(-2.575 + 0.3, 0.425, -2.25 - 0.6));

	modelPt2 = glm::rotate(modelPt2, 0.0f, glm::vec3(0, 0, 1));

	modelPt2 = glm::scale(modelPt2, glm::vec3(0.05, 1.4, 0.075));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


	glm::mat4 modelPt3;
	modelPt3 = glm::translate(modelPt3, glm::vec3(1.65 + 0.3, 0.425, -2.25 - 0.6));

	modelPt3 = glm::rotate(modelPt3, 0.0f, glm::vec3(0, 0, 1));

	modelPt3 = glm::scale(modelPt3, glm::vec3(0.05, 1.4, 0.075));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt3));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelPt4;
	modelPt4 = glm::translate(modelPt4, glm::vec3(1.65 + 0.3, -0.45, -2.25 - 0.6));

	modelPt4 = glm::rotate(modelPt4, 0.0f, glm::vec3(0, 0, 1));

	modelPt4 = glm::scale(modelPt4, glm::vec3(0.15, 0.05, 0.5));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt4));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelPt5;
	modelPt5 = glm::translate(modelPt5, glm::vec3(-2.575 + 0.3, -0.45, -2.25 - 0.6));

	modelPt5 = glm::rotate(modelPt5, 0.0f, glm::vec3(0, 0, 1));

	modelPt5 = glm::scale(modelPt5, glm::vec3(0.15, 0.05, 0.5));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt5));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelPt6;
	modelPt6 = glm::translate(modelPt6, glm::vec3(-2.475 + 0.3, 1.375, -2.25 - 0.6));

	modelPt6 = glm::rotate(modelPt6, 0.0f, glm::vec3(0, 0, 1));

	modelPt6 = glm::scale(modelPt6, glm::vec3(0.1, 0.05, 0.05));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt6));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelPt7;
	modelPt7 = glm::translate(modelPt7, glm::vec3(1.6 + 0.3, 1.375, -2.25 - 0.6));

	modelPt7 = glm::rotate(modelPt7, 0.0f, glm::vec3(0, 0, 1));

	modelPt7 = glm::scale(modelPt7, glm::vec3(0.1, 0.05, 0.05));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPt7));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedTableDosen(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	int t = -6;
	int tz = -3;
	
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(t, 0, tz));
		model = glm::translate(model, glm::vec3(0.65, 0, -0.25));

		model = glm::rotate(model, 0.0f, glm::vec3(0, 0, 1));

		model = glm::scale(model, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model2;
		model2 = glm::translate(model2, glm::vec3(t, 0, tz));
		model2 = glm::translate(model2, glm::vec3(-0.875, 0, -0.25));

		model2 = glm::rotate(model2, 0.0f, glm::vec3(0, 0, 1));

		model2 = glm::scale(model2, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model3;
		model3 = glm::translate(model3, glm::vec3(t, 0, tz));
		model3 = glm::translate(model3, glm::vec3(-0.875, 0, 0.75));

		model3 = glm::rotate(model3, 0.0f, glm::vec3(0, 0, 1));

		model3 = glm::scale(model3, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model4;
		model4 = glm::translate(model4, glm::vec3(t, 0, tz));
		model4 = glm::translate(model4, glm::vec3(0.65, 0, 0.75));

		model4 = glm::rotate(model4, 0.0f, glm::vec3(0, 0, 1));

		model4 = glm::scale(model4, glm::vec3(0.1, 0.5, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model5;
		model5 = glm::translate(model5, glm::vec3(t, 0, tz));
		model5 = glm::translate(model5, glm::vec3(-0.115, 0.55, 0.25));

		model5 = glm::rotate(model5, 0.0f, glm::vec3(0, 0, 1));

		model5 = glm::scale(model5, glm::vec3(0.875, 0.1, 0.6));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model5));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model6;
		model6 = glm::translate(model6, glm::vec3(t, 0, tz));
		model6 = glm::translate(model6, glm::vec3(0.65, 0.2, 0.25));

		model6 = glm::rotate(model6, 0.0f, glm::vec3(0, 0, 1));

		model6 = glm::scale(model6, glm::vec3(0.1, 0.35, 0.6));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model7;
		model7 = glm::translate(model7, glm::vec3(t, 0, tz));
		model7 = glm::translate(model7, glm::vec3(-0.875, 0.2, 0.25));

		model7 = glm::rotate(model7, 0.0f, glm::vec3(0, 0, 1));

		model7 = glm::scale(model7, glm::vec3(0.1, 0.35, 0.6));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model7));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glm::mat4 model8;
		model8 = glm::translate(model8, glm::vec3(t, 0, tz));
		model8 = glm::translate(model8, glm::vec3(-0.115, 0.2, 0.75));

		model8 = glm::rotate(model8, 0.0f, glm::vec3(0, 0, 1));

		model8 = glm::scale(model8, glm::vec3(0.875, 0.35, 0.1));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model8));

		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedWall(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelWall;
	modelWall = glm::translate(modelWall, glm::vec3(0, 0, -7));

	modelWall = glm::rotate(modelWall, 0.0f, glm::vec3(0, 0, 1));

	modelWall = glm::scale(modelWall, glm::vec3(9, 5, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWall));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelWall2;
	modelWall2 = glm::translate(modelWall2, glm::vec3(-9, 0, 3));

	modelWall2 = glm::rotate(modelWall2, 0.0f, glm::vec3(0, 0, 1));

	modelWall2 = glm::scale(modelWall2, glm::vec3(0.1, 5, 10));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWall2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelWall3;
	modelWall3 = glm::translate(modelWall3, glm::vec3(9, 0, 3));

	modelWall3 = glm::rotate(modelWall3, 0.0f, glm::vec3(0, 0, 1));

	modelWall3 = glm::scale(modelWall3, glm::vec3(0.1, 5, 10));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWall3));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelWall5;
	modelWall5 = glm::translate(modelWall5, glm::vec3(0, 0, 13));

	modelWall5 = glm::rotate(modelWall5, 0.0f, glm::vec3(0, 0, 1));

	modelWall5 = glm::scale(modelWall5, glm::vec3(9, 5, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWall5));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelRoof;
	modelRoof = glm::translate(modelRoof, glm::vec3(0, 5, 3));

	modelRoof = glm::rotate(modelRoof, 0.0f, glm::vec3(0, 0, 1));

	modelRoof = glm::scale(modelRoof, glm::vec3(9, 0.1, 10));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelRoof));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, texture8);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedDoor(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelDoor;
	modelDoor = glm::translate(modelDoor, glm::vec3(8.9, 1, -3));

	modelDoor = glm::rotate(modelDoor, 0.0f, glm::vec3(0, 0, 1));

	modelDoor = glm::scale(modelDoor, glm::vec3(0.05, 2, 2));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelDoor));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTextureChairDosen(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelCD1;
	modelCD1 = glm::translate(modelCD1, glm::vec3(-6, 0, -6));
	modelCD1 = glm::translate(modelCD1, glm::vec3(0, -0.1, 1.7));

	modelCD1 = glm::rotate(modelCD1, 0.0f, glm::vec3(0, 0, 1));

	modelCD1 = glm::scale(modelCD1, glm::vec3(0.1, 0.2, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD1));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCD2;
	modelCD2 = glm::translate(modelCD2, glm::vec3(-6, 0, -6));
	modelCD2 = glm::translate(modelCD2, glm::vec3(-0, -0.16, 1.9));

	modelCD2 = glm::scale(modelCD2, glm::vec3(0.1, 0.1, 0.2));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCD3;
	modelCD3 = glm::translate(modelCD3, glm::vec3(-6, 0, -6));
	modelCD3 = glm::translate(modelCD3, glm::vec3(-0.1, -0.16, 1.5));

	modelCD3 = glm::rotate(modelCD3, 15.0f, glm::vec3(0, 1, 0));

	modelCD3 = glm::scale(modelCD3, glm::vec3(0.2, 0.1, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD3));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCD4;
	modelCD4 = glm::translate(modelCD4, glm::vec3(-6, 0, -6));
	modelCD4 = glm::translate(modelCD4, glm::vec3(0.15, -0.16, 1.5));

	modelCD4 = glm::rotate(modelCD4, 359.0f, glm::vec3(0, 1, 0));

	modelCD4 = glm::scale(modelCD4, glm::vec3(0.2, 0.1, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD4));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCDK2;
	modelCDK2 = glm::translate(modelCDK2, glm::vec3(-6, 0, -6));
	modelCDK2 = glm::translate(modelCDK2, glm::vec3(-0, -0.4, 2));

	modelCDK2 = glm::scale(modelCDK2, glm::vec3(0.1, 0.15, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCDK2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCDK3;
	modelCDK3 = glm::translate(modelCDK3, glm::vec3(-6, 0, -6));
	modelCDK3 = glm::translate(modelCDK3, glm::vec3(-0.31, -0.4, 1.4));

	//modelCDK3 = glm::rotate(modelCDK3, 15.0f, glm::vec3(0, 1, 0));

	modelCDK3 = glm::scale(modelCDK3, glm::vec3(0.1, 0.15, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCDK3));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCDK4;
	modelCDK4 = glm::translate(modelCDK4, glm::vec3(-6, 0, -6));
	modelCDK4 = glm::translate(modelCDK4, glm::vec3(0.25, -0.4, 1.4));

	//modelCDK4 = glm::rotate(modelCDK4, 359.0f, glm::vec3(0, 1, 0));

	modelCDK4 = glm::scale(modelCDK4, glm::vec3(0.1, 0.15, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCDK4));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	//atas
	/*glm::mat4 modelCD5;
	modelCD5 = glm::translate(modelCD5, glm::vec3(-9, 0.2, -7));
	modelCD5 = glm::translate(modelCD5, glm::vec3(-0.1, 0, 1.7));

	//modelCD5 = glm::rotate(modelCD5, 10.0f, glm::vec3(0, 1, 0));

	modelCD5 = glm::scale(modelCD5, glm::vec3(0.95, 0.15, 0.8));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD5));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCD6;
	modelCD6 = glm::translate(modelCD6, glm::vec3(-9, 0.9, -7));
	modelCD6 = glm::translate(modelCD6, glm::vec3(-0.1, -0.29, 1.2));

	modelCD6 = glm::rotate(modelCD6, 0.0f, glm::vec3(0, 1, 0));

	modelCD6 = glm::scale(modelCD6, glm::vec3(0.95, 1, 0.2));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD6));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTextureChairDosenAtas(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	//atas
	glm::mat4 modelCD5;
	modelCD5 = glm::translate(modelCD5, glm::vec3(-6, 0.2, -6));
	modelCD5 = glm::translate(modelCD5, glm::vec3(-0.1, 0, 1.7));

	//modelCD5 = glm::rotate(modelCD5, 10.0f, glm::vec3(0, 1, 0));

	modelCD5 = glm::scale(modelCD5, glm::vec3(0.45, 0.1, 0.4));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD5));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelCD6;
	modelCD6 = glm::translate(modelCD6, glm::vec3(-6, 0.9, -6));
	modelCD6 = glm::translate(modelCD6, glm::vec3(-0.1, -0.29, 1.2));

	modelCD6 = glm::rotate(modelCD6, 0.0f, glm::vec3(0, 1, 0));

	modelCD6 = glm::scale(modelCD6, glm::vec3(0.45, 0.5, 0.1));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelCD6));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTextureProjector(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelProj;
	modelProj = glm::translate(modelProj, glm::vec3(-5, 4.8, 0));

	modelProj = glm::rotate(modelProj, 0.0f, glm::vec3(0, 0, 1));

	modelProj = glm::scale(modelProj, glm::vec3(0.05, 0.5, 0.05));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelProj));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelProj2;
	modelProj2 = glm::translate(modelProj2, glm::vec3(-5, 4, 0));

	modelProj2 = glm::rotate(modelProj2, -355.0f, glm::vec3(1, 0, 0));

	modelProj2 = glm::scale(modelProj2, glm::vec3(0.4, 0.2, 0.5));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelProj2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 modelProj3;
	modelProj3 = glm::translate(modelProj3, glm::vec3(-5, 4, -0.51));

	modelProj3 = glm::rotate(modelProj3, -355.0f, glm::vec3(1, 0, 0));

	modelProj3 = glm::scale(modelProj3, glm::vec3(0.4, 0.2, 0));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelProj3));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedScreen(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);

	GLint modelLoc = glGetUniformLocation(shader, "model");

	glm::mat4 modelLayar;
	modelLayar = glm::translate(modelLayar, glm::vec3(-5, 2.7, -6.95));

	modelLayar = glm::rotate(modelLayar, 0.0f, glm::vec3(0, 0, 1));

	modelLayar = glm::scale(modelLayar, glm::vec3(1.7, 1.5, 0.5));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelLayar));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedPlane(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(planeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}

void Demo::InitCamera()
{
	posCamX = 0.0f;
	posCamY = 1.0f;
	posCamZ = 8.0f;
	viewCamX = 0.0f;
	viewCamY = 1.0f;
	viewCamZ = 0.0f;
	upCamX = 0.0f;
	upCamY = 1.0f;
	upCamZ = 0.0f;
	CAMERA_SPEED = 0.001f;
	fovy = 45.0f;
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Demo::MoveCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	// forward positive cameraspeed and backward negative -cameraspeed.
	posCamX = posCamX + x * speed;
	posCamZ = posCamZ + z * speed;
	viewCamX = viewCamX + x * speed;
	viewCamZ = viewCamZ + z * speed;
}

void Demo::StrafeCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	float orthoX = -z;
	float orthoZ = x;

	// left positive cameraspeed and right negative -cameraspeed.
	posCamX = posCamX + orthoX * speed;
	posCamZ = posCamZ + orthoZ * speed;
	viewCamX = viewCamX + orthoX * speed;
	viewCamZ = viewCamZ + orthoZ * speed;
}

void Demo::RotateCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	viewCamZ = (float)(posCamZ + glm::sin(speed) * x + glm::cos(speed) * z);
	viewCamX = (float)(posCamX + glm::cos(speed) * x - glm::sin(speed) * z);
}


int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Camera: Free Camera Implementation", 1920, 1080, false, true);
}