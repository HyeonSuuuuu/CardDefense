#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
out vec3 vWorldPos;  // 조명 계산용 월드 좌표
out vec3 vNormal;    // 월드 공간 법선
out vec2 vUV;
void main()
{
	vec4 world = uModel * vec4(aPosition, 1.0);
	gl_Position = uProjection * uView * world;
	vWorldPos = world.xyz;
	// 크기를 축마다 다르게 바꿔도 법선이 면에 수직이도록 역전치 행렬을 쓴다.
	vNormal = mat3(transpose(inverse(uModel))) * aNormal;
	vUV = aUV;
}
