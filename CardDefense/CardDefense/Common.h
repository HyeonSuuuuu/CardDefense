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

// 정점 형식: 위치(location 0) + 법선(1) + UV(2) = float 8개
constexpr int VertexFloats = 8;

struct Mesh
{
	GLuint vao = 0, vbo = 0, ebo = 0;
	GLsizei count = 0;
	GLenum mode = GL_TRIANGLES;
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
void SetInt(GLuint program, const char* name, int v);
void Draw(GLuint program, const Mesh& m, const glm::mat4& model);
