#include "Capture.h"
#include <gl/glew.h>
#include <cstdio>
#include <vector>

bool SaveScreenshotBmp(const char* path, int width, int height)
{
	// BMP 한 줄은 4바이트 배수여야 한다.
	int stride = (width * 3 + 3) & ~3;
	std::vector<unsigned char> pixels(stride * height);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

	unsigned int imageSize = (unsigned int)pixels.size();
	unsigned char header[54] = { 'B', 'M' };
	auto put32 = [&](int offset, unsigned int value) {
		for (int i = 0; i < 4; ++i) {
			header[offset + i] = (unsigned char)(value >> (8 * i));
		}
	};
	put32(2, 54 + imageSize);       // 파일 크기
	put32(10, 54);                  // 픽셀 시작 위치
	put32(14, 40);                  // 정보 헤더 크기
	put32(18, width);
	put32(22, height);              // 양수 = 아래 줄부터 (glReadPixels와 같은 순서)
	header[26] = 1;                 // 색 평면 수
	header[28] = 24;                // 픽셀당 비트
	put32(34, imageSize);

	FILE* file = nullptr;
	if (fopen_s(&file, path, "wb") != 0 || !file) {
		printf("스크린샷 파일을 열 수 없음: %s\n", path);
		return false;
	}
	fwrite(header, 1, sizeof(header), file);
	fwrite(pixels.data(), 1, pixels.size(), file);
	fclose(file);
	return true;
}
