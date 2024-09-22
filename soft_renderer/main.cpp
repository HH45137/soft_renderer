#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

#define RENDERER_SIZE_WIDTH 512
#define RENDERER_SIZE_HEIGHT 512
#define RENDERER_CENTER_X RENDERER_SIZE_WIDTH / 2
#define RENDERER_CENTER_Y RENDERER_SIZE_HEIGHT / 2

int gen_random(int min, int max);

int gen_random(int min, int max, int seed);

void draw_pixel(int x, int y, int r, int g, int b);

void draw_line_dda(int x_start, int y_start, int x_end, int y_end, int r, int g, int b);


int gen_random(int min, int max)
{
	srand(time(0));
	int rand_num = min + rand() % (max - min + 1);
	return rand_num;
}

int gen_random(int min, int max, int seed)
{
	srand(seed);
	int rand_num = min + rand() % (max - min + 1);
	return rand_num;
}

void draw_pixel(int x, int y, int r, int g, int b)
{
	putpixel(x, y, RGB(r, g, b));
}

void draw_line_dda(int x_start, int y_start, int x_end, int y_end, int r, int g, int b)
{
	float dx = (float)x_end - x_start;
	float dy = (float)y_end - y_start;

	float k = dy / dx;

	// 取得步数最多的一个轴（x或y）
	int steps = abs(y_end - y_start);
	if (fabs(dx) > fabs(dy)) { steps = abs(x_end - x_start); }

	// 初始化起始点
	float x = x_start;
	float y = y_start;

	// 计算每一step需要走多远
	float xinc = dx / steps;
	float yinc = dy / steps;

	for (int i = 0; i < steps; i++)
	{
		x += xinc;
		y += yinc;

		draw_pixel(x, (int)(y + 0.5), r, g, b);
	}

	return;
}

void draw_mesh_wireframe(const char* obj_file_path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, obj_file_path);

	if (!err.empty())
	{
		std::cout << err << "\n";
	}

	if (!result)
	{
		exit(1);
	}

	for (size_t s = 0; s < shapes.size(); s++)
	{
		size_t index_offset = 0;
		glm::vec3 v_cur{}, v_pre1{}, v_pre2{};
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				v_pre2 = v_pre1;
				v_pre1 = v_cur;
				v_cur = { vx,vy,vz };
			}
			index_offset += fv;

			int x0 = (v_pre1.x + 1.) * RENDERER_CENTER_X;
			int y0 = (v_pre1.y + 1.) * RENDERER_CENTER_Y - RENDERER_CENTER_Y;
			int x1 = (v_cur.x + 1.) * RENDERER_CENTER_X;
			int y1 = (v_cur.y + 1.) * RENDERER_CENTER_Y - RENDERER_CENTER_Y;
			int x2 = (v_pre2.x + 1.) * RENDERER_CENTER_X;
			int y2 = (v_pre2.y + 1.) * RENDERER_CENTER_Y - RENDERER_CENTER_Y;

			draw_line_dda(x0, y0, x1, y1, 0, 0, 255);
			draw_line_dda(x2, y2, x1, y1, 255, 0, 0);
			draw_line_dda(x2, y2, x0, y0, 0, 255, 0);
		}
	}
}

int main()
{
	initgraph(RENDERER_SIZE_WIDTH, RENDERER_SIZE_HEIGHT);

	// 设置坐标系
	setorigin(0, RENDERER_SIZE_HEIGHT);
	setaspectratio(1, -1);

	draw_line_dda(68, 66, 123, 300, 255, 0, 0);
	draw_line_dda(68, 66, 14, 300, 0, 255, 0);
	draw_line_dda(68, 66, 177, 10, 0, 0, 255);
	draw_line_dda(68, 66, 14, 10, 255, 0, 255);

	draw_mesh_wireframe("../assets/wukong_mesh.obj");

	_getch();
	closegraph();

	return 0;
}
