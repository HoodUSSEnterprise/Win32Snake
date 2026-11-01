#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"

#define CELL_SIZE     20
#define X_SIZE        40
#define Y_SIZE        30
#define WINDOW_WIDTH  (CELL_SIZE * X_SIZE)
#define WINDOW_HEIGHT (CELL_SIZE * Y_SIZE)

// dir
#define LEFT  0
#define RIGHT 1
#define UP    2
#define DOWN  3

int dir[4][2] = {
	{-1,  0 },  // LEFT
	{ 1,  0 },  // RIGHT
	{ 0, -1 },  // UP
	{ 0,  1 }   // DOWN
};

// define some basic types
typedef struct Point
{
	int x;
	int y;
} Point;

typedef struct Snake
{
	Point* body;
	int len;
	int dir;
} Snake;

typedef struct Food
{
	Point pos;
} Food;

// some global varibles
Snake snake;
Food food;
Point snake_tail;
HDC snake_dc;
HDC food_dc;
HBITMAP snake_bitmap;
HBITMAP food_bitmap;
int init_mem_dc;
int next_dir;
int can_change_dir;

BOOL IsOnSnake(int x, int y)
{
	for (int i = 0; i < snake.len; i++)
	{
		if (snake.body[i].x == x && snake.body[i].y == y)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CreateFood()
{
	do
	{
		food.pos.x = rand() % X_SIZE;
		food.pos.y = rand() % Y_SIZE;
	} while (IsOnSnake(food.pos.x, food.pos.y));
}

void InitGame(HINSTANCE hInstance)
{
	// init snake
	snake.body = (Point*)calloc(X_SIZE * Y_SIZE, sizeof(Point));
	snake.len = 3;
	snake.dir = RIGHT;
	next_dir = RIGHT;
	can_change_dir = 1;

	snake.body[0].x = X_SIZE / 2 + 1;
	snake.body[0].y = Y_SIZE / 2;
	snake.body[1].x = X_SIZE / 2;
	snake.body[1].y = Y_SIZE / 2;
	snake.body[2].x = X_SIZE / 2 - 1;
	snake.body[2].y = Y_SIZE / 2;

	// init food
	do
	{
		food.pos.x = rand() % X_SIZE;
		food.pos.y = rand() % Y_SIZE;
	} while (IsOnSnake(food.pos.x, food.pos.y));

	// init bitmap
	snake_bitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SNAKE));
	food_bitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FOOD));
}

void InitMemDC(HDC hdc)
{
	// init memory dc
	snake_dc = CreateCompatibleDC(hdc);
	food_dc = CreateCompatibleDC(hdc);

	SelectObject(snake_dc, snake_bitmap);
	SelectObject(food_dc, food_bitmap);

	init_mem_dc = 1;
}

void DrawSnake(HDC hdc)
{
	for (int i = 0; i < snake.len; i++)
	{
		int x = snake.body[i].x * CELL_SIZE;
		int y = snake.body[i].y * CELL_SIZE;
		StretchBlt(hdc, x, y, CELL_SIZE, CELL_SIZE, snake_dc, 0, 0, 48, 48, SRCCOPY);
	}
}

void DrawFood(HDC hdc)
{
	int x = food.pos.x * CELL_SIZE;
	int y = food.pos.y * CELL_SIZE;
	StretchBlt(hdc, x, y, CELL_SIZE, CELL_SIZE, food_dc, 0, 0, 48, 48, SRCCOPY);
}

void DrawGameBoard(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	if (init_mem_dc == 0)
	{
		InitMemDC(hdc);
	}

	DrawSnake(hdc);
	DrawFood(hdc);

	EndPaint(hwnd, &ps);
}

void SnakeMove()
{
	snake.dir = next_dir;

	snake_tail.x = snake.body[snake.len - 1].x;
	snake_tail.y = snake.body[snake.len - 1].y;

	for (int i = snake.len - 1; i >= 1; i--)
	{
		snake.body[i].x = snake.body[i - 1].x;
		snake.body[i].y = snake.body[i - 1].y;
	}

	snake.body[0].x += dir[snake.dir][0];
	snake.body[0].y += dir[snake.dir][1];
}

BOOL CheckIfOver()
{
	// hit wall
	if (snake.body[0].x < 0 || snake.body[0].x >= X_SIZE || snake.body[0].y < 0 || snake.body[0].y >= Y_SIZE)
	{
		return TRUE;
	}

	// hit self
	for (int i = 1; i < snake.len; i++)
	{
		if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CheckIfEatFood()
{
	if (snake.body[0].x == food.pos.x && snake.body[0].y == food.pos.y)
	{
		snake.len++;
		snake.body[snake.len - 1].x = snake_tail.x;
		snake.body[snake.len - 1].y = snake_tail.y;
		CreateFood();
	}
}

void KeyBoradEvent(WPARAM wParam)
{
	if (can_change_dir == 0)
	{
		return;
	}

	switch (wParam)
	{
	case VK_LEFT:
	case 'A':
	case 'a':
		if (snake.dir != RIGHT)
		{
			next_dir = LEFT;
			can_change_dir = 0;
		}
		break;
	case VK_RIGHT:
	case 'D':
	case 'd':
		if (snake.dir != LEFT)
		{
			next_dir = RIGHT;
			can_change_dir = 0;
		}
		break;
	case VK_UP:
	case 'W':
	case 'w':
		if (snake.dir != DOWN)
		{
			next_dir = UP;
			can_change_dir = 0;
		}
		break;
	case VK_DOWN:
	case 'S':
	case 's':
		if (snake.dir != UP)
		{
			next_dir = DOWN;
			can_change_dir = 0;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		SetTimer(hwnd, 1, 200, NULL);
		break;
	case WM_TIMER:
		SnakeMove();
		can_change_dir = 1;
		if (CheckIfOver())
		{
			KillTimer(hwnd, 1);
			MessageBox(NULL, L"Game Over", L"Snake", MB_OK);
			PostQuitMessage(0);
			break;
		}

		CheckIfEatFood();

		InvalidateRect(hwnd, NULL, TRUE);

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		KeyBoradEvent(wParam);
		break;
	case WM_PAINT:
		DrawGameBoard(hwnd);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// srand seed
	srand((unsigned int)time(NULL));

	// window class
	WNDCLASS wc = { 0 };
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"Snake Game";
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	// register class
	RegisterClass(&wc);

	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	// get handle of window
	HWND hwnd = CreateWindow(wc.lpszClassName, L"Snake", WS_OVERLAPPEDWINDOW, 100, 100,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

	InitGame(hInstance);

	// show and update window
	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	// message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}