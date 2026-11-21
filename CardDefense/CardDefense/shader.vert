#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uOutline; // 0보다 크면 정점을 법선 방향으로 이만큼 밀어낸다 (툰 외곽선용 껍데기)
uniform mat4 uLightSpace; // 월드 → 빛의 클립 좌표 (그림자 맵 비교용)
uniform vec4 uUVRect;   // 텍스처의 일부만 쓸 때 (xy = 시작, zw = 크기). 기본 (0, 0, 1, 1)
out vec3 vWorldPos;  // 조명 계산용 월드 좌표
out vec3 vLocalPos;  // 모델 좌표 (디졸브 노이즈가 몹을 따라 움직이도록)
out vec3 vNormal;    // 월드 공간 법선
out vec2 vUV;
out vec4 vLightPos;  // 빛에서 본 위치
uniform mat4 uPaintMatrix; // 월드 → 원화 프로젝터의 클립 좌표
out vec4 vPaintPos;  // 프로젝터에서 본 위치 (투사 텍스처 좌표)
void main()
{
	vec4 world = uModel * vec4(aPosition + aNormal * uOutline, 1.0);
	gl_Position = uProjection * uView * world;
	vWorldPos = world.xyz;
	vLocalPos = aPosition;
	// 크기를 축마다 다르게 바꿔도 법선이 면에 수직이도록 역전치 행렬을 쓴다.
	vNormal = mat3(transpose(inverse(uModel))) * aNormal;
	vUV = uUVRect.xy + aUV * uUVRect.zw;
	vLightPos = uLightSpace * world;
	vPaintPos = uPaintMatrix * world;
}
