#include "Obj.h"
#include "Texture.h"
#include <cstring>

// 경로에서 폴더 부분만 ("a/b/c.obj" → "a/b/")
static std::string Folder(const char* path)
{
	std::string s = path;
	size_t slash = s.find_last_of("/\\");
	return slash == std::string::npos ? "" : s.substr(0, slash + 1);
}

// MTL에서 첫 번째 map_Kd 경로를 찾는다
static std::string ReadDiffuseMap(const std::string& mtlPath)
{
	FILE* file = nullptr;
	fopen_s(&file, mtlPath.c_str(), "r");
	if (!file) {
		printf("mtl not found: %s\n", mtlPath.c_str());
		return "";
	}
	char line[512], name[400];
	std::string result;
	while (fgets(line, sizeof(line), file)) {
		if (sscanf_s(line, " map_Kd %399s", name, (unsigned)sizeof(name)) == 1) {
			result = name;
			break;
		}
	}
	fclose(file);
	return result;
}

// "3/5/2", "3//2", "3/5", "3" 하나를 읽는다. 없는 번호는 0 (OBJ 번호는 1부터).
static void ParseCorner(const char* token, int& v, int& vt, int& vn)
{
	v = vt = vn = 0;
	if (sscanf_s(token, "%d/%d/%d", &v, &vt, &vn) == 3) return;
	if (sscanf_s(token, "%d//%d", &v, &vn) == 2) return;
	if (sscanf_s(token, "%d/%d", &v, &vt) == 2) return;
	sscanf_s(token, "%d", &v);
}

bool LoadModel(const char* objPath, Model& model)
{
	FILE* file = nullptr;
	fopen_s(&file, objPath, "r");
	if (!file) {
		printf("obj not found: %s\n", objPath);
		return false;
	}

	std::vector<glm::vec3> positions, normals;
	std::vector<glm::vec2> uvs;
	std::vector<float> vertices;  // 위치·법선·UV를 꼭짓점마다 펼쳐서 저장 (인덱스 없이 그림)
	std::string mtlName;

	char line[512];
	while (fgets(line, sizeof(line), file)) {
		float x, y, z;
		if (sscanf_s(line, "v %f %f %f", &x, &y, &z) == 3) {
			positions.push_back(glm::vec3(x, y, z));
		}
		else if (sscanf_s(line, "vt %f %f", &x, &y) == 2) {
			uvs.push_back(glm::vec2(x, y));
		}
		else if (sscanf_s(line, "vn %f %f %f", &x, &y, &z) == 3) {
			normals.push_back(glm::vec3(x, y, z));
		}
		else if (strncmp(line, "mtllib ", 7) == 0) {
			char name[400];
			if (sscanf_s(line + 7, "%399s", name, (unsigned)sizeof(name)) == 1) {
				mtlName = name;
			}
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			// 면의 꼭짓점들을 읽고, 4개 이상이면 첫 꼭짓점 기준 부채꼴로 삼각형을 나눈다
			int corners[8][3];
			int count = 0;
			char* context = nullptr;
			for (char* token = strtok_s(line + 2, " \t\r\n", &context); token && count < 8;
				token = strtok_s(nullptr, " \t\r\n", &context)) {
				ParseCorner(token, corners[count][0], corners[count][1], corners[count][2]);
				++count;
			}
			for (int k = 1; k + 1 < count; ++k) {
				const int tri[3] = { 0, k, k + 1 };
				for (int c : tri) {
					int v = corners[c][0], vt = corners[c][1], vn = corners[c][2];
					glm::vec3 p = (v > 0 && v <= (int)positions.size()) ? positions[v - 1] : glm::vec3(0.f);
					glm::vec3 n = (vn > 0 && vn <= (int)normals.size()) ? normals[vn - 1] : glm::vec3(0.f, 1.f, 0.f);
					glm::vec2 t = (vt > 0 && vt <= (int)uvs.size()) ? uvs[vt - 1] : glm::vec2(0.f);
					vertices.insert(vertices.end(), { p.x, p.y, p.z, n.x, n.y, n.z, t.x, t.y });
				}
			}
		}
	}
	fclose(file);

	if (vertices.empty()) {
		printf("obj has no faces: %s\n", objPath);
		return false;
	}

	model.boundsMin = model.boundsMax = positions[0];
	for (const glm::vec3& p : positions) {
		model.boundsMin = glm::min(model.boundsMin, p);
		model.boundsMax = glm::max(model.boundsMax, p);
	}

	model.mesh = CreateMesh(vertices, {}, GL_TRIANGLES);

	model.texture = 0;
	if (!mtlName.empty()) {
		std::string folder = Folder(objPath);
		std::string texture = ReadDiffuseMap(folder + mtlName);
		if (!texture.empty()) {
			model.texture = LoadTexture((folder + texture).c_str());
		}
	}
	return true;
}

void DrawModel(GLuint program, const Model& model, const glm::mat4& transform)
{
	if (model.texture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model.texture);
		SetInt(program, "uTexture", 0);
		SetInt(program, "uUseTexture", 1);
	}
	Draw(program, model.mesh, transform);
	SetInt(program, "uUseTexture", 0);
}
