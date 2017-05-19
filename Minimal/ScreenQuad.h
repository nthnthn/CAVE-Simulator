#ifndef _SCREEN_QUAD_H_
#define _SCREEN_QUAD_H_

#include <GL\glew.h>
#include<glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
class ScreenQuad
{
public:
	ScreenQuad(int state);
	~ScreenQuad();
	void draw(GLuint, GLuint, const glm::mat4 &, const glm::mat4 &, bool isFailing);
	GLuint FramebufferName;
	GLuint renderedTexture;
	GLfloat quadVerts[20];
	glm::vec3 getVertex(int corner);

private:
	glm::mat4 toWorld;
	GLfloat angle;
	GLuint VBO, VAO, EBO, VUV;
};

#endif