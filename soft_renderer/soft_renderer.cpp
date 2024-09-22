#include <iostream>
#include <graphics.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>

#define RENDERER_SIZE_WIDTH 640
#define RENDERER_SIZE_HEIGHT 480
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

	_getch();
	closegraph();

	return 0;
}
