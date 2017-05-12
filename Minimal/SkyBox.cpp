#include "SkyBox.h"

GLfloat skyVerts[] = {
	// Front skyVerts
	-0.5, -0.5,  0.5,
	0.5, -0.5,  0.5,
	0.5,  0.5,  0.5,
	-0.5,  0.5,  0.5,
	// Back skyVerts
	-0.5, -0.5, -0.5,
	0.5, -0.5, -0.5,
	0.5,  0.5, -0.5,
	-0.5,  0.5, -0.5
};

GLuint skyI[] = {  // Note that we start from 0!
				   // Front face
	0, 1, 2,
	2, 3, 0,
	// Top face
	1, 5, 6,
	6, 2, 1,
	// Back face
	7, 6, 5,
	5, 4, 7,
	// Bottom face
	4, 0, 3,
	3, 7, 4,
	// Left face
	4, 5, 1,
	1, 0, 4,
	// Right face
	3, 2, 6,
	6, 7, 3
};

SkyBox::SkyBox(int state)
{
	this->toWorld = glm::mat4(1.0f);
	if (state == 0) {
		//left, right, up, down, back, front = "../Minimal/Textures/vr_test_pattern.ppm";
		for (int i = 0; i < 6; i++) {
			faces.push_back("../Minimal/Textures/vr_test_pattern.ppm");
		}
	}
	else if (state == 5) {
		scale(glm::vec3(5.0f, 20.0f, 5.0f));
		faces.push_back("../Minimal/Textures/custom/left.ppm");
		faces.push_back("../Minimal/Textures/custom/right.ppm");
		faces.push_back("../Minimal/Textures/custom/top.ppm");
		faces.push_back("../Minimal/Textures/custom/bottom.ppm");
		faces.push_back("../Minimal/Textures/custom/back.ppm");
		faces.push_back("../Minimal/Textures/custom/front.ppm");
	}
	else {
		scale(100.0f);
		if (state == 1) {
			faces.push_back("../Minimal/Textures/left-ppm/px.ppm");
			faces.push_back("../Minimal/Textures/left-ppm/nx.ppm"); 
			faces.push_back("../Minimal/Textures/left-ppm/py.ppm");
			faces.push_back("../Minimal/Textures/left-ppm/ny.ppm");
			faces.push_back("../Minimal/Textures/left-ppm/pz.ppm");
			faces.push_back("../Minimal/Textures/left-ppm/nz.ppm");
		}

		else if (state == 2) {
			faces.push_back("../Minimal/Textures/right-ppm/px.ppm");
			faces.push_back("../Minimal/Textures/right-ppm/nx.ppm");
			faces.push_back("../Minimal/Textures/right-ppm/py.ppm");
			faces.push_back("../Minimal/Textures/right-ppm/ny.ppm");
			faces.push_back("../Minimal/Textures/right-ppm/pz.ppm");
			faces.push_back("../Minimal/Textures/right-ppm/nz.ppm");
		}

		else if (state == 3) {
			scale(0.01f);
			translate(glm::vec3(0.0f, -0.05f, 0.0f));
			//left, right, up, down, back, front = "../Minimal/Textures/vr_test_pattern.ppm";
			for (int i = 0; i < 6; i++) {
				faces.push_back("../Minimal/Textures/left.ppm");
			}
		}

		else if (state == 4) {
			scale(0.01f);
			translate(glm::vec3(0.0f, -0.05f, 0.0f));
			//left, right, up, down, back, front = "../Minimal/Textures/vr_test_pattern.ppm";
			for (int i = 0; i < 6; i++) {
				faces.push_back("../Minimal/Textures/right.ppm");
			}
		}
	}

	this->angle = 0.0f;

	unsigned char *image;
	int width, height;

	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyVerts), skyVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyI), skyI, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	numOfIndices = 36;

	glGenTextures(1, &textId);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textId);

	for (GLuint i = 0; i < faces.size(); i++) {
		image = loadPPM(faces[i], width, height);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width,
			height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}


	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// Make sure no bytes are padded:
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Select GL_MODULATE to mix texture with polygon color for shading:
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

SkyBox::~SkyBox() {}


void SkyBox::draw(GLuint shaderProgram, const glm::mat4 &projection, const glm::mat4 &modelview)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, textId);
	glUseProgram(shaderProgram);
	GLuint MatrixID = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &projection[0][0]);

	MatrixID = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(modelview*toWorld)[0][0]);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numOfIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void SkyBox::scale(float scalefactor) {
	this->toWorld = glm::scale(this->toWorld, glm::vec3(scalefactor, scalefactor, scalefactor));
}

void SkyBox::scale(glm::vec3 scalarVector) {
	this->toWorld = glm::scale(this->toWorld, scalarVector);
}

void SkyBox::translate(glm::vec3 transfactor) {
	this->toWorld = glm::translate(glm::mat4(1.0f), transfactor) * this->toWorld;
}

void SkyBox::setScale(float scalefactor) {
	this->toWorld = glm::mat4(1.0f);
	translate(glm::vec3(0.0f, 0.0f, -0.4f));
	scale(scalefactor);
}

unsigned char* SkyBox::loadPPM(const char* filename, int& width, int& height)
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
		return 0;
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

		return 0;
	}

	return rawData;
}