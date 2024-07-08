#include <glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include "shader_s.h"
typedef struct tile
{
	uint16_t id;
 } tile;
#define tileSize 0.02f
#define widht 100
typedef struct buffer
{
	unsigned int VAO, VBO;
}buffer;
uint16_t* tiles;
uint16_t ran16(uint16_t cap)
{
	uint16_t ret;
	_rdrand16_step(&ret);
	return ret % cap;
}
buffer CreateTile(float size)
{
	float vert[] = { 0,0,0,size, size,0,0,size, size,0,size,size };
	unsigned int vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	return { vao,vbo };
}
void drawTile(buffer id)
{
	glBindVertexArray(id.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void markAsVisible(uint16_t* cell)
{
	(*cell) = 1;
}
void render(buffer id, GLFWwindow *win, Shader *sh, uint32_t mapSize)
{
	for (uint32_t y = 0;y<mapSize;y++)
	{
		for (uint32_t x = 0; x < mapSize; x++)
		{
			uint16_t color = tiles[x + (y * widht)];
			if (color % 32768 != 0)
			{
				glm::mat4 position = glm::mat4(1.0f);
				sh->use();
				position = glm::translate(position, glm::vec3(((float)x * tileSize) - 1.0f, (-1.0f * (((float)y * tileSize) - 1.0f)) - tileSize, 0.0f));
				sh->setMat4("pos", position);
				glm::vec3 colour = { color,color,color};
				sh->setVec3("atex", colour);
				glBindVertexArray(id.VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
	}
}
uint16_t waveFunc(uint16_t right, uint16_t left, uint16_t down, uint16_t up)
{

	uint16_t xpp = (left+right+up+down)% 32768;
	xpp += 32768;
	return xpp;
}
uint32_t translateScreenToMapX(double x)
{
	return x/(1000 / widht) ;
}
uint32_t translateScreenToMapY(double y)
{
	return y/(1000/widht);
}
uint16_t getBlock(double x, double y)
{
	uint32_t xi = translateScreenToMapX(x);
	uint32_t yi = translateScreenToMapY(y);
	if (yi<0 || xi<0 || xi>(widht - 1) || yi>(widht - 1))
		return 0;
	return tiles[xi + (yi * widht)];
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		printf("%d\n", getBlock(x, y));
	}
}
void freeGPU(buffer id)
{
	glDeleteVertexArrays(1,&id.VAO);
	glDeleteBuffers(1,&id.VBO);
}
int main(int argc, char** argv)
{
	uint32_t sides[4] = { 1,-1,widht,-widht };
	srand(time(NULL));
	tiles =(uint16_t*)malloc((widht) * (widht ) *2);
	memset(tiles, 0, (widht ) * (widht) *2);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "xppp", NULL, NULL);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	buffer xp = CreateTile(tileSize);
	glClearColor(0, 0, 0, 0);
	Shader shader = Shader("vt.txt", "fr.txt");
	std::vector<uint32_t> cellStack;
	cellStack.push_back(0);
	markAsVisible(tiles);
	while(cellStack.size())
	{
		//main loop

		uint32_t currentCell = cellStack.back(); 
		cellStack.pop_back();
		uint16_t s[4] = { 1 };
		if (((currentCell + 2) % widht))
			s[0] = (currentCell + 2);
		if ((currentCell % widht))
			s[1] = (currentCell - 2);
		if (((currentCell + (widht * 2)))<(widht*widht))
			s[2] = (currentCell + (widht*2));
		if (int32_t(currentCell - (widht * 2))>0)
			s[3] = (currentCell - (widht * 2));
		std::vector<char> toChoose;
		for (char i = 0; i < 4; i++)
			if (!tiles[s[i]])
				toChoose.push_back(i);
		if (toChoose.size())
		{
			cellStack.push_back(currentCell);
			char randIndex = ran16(toChoose.size());
			uint32_t newCurrent = s[toChoose[randIndex]];
			markAsVisible(tiles + newCurrent);
			cellStack.push_back(newCurrent);
			markAsVisible(&tiles[(currentCell + sides[toChoose[randIndex]])]);
		}
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		render(xp, window, &shader, widht);
		glfwSwapBuffers(window);
	}
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		render(xp, window, &shader, widht);
		glfwSwapBuffers(window);
	}
	freeGPU(xp);
	free(tiles);
}
/*while(!tiles || !tiles[widht - 1] || !tiles[widht * (widht - 1)] || !tiles[(widht * widht) - 1])
	{
		//main loop
		uint32_t x = ran16(widht);
		uint32_t y = ran16(widht);
		uint32_t ran = ran16(chance);
		if (ran > 5000 && tiles[uint32_t(x + (y * widht))] == 0)
			uint16_t ran = ran16(32768);
		tiles[uint32_t(x + (y * widht))] = ran;
		for (uint32_t x1 = 0; x1 < widht; x1++)
		{
			for (uint32_t y1 = 0; y1 < widht; y1++)
			{
				uint16_t s[4] = { 0 };
				s[0] = tiles[uint32_t((x1 + 1) + (y1 * widht))] % (uint16_t)32768;
				if (x1 > 0)
					s[1] = tiles[uint32_t((x1 - 1) + (y1 * widht))] % (uint16_t)32768;
				s[2] = tiles[uint32_t(x1 + ((y1 + 1) * widht))] % (uint16_t)32768;
				if (y1 > 0)
					s[3] = tiles[uint32_t(x1 + ((y1 - 1) * widht))] % (uint16_t)32768;
				if ((s[0] + s[1] + s[2] + s[3]) == 0)
					continue;
				ran = rand()%chance;
				if (ran > 7000)
				{
					uint16_t* tmp = &tiles[uint32_t(x1 + (y1 * widht))];
					if (*tmp == 0)
						*tmp = waveFunc(s[0], s[1], s[2], s[3]);
				}
			}
		}
	}
	while (!*/