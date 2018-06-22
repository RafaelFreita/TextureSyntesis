#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image.h"
#include "stb_image_write.h"


#include <iostream>
#include <fstream>

#include <vector>

#include <glm\glm.hpp>

class Image {

public:
	Image() { glGenTextures(1, &textureID); };
	~Image() {};

	void Bind(std::vector<std::uint8_t> data, int w, int h, bool useAlpha) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		if (useAlpha) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		}
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
		}


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	void Load(const GLchar* path, bool useAlpha) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		data = stbi_load(path, &width, &height, &channels, 0);
		if (data) {
			if (useAlpha) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
		}
		//stbi_image_free(data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void Use() {
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	GLuint textureID;

	unsigned char* data;
	int width;
	int height;
	int channels;

};

const int HEIGHT = 16;
const int WIDTH = 16;

void SaveFile(const char* FileName, std::vector<std::uint8_t> data) {

}

float DistanceBetweenColors(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {

	float distR = ((float)r2 - r1);
	float distG = ((float)g2 - g1);
	float distB = ((float)b2 - b1);
	return (sqrt(pow(distR, 2) + pow(distG, 2) + pow(distG, 2)));
}

unsigned char* GenerateImage(Image* sampleImage, Image* preSynImage) {

	unsigned char* sampleData = sampleImage->data;
	int arrSize = preSynImage->width * preSynImage->height * 3;
	unsigned char* newImage = new unsigned char[arrSize];

	for (int i = 0; i < arrSize; i++)
	{
		newImage[i] = preSynImage->data[i];
	}

	int imageTempX = 0, imageTempY = 0;
	int tempX = 0, tempY = 0;

	glm::vec2 winnerPixelPos;
	float winnerPontuation;
	float pontuation = 0;

	// For every pixel
	for (int y = 0; y < preSynImage->height; y++)
	{
		for (int x = 0; x < preSynImage->width; x++)
		{

			winnerPontuation = INT_MAX;

			// For every pixel in sample image
			for (int sY = 0; sY < sampleImage->height; sY++)
			{
				for (int sX = 0; sX < sampleImage->width; sX++)
				{

					pontuation = 0.0f;

					// Check neighbors
					for (int nY = -2; nY <= 0; nY++)
					{

						imageTempY = y + nY;
						if (imageTempY < 0) {
							imageTempY = preSynImage->height + imageTempY;
						}
						else if (imageTempY >= preSynImage->height) {
							imageTempY = abs(preSynImage->height - imageTempY);
						}

						tempY = sY + nY;
						if (tempY < 0) {
							tempY = sampleImage->height + tempY;
						}
						else if (tempY >= sampleImage->height) {
							tempY = abs(sampleImage->height - tempY);
						}

						for (int nX = -2; nX <= 2; nX++)
						{
							if (nY == 0 && nX > 0) { // Last line going only until the proper pixel
								break;
							}

							imageTempX = x + nX;
							if (imageTempX < 0) {
								imageTempX = preSynImage->width + imageTempX;
							}
							else if (imageTempX >= preSynImage->width) {
								imageTempX = abs(preSynImage->width - imageTempX);
							}

							tempX = sX + nX;
							if (tempX < 0) {
								tempX = sampleImage->width + tempX;
							}
							else if (tempX >= sampleImage->width) {
								tempX = abs(sampleImage->width - tempX);
							}

							// Compare pixels from sample with preSyn
							// 0 - R // 1 - G // 2 - B
							// Multiplying for 3 because every pixel has 3 components

							int newImagePos = imageTempX * 3 + (imageTempY * preSynImage->width * 3);
							int tempPos = tempX * 3 + (tempY * sampleImage->width * 3);

							float dist = DistanceBetweenColors(
								newImage[newImagePos + 0],
								newImage[newImagePos + 1],
								newImage[newImagePos + 2],

								sampleData[tempPos + 0],
								sampleData[tempPos + 1],
								sampleData[tempPos + 2]
							);

							pontuation += dist;
						}
					}
					// End of neighbors

					// Assign new winner if pontution was less
					if (pontuation < winnerPontuation) {
						winnerPixelPos.x = sX;
						winnerPixelPos.y = sY;
						winnerPontuation = pontuation;
					}

				}
			}
			// End of sample check

			int pixelPos = x * 3 + (y * preSynImage->width * 3);
			int winerPixelPos = winnerPixelPos.x * 3 + (winnerPixelPos.y * sampleImage->width * 3);
			newImage[pixelPos + 0] = sampleData[winerPixelPos + 0];
			newImage[pixelPos + 1] = sampleData[winerPixelPos + 1];
			newImage[pixelPos + 2] = sampleData[winerPixelPos + 2];

		}
	}

	return newImage;
}


int main(int argc, char** argv) {

#pragma region Stupid Window
	GLFWwindow* window;
	int screenWidth, screenHeight;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(HEIGHT, WIDTH, "SynWindow", nullptr, nullptr);

	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed no init GLEW." << std::endl;
		return EXIT_FAILURE;
	}
#pragma endregion

	if (argc < 3) {
		std::cout << "You should run this with 3 arguments: sample image path, pre synthese image path and the result image path.\n";
		return 1;
	}

	stbi_set_flip_vertically_on_load(true);

	Image sampleImage;
	sampleImage.Load(argv[1], false);
	Image preSynImage;
	preSynImage.Load(argv[2], false);
	unsigned char* newData = GenerateImage(&sampleImage, &preSynImage);

	std::cout << "Starting synthesis " << std::endl;
	stbi_write_jpg(argv[3], sampleImage.width, sampleImage.height, 3, &newData[0], 100);
	std::cout << "Finished writing to " << argv[3] << std::endl;

	glfwTerminate();
	return 0;
}