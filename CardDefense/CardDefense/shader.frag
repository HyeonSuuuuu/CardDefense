#version 330 core
in vec3 vNormal;
in vec2 vUV;
uniform vec3 uColor;
uniform int uUseTexture;     // 1이면 텍스처 색 × uColor
uniform sampler2D uTexture;
out vec4 fragColor;
void main()
{
	vec3 color = uColor;
	if (uUseTexture == 1) {
		color *= texture(uTexture, vUV).rgb;
	}
	fragColor = vec4(color, 1.0);
}
