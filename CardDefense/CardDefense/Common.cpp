#include "Common.h"
#include <fstream>
#include <sstream>

std::string ReadFile(const char* path)
{
	std::ifstream file(path);
	if (!file) {
		printf("file not found: %s\n", path);
		return "";
	}
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

GLuint CompileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	GLint ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetShaderInfoLog(shader, 512, nullptr, log);
		printf("shader error: %s\n", log);
	}
	return shader;
}

GLuint CreateProgram(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexSource = ReadFile(vertexPath);
	std::string fragmentSource = ReadFile(fragmentPath);
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource.c_str());
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
	GLuint p = glCreateProgram();
	glAttachShader(p, vertexShader);
	glAttachShader(p, fragmentShader);
	glLinkProgram(p);
	GLint ok;
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetProgramInfoLog(p, 512, nullptr, log);
		printf("link error: %s\n", log);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return p;
}

Mesh CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices, GLenum mode)
{
	Mesh m;
	m.mode = mode;
	glGenVertexArrays(1, &m.vao);
	glGenBuffers(1, &m.vbo);
	glBindVertexArray(m.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	if (indices.empty()) {
		m.count = (GLsizei)(vertices.size() / 6);
	}
	else {
		glGenBuffers(1, &m.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);
		m.count = (GLsizei)indices.size();
	}
	glBindVertexArray(0);
	return m;
}

Mesh CreateAxes()
{
	std::vector<float> v = {
		-2.f, 0.f, 0.f, 1.f, 0.f, 0.f,   2.f, 0.f, 0.f, 1.f, 0.f, 0.f,
		0.f, -2.f, 0.f, 0.f, 1.f, 0.f,   0.f, 2.f, 0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, -2.f, 0.f, 0.f, 1.f,   0.f, 0.f, 2.f, 0.f, 0.f, 1.f,
	};
	return CreateMesh(v, {}, GL_LINES);
}

Mesh CreateCube()
{
	const glm::vec3 p[8] = {
		{ -0.5f, -0.5f,  0.5f }, { 0.5f, -0.5f,  0.5f }, { 0.5f, 0.5f,  0.5f }, { -0.5f, 0.5f,  0.5f },
		{ -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f },
	};
	const int faces[6][4] = {
		{ 0, 1, 2, 3 }, { 5, 4, 7, 6 }, { 4, 0, 3, 7 }, { 1, 5, 6, 2 }, { 3, 2, 6, 7 }, { 4, 5, 1, 0 },
	};
	const glm::vec3 colors[6] = {
		{ 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f }, { 1.f, 1.f, 0.f }, { 1.f, 0.f, 1.f }, { 0.f, 1.f, 1.f },
	};
	std::vector<float> v;
	std::vector<unsigned> i;
	for (int f = 0; f < 6; ++f) {
		unsigned base = f * 4;
		for (int k = 0; k < 4; ++k) {
			glm::vec3 q = p[faces[f][k]];
			v.insert(v.end(), { q.x, q.y, q.z, colors[f].r, colors[f].g, colors[f].b });
		}
		i.insert(i.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
	}
	return CreateMesh(v, i, GL_TRIANGLES);
}

void SetMat4(GLuint program, const char* name, const glm::mat4& m)
{
	glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(m));
}

void SetVec3(GLuint program, const char* name, const glm::vec3& v)
{
	glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(v));

}

void Draw(GLuint program, const Mesh& m, const glm::mat4& model)
{
	SetMat4(program, "uModel", model);
	glBindVertexArray(m.vao);
	if (m.ebo) {
		glDrawElements(m.mode, m.count, GL_UNSIGNED_INT, 0);
	}
	else {
		glDrawArrays(m.mode, 0, m.count);
	}
}

glm::mat4 Local(const Object& o, bool isHand)
{
	glm::mat4 model(1.f);

	model = glm::translate(model, o.position);
	model = glm::translate(model, o.pivot);
	
	model = glm::rotate(model, glm::radians(o.roll), glm::vec3(1.f, 0.f, 0.f));
	model = glm::rotate(model, glm::radians(o.yaw), glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, glm::radians(o.pitch), glm::vec3(0.f, 0.f, 1.f));


	model = glm::translate(model, -o.pivot);
	return model;
}

glm::mat4 World(const Object& o)
{
	glm::mat4 model = Local(o, false);
	const Object* p = o.parent;
	

	while (p != nullptr) {
		model = Local(*p, o.ishand) * model;
		p = p->parent;
	}
	return model;
}

glm::vec3 WorldPosition(const Object& o)
{
	return glm::vec3(World(o) * glm::vec4(0.f, 0.f, 0.f, 1.f));
}

glm::vec3 WorldForward(const Object& o)
{
	return glm::normalize(glm::vec3(World(o) * glm::vec4(1.f, 0.f, 0.f, 0.f)));
}
