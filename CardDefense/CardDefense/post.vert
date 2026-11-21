#version 330 core
// 화면 전체를 덮는 삼각형 하나 (정점 버퍼 없이 gl_VertexID 0, 1, 2로 만든다)
out vec2 vUV;
void main()
{
	vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);  // (0,0) (2,0) (0,2)
	vUV = p;
	gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
