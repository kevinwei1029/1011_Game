#include <iostream>
#include "TCPIP.h"
#include "conio.h"
#include <process.h>
#include <windows.h>

//  online backgammon game with 2 players on the network
#define W 40  // width
#define H 20  // height
int T[W][H] = { 0 }; // 0:empty, 1:O, 2:X
int ldx, ldy;
char mat[W][H];

TCPIP* lp;
SOCKET SSock, CSock, Sock;

void gotoxy(int xpos, int ypos, int color)
{
	COORD scrn;
	HANDLE hOuput = GetStdHandle(STD_OUTPUT_HANDLE);
	scrn.X = xpos; scrn.Y = ypos;
	SetConsoleCursorPosition(hOuput, scrn);
	SetConsoleTextAttribute(hOuput, color);
}

void SetColor(int color = 7)
{
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

// 檢查某個方向是否有連續的 n 個相同的棋子
bool check_direction(int cx, int cy, int dx, int dy, int n, char p) {
	int count = 1;  // 計數，包括 (cx, cy) 自己
	// 向一個方向檢查
	for (int i = 1; i < n; i++) {
		int nx = cx + i * dx;
		int ny = cy + i * dy;
		if (nx < 0 || nx >= W || ny < 0 || ny >= H || mat[nx][ny] != p) {
			break;
		}
		count++;
	}
	// 向相反方向檢查
	for (int i = 1; i < n; i++) {
		int nx = cx - i * dx;
		int ny = cy - i * dy;
		if (nx < 0 || nx >= W || ny < 0 || ny >= H || mat[nx][ny] != p) {
			break;
		}
		count++;
	}
	return count >= n;
}

// 檢查 cx, cy 位置的棋子是否在四個方向上連成長度為 n 的直線
bool ifline(int cx, int cy, int n, char p) {
	// 垂直檢查 (上下)
	if (check_direction(cx, cy, 2, 0, n, p)) {
		ldx = 2;
		ldy = 0;
		return true;
	}

	// 水平檢查 (左右)
	if (check_direction(cx, cy, 0, 2, n, p)) {
		ldx = 0;
		ldy = 2;
		return true;
	}

	// 左上到右下對角線檢查
	if (check_direction(cx, cy, 2, 2, n, p)) {
		ldx = 2;
		ldy = 2;
		return true;
	}

	// 左下到右上對角線檢查
	if (check_direction(cx, cy, 2, -2, n, p)) {
		ldx = 2;
		ldy = -2;
		return true;
	}

	return false;  // 如果沒有發現任何連續 n 個棋子的情況，返回 false
}

void lose4(int cx, int cy, char c) {
	int x = cx, y = cy;
	while (mat[x][y] == c) {
		x += ldx;
		y += ldy;
	}
	if (mat[x][y] == ' ') {
		x -= ldx;
		y -= ldy;
		while (mat[x][y] == c) {
			x -= ldx;
			y -= ldy;
		}
		if (mat[x][y] == ' ') {
			gotoxy(0, H + 5, 7);
			printf("%c is gonna win", c);
		}
	}
}

void line3(int cx, int cy, char c) {
	int x = cx, y = cy;
	while (mat[x][y] == c) {
		x += ldx;
		y += ldy;
	}
	if (mat[x][y] == ' ') {
		x -= ldx;
		y -= ldy;
		while (mat[x][y] == c) {
			x -= ldx;
			y -= ldy;
		}
		if (mat[x][y] == ' ') {
			gotoxy(0, H + 5, 7);
			printf("%c got a line3", c);
		}
	}
}

void printmat() {
	system("cls");
	int x, y;
	for (y = 0; y < H; y++)
	{
		for (x = 0; x < W; x++)
		{
			gotoxy(x, y, 7);
			if ((mat[x][y] == '+') || (mat[x][y] == '|') || (mat[x][y] == '-')) SetColor(6);
			std::cout << mat[x][y];
		}
	}
}

void SFun(void* p) {
	int i, Len = sizeof(sockaddr), x1, y1;
	char S1[2000];
	sockaddr Addr;
	SOCKET Server_Sock = (SOCKET)p;

	while (1) {
		Sock = accept(SSock, &Addr, &Len);
		gotoxy(0, H + 1, 4);
		printf("accept client");
		do
		{
			i = recv(Sock, S1, sizeof(S1), 0);
			if (i > 0)
			{
				S1[i] = 0;
				sscanf_s(S1, "(%d,%d)", &x1, &y1);
				//gotoxy(x1, y1, 4);
				//printf("X");
				T[x1][y1] = 2;
				mat[x1][y1] = 'X';
				printmat();
				if (ifline(x1, y1, 5, 'X')) {
					gotoxy(0, H + 1, 12);
					printf("X Win");
				}
				else if (ifline(x1, y1, 4, 'X')) {
					lose4(x1, y1, 'X');
				}
				else if (ifline(x1, y1, 3, 'X')) {
					line3(x1, y1, 'X');
				}
			}
		} while (i > 0);
		gotoxy(0, H + 1, 7);
		printf("client disconnected\nwaiting for new client......");
	}
}

void CFun(void* p) {
	int i, Len = sizeof(sockaddr), x1, y1;
	char S1[2000];
	sockaddr Addr;
	//SOCKET Server_Sock = (SOCKET)p;

	while (1) {
		i = recv(CSock, S1, sizeof(S1), 0);
		if (i > 0)
		{
			S1[i] = 0;
			sscanf_s(S1, "(%d,%d)", &x1, &y1);
			T[x1][y1] = 2;
			mat[x1][y1] = 'X';
			printmat();
			gotoxy(x1, y1, 7);
			printf("X");
			if (ifline(x1, y1, 5, 'X')) {
				gotoxy(0, H + 1, 12);
				printf("X Win");
			}
			else if (ifline(x1, y1, 4, 'X')) {
				lose4(x1, y1, 'X');
			}
			else if (ifline(x1, y1, 3, 'X')) {
				line3(x1, y1, 'X');
			}
		}
	}
}

int main() {
	memset(T, 0, sizeof(T));
	memset(mat, 0, sizeof(mat));
	// 3. 啟動網路連線、雙人對戰 server port = 6000
	lp = new TCPIP();
	lp->Start_TCP_Server(&SSock, 6000);
	_beginthread(SFun, 0, (void*)SSock);

	// 1. 設計遊戲框
	int x, y;
	x = y = ldx = ldy = 0;
	for (y = 0; y < H; y++)
	{
		for (x = 0; x < W; x++)
		{
			gotoxy(x, y, 6);
			if (x % 2 && y % 2) mat[x][y] = '+';
			else if (x % 2) 	mat[x][y] = '|';
			else if (y % 2) 	mat[x][y] = '-';
			else				mat[x][y] = ' ';

			std::cout << mat[x][y];
		}
	}
	
	char c = 0, IP[100], S1[2000];
	int cx, cy;
	cx = (W / 2) - (W / 2) % 2;
	cy = (H / 2) - (H / 2) % 2;
	gotoxy(cx, cy, 15);
	do 
	{
		// 2. 控制鍵盤下棋
		c = _getch();
		switch (c)
		{
		case 0x48: cy -= 2; break;		// up
		case 0x50: cy += 2; break;		// down
		case 0x4B: cx -= 2; break;		// left
		case 0x4D: cx += 2; break;		// right
		case 0x0d:						// enter
			//printf("O");
			T[cx][cy] = 1;
			mat[cx][cy] = 'O';
			printmat();
			ldx = ldy = 0;
			if (ifline(cx, cy, 5, 'O')) {
				gotoxy(0, H + 1, 12);
				printf("%c Win", 'O');
			}
			else if (ifline(cx, cy, 4, 'O')) {
				lose4(cx, cy, 'O');
			}
			else if (ifline(cx, cy, 3, 'O')) {
				line3(cx, cy, 'O');
			}
			/*
			for (int i = 0; i < W; i++) {
				gotoxy(W + 2, i, 7);
				for (int j = 0; j < H; j++) {
					if (i == cx && j == cy) SetColor(27);
					else SetColor(7);
					std::cout << T[i][j];
				}
			}*/
			/*for (int d = 0; d < 8; d++) {
				if (ifline(cx, cy, 5, d, 1)) {
					gotoxy(0, H + 1, 12);
					printf("O Win");
					gotoxy(0, H + 2, 7);
					std::cout << "d = " << d;
					break;
				}
			}*/
			sprintf_s(S1, sizeof(S1), "(%d,%d)", cx, cy);
			send(Sock, S1, strlen(S1), 0);
			break;
		case 0x20: printf(" "); break;	// space
		case 'c':						// client
			gotoxy(0, H, 15);
			printf("IP=");
			scanf_s("%s", IP, sizeof(IP));
			lp->Start_TCP_Client(&CSock, IP, 0, 6000);
			Sock = CSock;
			// connect, recv => thread
			_beginthread(CFun, 0, (void*)CSock);
			break;
		default: break;
		}
		if (cx < 0) cx = 0;
		else if (cx >= W) cx -= 2;
		if (cy < 0) cy = 0;
		else if (cy >= H) cy -= 2;
		gotoxy(cx, cy, 15);
		//Sleep(500);

		// 4. 偵測活三、絕四

		// 5. 搜尋雙活三、活三+絕四最佳位置

	} while (c != 'q');
	gotoxy(0, H + 1, 4);
	printf("quit..........");
	gotoxy(0, H + 2, 7);
	return 0;
}

void Sort() {
	int x, y, z, n;
	for (y = 0; y < H; y++)
	{
		for (x = 0; x < W; x++)
		{
			for (n = 0, z = 0; z < 10; z += 2)
			{
				if (n >= 5)
				{
					gotoxy(0, H + 1, 12);
					printf("5-Star");
				}
			}
		}
	}
	//  1. 搜尋O, X
	//  2. 搜尋四個方向：直、橫、左斜、右斜
	//  3. 搜尋活三、絕四
	//  4. 搜尋雙活三、活三+絕四最佳位置
}
