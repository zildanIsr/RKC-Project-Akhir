#pragma once
#include "RenderEngine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <SOIL/SOIL.h>

class Demo :
	public RenderEngine
{
public:
	Demo();
	~Demo();
private:
	GLuint depthmapShader, shadowmapShader, cubeVBO, cubeVAO, cubeEBO, cube_texture, planeVBO, planeVAO, planeEBO, plane_texture, stexture, stexture2, depthMapFBO, depthMap,
		texture, texture2, texture3, texture4, texture5, texture6, texture7, texture8, texture9, texture10, texture11, texture12, texture13;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	float viewCamX, viewCamY, viewCamZ, upCamX, upCamY, upCamZ, posCamX, posCamY, posCamZ, CAMERA_SPEED, fovy;
	float angle = 0;
	virtual void Init();
	virtual void DeInit();
	virtual void Update(double deltaTime);
	virtual void Render();
	virtual void ProcessInput(GLFWwindow *window);
	void BuildTexturedCube();
	void BuildTexturedPlane();
	void DrawTexturedCube(GLuint shader);
	void DrawTexturedPlane(GLuint shader);
	void BuildDepthMap();
	void BuildShaders();
	void MoveCamera(float speed);
	void StrafeCamera(float speed);
	void RotateCamera(float speed);
	void InitCamera();
};

