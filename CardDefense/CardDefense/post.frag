#version 330 core
// 회화풍 후처리: 쿠와하라 필터. 픽셀 둘레를 네 사분면으로 나눠 색이 가장 고른(분산이 작은) 사분면의 평균을 쓴다.
// 평평한 곳은 붓 자국처럼 뭉개지고, 경계는 분산이 큰 쪽을 피하므로 흐려지지 않는다.
in vec2 vUV;
uniform sampler2D uScene;  // 3D 장면을 그린 색 텍스처
uniform int uRadius;       // 사분면 한 변 = uRadius + 1 픽셀
out vec4 fragColor;

void main()
{
	vec2 texel = 1.0 / vec2(textureSize(uScene, 0));
	const vec2 dirs[4] = vec2[4](vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));
	float count = float((uRadius + 1) * (uRadius + 1));
	float best = 1e9;
	vec3 color = vec3(0.0);
	for (int q = 0; q < 4; ++q) {
		vec3 sum = vec3(0.0), sum2 = vec3(0.0);
		for (int x = 0; x <= uRadius; ++x) {
			for (int y = 0; y <= uRadius; ++y) {
				vec3 c = texture(uScene, vUV + dirs[q] * vec2(x, y) * texel).rgb;
				sum += c;
				sum2 += c * c;
			}
		}
		vec3 mean = sum / count;
		vec3 variance = sum2 / count - mean * mean;  // E[c²] - E[c]²
		float v = variance.r + variance.g + variance.b;
		if (v < best) {
			best = v;
			color = mean;
		}
	}
	fragColor = vec4(color, 1.0);
}
