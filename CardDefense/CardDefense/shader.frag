#version 330 core
in vec3 vWorldPos;
in vec3 vLocalPos;
in vec3 vNormal;
in vec2 vUV;
in vec4 vLightPos;
in vec4 vPaintPos;
uniform vec3 uColor;
uniform int uUseTexture;     // 1이면 텍스처 색 × uColor
uniform sampler2D uTexture;
uniform int uLighting;       // 1이면 조명 계산, 0이면 색 그대로 (UI·체력 막대)
uniform int uToon;           // 1이면 툰 셰이딩(밝기 단계 + 테두리 빛), 0이면 퐁 셰이딩
uniform vec3 uLightDir;      // 표면에서 빛 쪽으로 가는 방향 (정규화)
uniform vec3 uViewPos;       // 카메라 월드 좌표
uniform float uFlash;        // 0~1, 맞았을 때 흰색으로 섞는 정도
uniform float uAlpha;        // 불투명도 (텍스처 알파와 곱함)
uniform float uDissolve;     // 0~1, 사라지는 정도 (0이면 끔)
uniform sampler2D uShadowMap;
uniform int uShadowOn;
uniform sampler2D uPaint;    // 판 원화 (기본 카메라 시점 그림)
uniform int uPaintOn;
uniform int uPaintSurface;   // 1이면 이 물체에 원화를 투사한다 (판)
uniform vec3 uPaintPos;      // 프로젝터 위치 = 기본 카메라 위치
out vec4 fragColor;

// ---- fBm 노이즈: 격자 꼭짓점마다 난수를 두고 부드럽게 보간한 값 노이즈를 여러 크기로 겹친다
float Hash(vec3 p)
{
	p = fract(p * 0.3183099 + 0.1);
	p *= 17.0;
	return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float ValueNoise(vec3 p)
{
	vec3 i = floor(p);
	vec3 f = fract(p);
	vec3 u = f * f * (3.0 - 2.0 * f);  // 경계에서 매끄럽게
	return mix(mix(mix(Hash(i + vec3(0, 0, 0)), Hash(i + vec3(1, 0, 0)), u.x),
	               mix(Hash(i + vec3(0, 1, 0)), Hash(i + vec3(1, 1, 0)), u.x), u.y),
	           mix(mix(Hash(i + vec3(0, 0, 1)), Hash(i + vec3(1, 0, 1)), u.x),
	               mix(Hash(i + vec3(0, 1, 1)), Hash(i + vec3(1, 1, 1)), u.x), u.y), u.z);
}

float Fbm(vec3 p)
{
	float sum = 0.0;
	float amplitude = 0.5;
	for (int k = 0; k < 4; ++k) {   // 옥타브 4개: 크기는 절반, 주파수는 두 배씩
		sum += amplitude * ValueNoise(p);
		p *= 2.0;
		amplitude *= 0.5;
	}
	return sum / 0.9375;  // 0.5 + 0.25 + 0.125 + 0.0625로 나눠 0~1로
}

// 0(빛 받음) ~ 1(그림자). 주변 3×3 텍셀을 같이 비교해(PCF) 그림자 가장자리를 부드럽게
float Shadow(vec3 n)
{
	vec3 p = vLightPos.xyz / vLightPos.w * 0.5 + 0.5;  // NDC → 0~1 텍스처 좌표·깊이
	if (p.z > 1.0) {
		return 0.0;
	}
	// 빛과 비스듬한 면일수록 깊이 오차가 커서 바이어스를 키운다 (자기 그림자 줄무늬 방지)
	float bias = max(0.004 * (1.0 - dot(n, uLightDir)), 0.0008);
	vec2 texel = 1.0 / textureSize(uShadowMap, 0);
	float shadow = 0.0;
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float nearest = texture(uShadowMap, p.xy + vec2(x, y) * texel).r;
			shadow += p.z - bias > nearest ? 1.0 : 0.0;
		}
	}
	return shadow / 9.0;
}

void main()
{
	// 디졸브: 노이즈 값이 기준보다 작은 조각은 버리고, 경계는 주황색으로 태운다
	bool burning = false;
	if (uDissolve > 0.0) {
		float noise = Fbm(vLocalPos * 6.0);
		if (noise < uDissolve) {
			discard;
		}
		if (noise < uDissolve + 0.08) {
			burning = true;
		}
	}

	vec3 color = uColor;
	float alpha = uAlpha;
	if (uUseTexture == 1) {
		vec4 texel = texture(uTexture, vUV);
		color *= texel.rgb;
		alpha *= texel.a;
	}

	// 투사 텍스처 매핑: 프로젝터 클립 좌표를 w로 나눠 0~1 그림 좌표로. 프로젝터를 향한 면에만
	bool painted = false;
	if (uPaintOn == 1 && uPaintSurface == 1 && vPaintPos.w > 0.0) {
		vec2 uv = vPaintPos.xy / vPaintPos.w * 0.5 + 0.5;
		bool facing = dot(normalize(vNormal), uPaintPos - vWorldPos) > 0.0;
		if (facing && all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)))) {
			color = uColor * texture(uPaint, uv).rgb;
			painted = true;
		}
	}

	if (painted && uLighting == 1) {
		// 원화에 명암이 이미 그려져 있으므로 조명은 다시 하지 않고, 말이 드리운 그림자만 얹는다
		float shadow = uShadowOn == 1 ? Shadow(normalize(vNormal)) : 0.0;
		color *= 1.0 - 0.45 * shadow;
	}
	else if (uLighting == 1) {
		vec3 n = normalize(vNormal);
		if (!gl_FrontFacing) {
			n = -n;  // 활시위처럼 얇은 면은 뒷면도 보이므로 법선을 뒤집는다
		}
		vec3 v = normalize(uViewPos - vWorldPos);
		vec3 r = reflect(-uLightDir, n);
		float shadow = uShadowOn == 1 ? Shadow(n) : 0.0;
		float diffuse = max(dot(n, uLightDir), 0.0) * (1.0 - 0.8 * shadow);
		float specular = pow(max(dot(r, v), 0.0), 32.0) * (1.0 - shadow);

		if (uToon == 1) {
			// 밝기를 세 단계로 끊고, 반사광은 켜고 끄기만, 가장자리에 테두리 빛
			float band = diffuse > 0.6 ? 1.0 : (diffuse > 0.25 ? 0.75 : 0.5);
			float highlight = specular > 0.5 ? 0.2 : 0.0;
			float rim = smoothstep(0.62, 0.7, 1.0 - max(dot(n, v), 0.0)) * diffuse;
			color = color * (0.35 + 0.7 * band) + vec3(highlight) + vec3(0.25 * rim);
		}
		else {
			color = color * (0.4 + 0.65 * diffuse) + vec3(0.2 * specular);
		}
	}

	color = mix(color, vec3(1.0), uFlash);
	if (burning) {
		color = vec3(1.0, 0.5, 0.15);  // 타는 가장자리는 조명과 상관없이 주황
	}
	fragColor = vec4(color, alpha);
}
