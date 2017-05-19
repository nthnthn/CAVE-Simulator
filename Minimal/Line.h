#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"uniform mat4 modelview;"
"uniform mat4 projection;"
"void main()\n"
"{\n"
"gl_Position = projection * modelview* vec4(position.x, position.y, position.z, 1.0);\n"
"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
"out vec4 color;\n"
"uniform vec4 col;"
"void main()\n"
"{\n"
"color = col;\n"
"}\n\0";

class Line
{
public:
	GLuint shaderProgram, VBO, VAO;
	GLfloat vertices[6];
	vec4 Color;

	Line(glm::vec3 endpoint) {
		colorGreen();
		// Build and compile our shader program
		// Vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		// Check for compile time errors
		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		// Check for compile time errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Link shaders
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		// Check for linking errors
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);


		//// Set up vertex data (and buffer(s)) and attribute pointers
		vertices[0] = -0.5f;
		vertices[1] = -0.5f;
		vertices[2] = 0.0f;
		vertices[3] = endpoint.x;
		vertices[4] = endpoint.y;
		vertices[5] = endpoint.z;
		//vertices = {
		//	-0.5f, -0.5f, 0.0f, // Left  
		//	0.5f, -0.5f, 0.0f, // Right 
		//	//0.0f,  0.5f, 0.0f  // Top   
		//};
		//GLuint VBO, VAO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
		glLineWidth(10.0f);
	}

	void colorGreen() {
		Color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	void colorRed() {
		Color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	void update(float x, float y, float z, float orientX, float orientY, float orientZ) {
		vertices[0] = x;
		vertices[1] = y;
		vertices[2] = z;
		vertices[3] = orientX;
		vertices[4] = orientY;
		vertices[5] = orientZ;
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
	}

	void update(float x, float y, float z) {
		vertices[0] = x;
		vertices[1] = y;
		vertices[2] = z;
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

		glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
	}

	void draw(const mat4 & projection, const mat4 & modelview)
	{
		glUseProgram(shaderProgram);
		GLuint location = glGetUniformLocation(shaderProgram, "modelview");
		glUniformMatrix4fv(location, 1, GL_FALSE, &modelview[0][0]);
		location = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);
		location = glGetUniformLocation(shaderProgram, "col");
		glUniform4fv(location, 1, &Color[0]);
		// Render
		// Clear the colorbuffer
		// Draw our first triangle
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 3);
		glBindVertexArray(0);

		// Swap the screen buffers
		//glfwSwapBuffers(window);
	}

	vec3 getStart() {
		return vec3(vertices[0], vertices[1], vertices[2]);
	}

	vec3 getEnd() {
		return vec3(vertices[3], vertices[4], vertices[5]);
	}
};
