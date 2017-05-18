#include "ScreenQuad.h"

ScreenQuad::ScreenQuad(int state)
{
	if (state == 0) {
		//Bottom Left
		quadVerts[0] = -1.0f;
		quadVerts[1] = -1.0f;
		quadVerts[2] = -3.0f;
		quadVerts[3] = -1.0f;
		quadVerts[4] = -1.0f;

		//Bottom Right
		quadVerts[5] = 1.0f;
		quadVerts[6] = -1.0f;
		quadVerts[7] = -3.0f;
		quadVerts[8] = 1.0f;
		quadVerts[9] = -1.0f;

		//Top Left
		quadVerts[10] = -1.0f;
		quadVerts[11] = 1.0f;
		quadVerts[12] = -3.0f;
		quadVerts[13] = -1.0f;
		quadVerts[14] = 1.0f;

		//Top Right
		quadVerts[15] = 1.0f;
		quadVerts[16] = 1.0f;
		quadVerts[17] = -3.0f;
		quadVerts[18] = 1.0f;
		quadVerts[19] = 1.0f;
	}
	if (state == 1) {
		//Bottom Left
		quadVerts[0] = -1.0f;
		quadVerts[1] = -1.0f;
		quadVerts[2] = -3.0f;
		//Bottom Right
		quadVerts[3] = 1.0f;
		quadVerts[4] = -1.0f;
		quadVerts[5] = -3.0f;
		//Top Left
		quadVerts[6] = -1.0f;
		quadVerts[7] = 1.0f;
		quadVerts[8] = -3.0f;
		//Top Right
		quadVerts[9] = 1.0f;
		quadVerts[10] = 1.0f;
		quadVerts[11] = -3.0f;
	}
	
	static const GLuint quadI[] = {  // Note that we start from 0!
									 // Front face
		0, 1, 2,
		2, 1, 3,
	};
	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadI), quadI, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);



	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);


	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

}

ScreenQuad::~ScreenQuad() {}


void ScreenQuad::draw(GLuint shaderProgram, const glm::mat4 &projection, const glm::mat4 &modelview)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glUseProgram(shaderProgram);

	GLuint texId = glGetUniformLocation(shaderProgram, "texFramebuffer");
	glUniform1i(texId, 0);

	GLuint MatrixID = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projection[0][0]);

	MatrixID = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelview[0][0]);

	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	

}
