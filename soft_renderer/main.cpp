﻿#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

#define RENDERER_SIZE_WIDTH 1024
#define RENDERER_SIZE_HEIGHT 1024
#define RENDERER_CENTER_X RENDERER_SIZE_WIDTH / 2
#define RENDERER_CENTER_Y RENDERER_SIZE_HEIGHT / 2


struct global_var_s
{
	float* zbuffer = nullptr;
} global_var;


int gen_random(int min, int max);

int gen_random(int min, int max, int seed);

void draw_pixel(int x, int y, int r, int g, int b);

void draw_line_dda(int x_start, int y_start, int x_end, int y_end, int r, int g, int b);

void draw_triangle_line_sweeping(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, int r, int g, int b);

void draw_triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, int r, int g, int b);


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

void draw_triangle_line_sweeping(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, int r, int g, int b)
{
	if (v0.y > v1.y) { std::swap(v0, v1); }
	if (v0.y > v2.y) { std::swap(v0, v2); }
	if (v1.y > v2.y) { std::swap(v1, v2); }
	int total_height = v2.y - v0.y;

	for (int i = 0; i < total_height; i++) {
		for (int y = v0.y; y < v1.y; y++) {
			bool second_half = i > v1.y - v0.y || v1.y == v0.y;
			int segment_height = second_half ? v2.y - v1.y : v1.y - v0.y;
			float alpha = (float)i / total_height;
			float beta = (float)(i - (second_half ? v1.y - v0.y : 0)) / segment_height;
			glm::ivec2 A = v0 + (v2 - v0) * alpha;
			glm::ivec2 B = second_half ? v1 + (v2 - v1) * beta : v0 + (v1 - v0) * beta;
			if (A.x > B.x) { std::swap(A, B); }
			for (int j = A.x; j <= B.x; j++)
			{
				int final_x = j, final_y = v0.y + i;
				draw_pixel(final_x, final_y, r, g, b);
			}
		}
	}

}

glm::vec3 barycentric(glm::ivec2* pts, glm::ivec2 P)
{
	glm::fvec3 u = glm::cross(
		glm::fvec3(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]),
		glm::fvec3(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1])
	);

	if (std::abs(u.z) < 1) return glm::fvec3(-1, 1, 1);

	return glm::fvec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void draw_triangle_barycentric_coord(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, int r, int g, int b)
{
	glm::ivec2 points[3] = { {v0.x,v0.y},{v1.x,v1.y},{v2.x,v2.y} };

	glm::ivec2 bboxmin{ RENDERER_SIZE_WIDTH - 1,RENDERER_SIZE_HEIGHT - 1 };
	glm::ivec2 bboxmax{ 0,0 };
	glm::ivec2 clamp{ RENDERER_SIZE_WIDTH - 1,RENDERER_SIZE_HEIGHT - 1 };

	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = glm::max<int>(0, glm::min<int>(bboxmin.x, points[i].x));
		bboxmin.y = glm::max<int>(0, glm::min<int>(bboxmin.y, points[i].y));

		bboxmax.x = glm::min<int>(clamp.x, glm::max<int>(bboxmax.x, points[i].x));
		bboxmax.y = glm::min<int>(clamp.y, glm::max<int>(bboxmax.y, points[i].y));
	}
	glm::ivec2 P{};
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			glm::fvec3 bc_screen = barycentric(points, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) { continue; }
			draw_pixel(P.x, P.y, r, g, b);
		}
	}
}

void draw_triangle_barycentric_coord_depth(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, int r, int g, int b)
{
	glm::ivec2 points[3] = { {v0.x,v0.y},{v1.x,v1.y},{v2.x,v2.y} };

	glm::ivec2 bboxmin{ RENDERER_SIZE_WIDTH - 1,RENDERER_SIZE_HEIGHT - 1 };
	glm::ivec2 bboxmax{ 0,0 };
	glm::ivec2 clamp{ RENDERER_SIZE_WIDTH - 1,RENDERER_SIZE_HEIGHT - 1 };

	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = glm::max<int>(0, glm::min<int>(bboxmin.x, points[i].x));
		bboxmin.y = glm::max<int>(0, glm::min<int>(bboxmin.y, points[i].y));

		bboxmax.x = glm::min<int>(clamp.x, glm::max<int>(bboxmax.x, points[i].x));
		bboxmax.y = glm::min<int>(clamp.y, glm::max<int>(bboxmax.y, points[i].y));
	}
	glm::vec3 P{};
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			glm::fvec3 bc_screen = barycentric(points, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) { continue; }

			// Generate Z-Buffer
			{
				if (global_var.zbuffer == nullptr)
				{
					global_var.zbuffer = new float[RENDERER_SIZE_WIDTH * RENDERER_SIZE_HEIGHT] {1.0f};
				}

				float z = glm::normalize(v0).z;
				int idx = P.x + P.y * RENDERER_SIZE_WIDTH;
				if (global_var.zbuffer[idx] < z)
				{
					global_var.zbuffer[idx] = z;
					draw_pixel(P.x, P.y, r, g, b);
				}
				r = global_var.zbuffer[idx] * 255;
				g = global_var.zbuffer[idx] * 255;
				b = global_var.zbuffer[idx] * 255;
				//draw_pixel(P.x, P.y, r, g, b);
			}

		}
	}
}

void draw_mesh(const char* obj_file_path, int r, int g, int b)
{
	glm::vec3 light_dir(-1, 0, -1);

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

				// 每个三角形的每个顶点的屏幕空间坐标
				std::vector<glm::vec3> v_screen{
					glm::ivec3 {
						(v_pre1.x + 1.) * RENDERER_CENTER_X,
						(v_pre1.y + 1.) * RENDERER_CENTER_Y,
						(v_pre1.z + 1.) * RENDERER_CENTER_Y
					},
					glm::ivec3 {
						(v_cur.x + 1.) * RENDERER_CENTER_X,
						(v_cur.y + 1.) * RENDERER_CENTER_Y,
						(v_cur.z + 1.) * RENDERER_CENTER_Y
					},
					glm::ivec3 {
						(v_pre2.x + 1.) * RENDERER_CENTER_X,
						(v_pre2.y + 1.) * RENDERER_CENTER_Y,
						(v_pre2.z + 1.) * RENDERER_CENTER_Y
					}
				};

				// 每个三角形的每个顶点的世界空间坐标
				std::vector<glm::vec3> v_world{
					v_pre1,
					v_cur,
					v_pre2
				};

				// 计算着色
				{
					glm::vec3 normal = glm::cross(v_world[2] - v_world[0], (v_world[1] - v_world[0]));
					normal = glm::normalize(normal);
					float intensity = glm::dot(normal, light_dir);
					if (intensity > 0) {
						r = intensity * 255;
						g = intensity * 255;
						b = intensity * 255;
					}
				}

				//draw_triangle_line_sweeping(v_screen[0], v_screen[1], v_screen[2], r, g, b);
				//draw_triangle_barycentric_coord(v_screen[0], v_screen[1], v_screen[2], r, g, b);
				draw_triangle_barycentric_coord_depth(v_screen[0], v_screen[1], v_screen[2], r, g, b);
			}
		}
	}
}


int main()
{
	initgraph(RENDERER_SIZE_WIDTH, RENDERER_SIZE_HEIGHT);

	// 设置坐标系
	setorigin(0, RENDERER_SIZE_HEIGHT);
	setaspectratio(1, -1);

	//draw_mesh("../assets/wukong_mesh.obj", 255, 0, 0);
	draw_mesh("../assets/african_head.obj", 255, 255, 255);

	draw_triangle_barycentric_coord(glm::vec3(10, 70, 0), glm::vec3(50, 160, 0), glm::vec3(70, 90, 0), 255, 0, 0);
	draw_triangle_barycentric_coord(glm::vec3(180, 50, 0), glm::vec3(150, 1, 0), glm::vec3(70, 180, 0), 0, 255, 0);
	draw_triangle_barycentric_coord(glm::vec3(180, 150, 0), glm::vec3(120, 160, 0), glm::vec3(130, 180, 0), 255, 0, 255);

	_getch();
	closegraph();

	return 0;
}
