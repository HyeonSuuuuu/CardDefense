#include "Texture.h"
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint LoadTexture(const char* path)
{
	stbi_set_flip_vertically_on_load(1);
	int width, height, channels;
	unsigned char* pixels = stbi_load(path, &width, &height, &channels, 4);  // 항상 RGBA로 받는다
	if (!pixels) {
		printf("texture load failed: %s (%s)\n", path, stbi_failure_reason());
		return 0;
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(pixels);
	return texture;
}
