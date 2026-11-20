#include "Common.h"
#include <cmath>
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
	const GLsizei stride = VertexFloats * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                    // 위치
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));  // 법선
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));  // UV
	glEnableVertexAttribArray(2);
	if (indices.empty()) {
		m.count = (GLsizei)(vertices.size() / VertexFloats);
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
	// 선이라 법선·UV는 쓰지 않는다 (0으로 채움)
	std::vector<float> v = {
		-2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,   2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
		0.f, -2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,   0.f, 2.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, -2.f, 0.f, 0.f, 0.f, 0.f, 0.f,   0.f, 0.f, 2.f, 0.f, 0.f, 0.f, 0.f, 0.f,
	};
	return CreateMesh(v, {}, GL_LINES);
}

Mesh CreateCube()
{
	const glm::vec3 p[8] = {
		{ -0.5f, -0.5f,  0.5f }, { 0.5f, -0.5f,  0.5f }, { 0.5f, 0.5f,  0.5f }, { -0.5f, 0.5f,  0.5f },
		{ -0.5f, -0.5f, -0.5f }, { 0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, -0.5f }, { -0.5f, 0.5f, -0.5f },
	};
	// 면마다 꼭짓점 4개 (바깥에서 봤을 때 반시계 방향)
	const int faces[6][4] = {
		{ 0, 1, 2, 3 }, { 5, 4, 7, 6 }, { 4, 0, 3, 7 }, { 1, 5, 6, 2 }, { 3, 2, 6, 7 }, { 4, 5, 1, 0 },
	};
	const glm::vec3 normals[6] = {
		{ 0.f, 0.f, 1.f }, { 0.f, 0.f, -1.f }, { -1.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, -1.f, 0.f },
	};
	const glm::vec2 uvs[4] = { { 0.f, 0.f }, { 1.f, 0.f }, { 1.f, 1.f }, { 0.f, 1.f } };

	std::vector<float> v;
	std::vector<unsigned> i;
	for (int f = 0; f < 6; ++f) {
		unsigned base = f * 4;
		for (int k = 0; k < 4; ++k) {
			glm::vec3 q = p[faces[f][k]];
			glm::vec3 n = normals[f];
			v.insert(v.end(), { q.x, q.y, q.z, n.x, n.y, n.z, uvs[k].x, uvs[k].y });
		}
		i.insert(i.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
	}
	return CreateMesh(v, i, GL_TRIANGLES);
}

Mesh CreateDisc(int segments)
{
	// 가운데 점 하나 + 테두리 점들, 삼각형을 부채꼴로 잇는다
	std::vector<float> v = { 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.5f, 0.5f };
	std::vector<unsigned> idx;
	for (int k = 0; k <= segments; ++k) {
		float a = 6.2831853f * k / segments;
		float x = std::cos(a), z = std::sin(a);
		v.insert(v.end(), { x, 0.f, z, 0.f, 1.f, 0.f, 0.5f + 0.5f * x, 0.5f + 0.5f * z });
		if (k > 0) {
			idx.insert(idx.end(), { 0u, (unsigned)k + 1, (unsigned)k });
		}
	}
	return CreateMesh(v, idx, GL_TRIANGLES);
}

Mesh CreateCircle(int segments)
{
	std::vector<float> v;
	for (int k = 0; k < segments; ++k) {
		float a = 6.2831853f * k / segments;
		v.insert(v.end(), { std::cos(a), 0.f, std::sin(a), 0.f, 1.f, 0.f, 0.f, 0.f });
	}
	return CreateMesh(v, {}, GL_LINE_LOOP);
}

Mesh CreateRoundedTile(float radius, int segments)
{
	// 테두리 점: 네 모서리 원 중심 + r(cos t, sin t). t가 0 → 2π로 한 바퀴 돌며 모서리마다 1/4 원씩
	const float inner = 0.5f - radius;
	const float cx[4] = { inner, -inner, -inner, inner };
	const float cz[4] = { inner, inner, -inner, -inner };
	std::vector<glm::vec2> rim, out;
	for (int c = 0; c < 4; ++c) {
		for (int k = 0; k <= segments; ++k) {
			float t = 1.5707963f * (c + (float)k / segments);
			glm::vec2 dir(std::cos(t), std::sin(t));
			rim.push_back(glm::vec2(cx[c], cz[c]) + radius * dir);
			out.push_back(dir);
		}
	}
	int n = (int)rim.size();

	std::vector<float> v;
	std::vector<unsigned> idx;
	// 윗면: 가운데 점에서 부채꼴. UV는 큐브 윗면과 같게 (u = x + 0.5, v = 0.5 - z)
	v.insert(v.end(), { 0.f, 0.5f, 0.f, 0.f, 1.f, 0.f, 0.5f, 0.5f });
	for (int k = 0; k < n; ++k) {
		v.insert(v.end(), { rim[k].x, 0.5f, rim[k].y, 0.f, 1.f, 0.f, rim[k].x + 0.5f, 0.5f - rim[k].y });
		unsigned a = 1 + k, b = 1 + (k + 1) % n;
		idx.insert(idx.end(), { 0u, b, a });  // 위에서 보면 t가 줄어드는 쪽이 반시계
	}
	// 옆면: 테두리 점마다 위·아래 두 점, 법선은 바깥쪽 (모서리에서는 원의 반지름 방향)
	unsigned base = (unsigned)(v.size() / VertexFloats);
	for (int k = 0; k < n; ++k) {
		float u = (float)k / n;
		v.insert(v.end(), { rim[k].x, -0.5f, rim[k].y, out[k].x, 0.f, out[k].y, u, 0.f });
		v.insert(v.end(), { rim[k].x, 0.5f, rim[k].y, out[k].x, 0.f, out[k].y, u, 1.f });
	}
	for (int k = 0; k < n; ++k) {
		unsigned b0 = base + 2 * k, t0 = b0 + 1;
		unsigned b1 = base + 2 * ((k + 1) % n), t1 = b1 + 1;
		idx.insert(idx.end(), { b1, b0, t0, b1, t0, t1 });
	}
	return CreateMesh(v, idx, GL_TRIANGLES);
}

Mesh CreateQuad()
{
	// 화면 좌표는 y가 아래로 커지고, 텍스처는 위아래를 뒤집어 읽었으므로(v = 0이 그림 아래)
	// 위쪽(y = 0) 꼭짓점에 v = 1을 준다.
	std::vector<float> v = {
		0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
		1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f,
		0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
	};
	return CreateMesh(v, { 0, 1, 2, 0, 2, 3 }, GL_TRIANGLES);
}

void SetVec4(GLuint program, const char* name, const glm::vec4& v)
{
	glUniform4fv(glGetUniformLocation(program, name), 1, glm::value_ptr(v));
}

void SetMat4(GLuint program, const char* name, const glm::mat4& m)
{
	glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(m));
}

void SetVec3(GLuint program, const char* name, const glm::vec3& v)
{
	glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(v));
}

void SetInt(GLuint program, const char* name, int v)
{
	glUniform1i(glGetUniformLocation(program, name), v);
}

void SetFloat(GLuint program, const char* name, float v)
{
	glUniform1f(glGetUniformLocation(program, name), v);
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
