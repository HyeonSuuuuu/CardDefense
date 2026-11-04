#pragma once
#include <gl/glew.h>
#include <gl/glfw3.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <string>
#include <vector>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")

struct Mesh
{
	GLuint vao = 0, vbo = 0, ebo = 0;
	GLsizei count = 0;
	GLenum mode = GL_TRIANGLES;
};

struct Object
{
	Object(Mesh& m, glm::vec3 p, glm::vec3 s) : mesh(&m), position(p), scale(s) {}

	Mesh* mesh = nullptr;
	Object* parent = nullptr;
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 velocity = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 pivot = glm::vec3(0.f);
	float roll = 0.f;
	float yaw = 0.f;
	float pitch = 0.f;
	glm::vec3 dir = glm::vec3(0.f);
	bool select = true;

	glm::vec3 color = glm::vec3(1.f, 0.f, 0.f);

	bool ishand = false;
};

struct Camera
{
	float distance = 11.f;
	float pitch = 50.f;
	float yaw = 0.f;
	float yawDir = 0.f;
};

std::string ReadFile(const char* path);
GLuint CreateProgram(const char* vertexPath, const char* fragmentPath);
Mesh CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices, GLenum mode);
Mesh CreateAxes();
Mesh CreateCube();
void SetMat4(GLuint program, const char* name, const glm::mat4& m);
void SetVec3(GLuint program, const char* name, const glm::vec3& v);
void Draw(GLuint program, const Mesh& m, const glm::mat4& model);
glm::mat4 Local(const Object& o);
glm::mat4 World(const Object& o);
glm::vec3 WorldPosition(const Object& o);
glm::vec3 WorldForward(const Object& o);
