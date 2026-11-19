#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
uniform vec3 uColor;
uniform int uUseTexture;     // 1이면 텍스처 색 × uColor
uniform sampler2D uTexture;
uniform int uLighting;       // 1이면 퐁 조명, 0이면 색 그대로 (축·체력 막대)
uniform vec3 uLightDir;      // 표면에서 빛 쪽으로 가는 방향 (정규화)
uniform vec3 uViewPos;       // 카메라 월드 좌표
uniform float uFlash;        // 0~1, 맞았을 때 흰색으로 섞는 정도
uniform float uAlpha;        // 불투명도 (텍스처 알파와 곱함)
out vec4 fragColor;
void main()
{
	vec3 color = uColor;
	float alpha = uAlpha;
	if (uUseTexture == 1) {
		vec4 texel = texture(uTexture, vUV);
		color *= texel.rgb;
		alpha *= texel.a;
	}

	if (uLighting == 1) {
		vec3 n = normalize(vNormal);
		if (!gl_FrontFacing) {
			n = -n;  // 활시위처럼 얇은 면은 뒷면도 보이므로 법선을 뒤집는다
		}
		vec3 v = normalize(uViewPos - vWorldPos);
		vec3 r = reflect(-uLightDir, n);
		float ambient = 0.4;
		float diffuse = 0.65 * max(dot(n, uLightDir), 0.0);
		float specular = 0.2 * pow(max(dot(r, v), 0.0), 32.0);
		color = color * (ambient + diffuse) + vec3(specular);
	}

	color = mix(color, vec3(1.0), uFlash);
	fragColor = vec4(color, alpha);
}
