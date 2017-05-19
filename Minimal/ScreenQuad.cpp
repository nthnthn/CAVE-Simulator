#include "ScreenQuad.h"

ScreenQuad::ScreenQuad(int state)
{
	if (state == 0) {
		//Bottom Left
		quadVerts[0] = -1.0f;
		quadVerts[1] = -1.0f;
		quadVerts[2] = -3.0f;
		quadVerts[3] = 0.0f;
		quadVerts[4] = 0.0f;

		//Bottom Right
		quadVerts[5] = 1.0f;
		quadVerts[6] = -1.0f;
		quadVerts[7] = -3.0f;
		quadVerts[8] = 1.0f;
		quadVerts[9] = 0.0f;

		//Top Left
		quadVerts[10] = -1.0f;
		quadVerts[11] = 1.0f;
		quadVerts[12] = -3.0f;
		quadVerts[13] = 0.0f;
		quadVerts[14] = 1.0f;

		//Top Right
		quadVerts[15] = 1.0f;
		quadVerts[16] = 1.0f;
		quadVerts[17] = -3.0f;
		quadVerts[18] = 1.0f;
		quadVerts[19] = 1.0f;
	}
	else if (state == 1) {
		//Bottom Left
		quadVerts[0] = -1.697f;
		quadVerts[1] = -1.2f;
		quadVerts[2] = 0.0f;
		quadVerts[3] = 0.0f;
		quadVerts[4] = 0.0f;
	
		//Bottom Right
		quadVerts[5] = 0.0f;
		quadVerts[6] = -1.2f;
		quadVerts[7] = -1.697f;
		quadVerts[8] = 1.0f;
		quadVerts[9] = 0.0f;

		//Top Left
		quadVerts[10] = -1.697f;
		quadVerts[11] = 1.2f;
		quadVerts[12] = 0.0f;
		quadVerts[13] = 0.0f;
		quadVerts[14] = 1.0f;

		//Top Right
		quadVerts[15] = 0.0f;
		quadVerts[16] = 1.2f;
		quadVerts[17] = -1.697f;
		quadVerts[18] = 1.0f;
		quadVerts[19] = 1.0f;
	}
	else if (state == 2) {
		//Bottom Left
		quadVerts[0] = 0.0f;
		quadVerts[1] = -1.2f;
		quadVerts[2] = -1.697f;
		quadVerts[3] = 0.0f;
		quadVerts[4] = 0.0f;

		//Bottom Right
		quadVerts[5] = 1.697f;
		quadVerts[6] = -1.2f;
		quadVerts[7] = 0.0f;
		quadVerts[8] = 1.0f;
		quadVerts[9] = 0.0f;

		//Top Left
		quadVerts[10] = 0.0f;
		quadVerts[11] = 1.2f;
		quadVerts[12] = -1.697f;
		quadVerts[13] = 0.0f;
		quadVerts[14] = 1.0f;

		//Top Right
		quadVerts[15] = 1.697f;
		quadVerts[16] = 1.2f;
		quadVerts[17] = 0.0f;
		quadVerts[18] = 1.0f;
		quadVerts[19] = 1.0f;
	}
	else if (state == 3) {
		//Left
		quadVerts[0] = -1.697f;
		quadVerts[1] = -1.2f;
		quadVerts[2] = 0.0f;
		quadVerts[3] = 0.0f;
		quadVerts[4] = 0.0f;

		//Front
		quadVerts[5] = 0.0f;
		quadVerts[6] = -1.2f;
		quadVerts[7] = -1.697f;
		quadVerts[8] = 1.0f;
		quadVerts[9] = 0.0f;

		//Back
		quadVerts[10] = 0.0f;
		quadVerts[11] = -1.2f;
		quadVerts[12] = 1.697f;
		quadVerts[13] = 0.0f;
		quadVerts[14] = 1.0f;

		//Right
		quadVerts[15] = 1.697f;
		quadVerts[16] = -1.2f;
		quadVerts[17] = 0.0f;
		quadVerts[18] = 1.0f;
		quadVerts[19] = 1.0f;
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
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(sizeof(float) * 3));



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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);


	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

}

ScreenQuad::~ScreenQuad() {}


glm::vec3 ScreenQuad::getVertex(int corner) {
	return glm::vec3(quadVerts[corner * 5], quadVerts[corner * 5 + 1], quadVerts[corner * 5 + 2]);
}

void ScreenQuad::draw(GLuint shaderProgram, GLuint blankShader, const glm::mat4 &projection, const glm::mat4 &modelview, bool isFailing)
{
	GLuint shader = (isFailing) ? blankShader : shaderProgram;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glUseProgram(shader);

	GLuint texId = glGetUniformLocation(shader, "texFramebuffer");
	glUniform1i(texId, 0);

	
	GLuint MatrixID = glGetUniformLocation(shader, "projection");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projection[0][0]);

	MatrixID = glGetUniformLocation(shader, "modelview");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &modelview[0][0]);

	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
	
}
