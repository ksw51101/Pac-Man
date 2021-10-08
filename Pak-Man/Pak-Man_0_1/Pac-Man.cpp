#include <windows.h>
#include <TCHAR.H>
#include <Math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <MMSystem.h>
#include "resource.h"

#define SIZE 25
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
POINT CenterPoint(RECT &r);	//사각형의 중앙좌표
POINT Find(int); //해당 캐릭터를 찾는 함수
BOOL HitTest(RECT&, RECT&); //사각형끼리 만났는지 확인
BOOL Game_Over(RECT&, RECT&); //팩맨과 유령이 만났는지 확인

void Ready_To_Start(int); //시작준비 세팅
void Dir_Pac(int);	//팩맨의 방향 결정
void Dir_Ghost(RECT&, RECT&, int); //유령의 방향 결정
void Move(int); //해당 캐릭터 움직이기

static POINT Oldp_Pac, Oldp_Ghost[3], Newp_Pac, Newp_Ghost[3];	//위치비교용 POINT
static RECT Ad_Pac_Position, Ad_Ghost_Position[3]; //맵에서 팩맨위치가 바뀌고나서 RECT
static RECT scoreR, explainR; //점수화 설명 출력용 RECT
static POINT Vir_Pac_Position, Vir_Ghost_Position[3];	//팩맨이 이동할 위치에 미리 가있는 좌표
static int score, life, Pac_Flag, Ghost_Flag[3], index;	//점수, 라이프, 팩맨과 유령이동용 flag, 꼬리움직이용 index
static int Save_Pac_Dir, Save_Ghost_Dir[3], Pac_dir, Ghost_dir[3]; //팩맨과 유령 방향
static POINT CanPac, CanGhost[3]; //갈수있으면 팩맨좌표, 못가면 (-1,-1)
static bool isPower; //파워업 아이템 먹었는지

void DrawScreen(HDC); //화면 그리기
POINT Can_I_Go(int, int); //지정된 방향으로 갈 수 있는지

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = _T("Pac-Man");

HBITMAP hBit[36]; //비트맵 핸들
static HDC mem1dc, mem2dc; //더블버퍼링
static HBITMAP hBit1, oldBit1, oldBit2;
static RECT Pac, Ghost[3]; //팩맨과 유령 RECT

char OriginalMaze[31][29] = { //맵
	"/------------7/------------7",
	"|............|!............|",
	"|./__7./___7.|!./___7./__7.|",
	"|o|  !.|   !.|!.|   !.|  !o|",
	"|.L--J.L---J.LJ.L---J.L--J.|",
	"|..........................|",
	"|./__7./7./______7./7./__7.|",
	"|.L--J.|!.L--7/--J.|!.L--J.|",
	"|......|!....|!....|!......|",
	"L____7.|L__7 |! /__J!./____J",
	"#####!.|/--J LJ L--7!.|#####",
	"#####!.|!          |!.|#####",
	"#####!.|! /__==__7 |!.|#####",
	"-----J.LJ |      ! LJ.L-----",
	"      .   | #### !   .      ",
	"_____7./7 |      ! /7./_____",
	"#####!.|! L______J |!.|#####",
	"#####!.|!          |!.|#####",
	"#####!.|! /______7 |!.|#####",
	"/----J.LJ L--7/--J LJ.L----7",
	"|............|!............|",
	"|./__7./___7.|!./___7./__7.|",
	"|.L-7!.L---J.LJ.L---J.|/-J.|",
	"|o..|!........>.......|!..o|",
	"L_7.|!./7./______7./7.|!./_J",
	"/-J.LJ.|!.L--7/--J.|!.LJ.L-7",
	"|......|!....|!....|!......|",
	"|./____JL__7.|!./__JL____7.|",
	"|.L--------J.LJ.L--------J.|",
	"|..........................|",
	"L--------------------------J",
};
char GhostMaze[3][31][29] = { {
	"/------------7/------------7",
	"|            |!            |",
	"| /__7 /___7 |! /___7 /__7 |",
	"| |  ! |   ! |! |   ! |  ! |",
	"| L--J L---J LJ L---J L--J |",
	"|                          |",
	"| /__7 /7 /______7 /7 /__7 |",
	"| L--J |! L--7/--J |! L--J |",
	"|      |!    |!    |!      |",
	"L____7 |L__7 |! /__J! /____J",
	"#####! |/--J LJ L--7! |#####",
	"#####! |!     1    |! |#####",
	"#####! |! /__==__7 |! |#####",
	"-----J LJ |      ! LJ L-----",
	"          | #### !          ",
	"_____7 /7 |      ! /7 /_____",
	"#####! |! L______J |! |#####",
	"#####! |!          |! |#####",
	"#####! |! /______7 |! |#####",
	"/----J LJ L--7/--J LJ L----7",
	"|            |!            |",
	"| /__7 /___7 |! /___7 /__7 |",
	"| L-7! L---J LJ L---J |/-J |",
	"|   |!                |!   |",
	"L_7 |! /7 /______7 /7 |! /_J",
	"/-J LJ |! L--7/--J |! LJ L-7",
	"|      |!    |!    |!      |",
	"| /____JL__7 |! /__JL____7 |",
	"| L--------J LJ L--------J |",
	"|                          |",
	"L--------------------------J",
	},
	 {
		 "/------------7/------------7",
		 "|            |!            |",
		 "| /__7 /___7 |! /___7 /__7 |",
		 "| |  ! |   ! |! |   ! |  ! |",
		 "| L--J L---J LJ L---J L--J |",
		 "|      2                   |",
		 "| /__7 /7 /______7 /7 /__7 |",
		 "| L--J |! L--7/--J |! L--J |",
		 "|      |!    |!    |!      |",
		 "L____7 |L__7 |! /__J! /____J",
		 "#####! |/--J LJ L--7! |#####",
		 "#####! |!          |! |#####",
		 "#####! |! /__==__7 |! |#####",
		 "-----J LJ |      ! LJ L-----",
		 "          | #### !          ",
		 "_____7 /7 |      ! /7 /_____",
		 "#####! |! L______J |! |#####",
		 "#####! |!          |! |#####",
		 "#####! |! /______7 |! |#####",
		 "/----J LJ L--7/--J LJ L----7",
		 "|            |!            |",
		 "| /__7 /___7 |! /___7 /__7 |",
		 "| L-7! L---J LJ L---J |/-J |",
		 "|   |!                |!   |",
		 "L_7 |! /7 /______7 /7 |! /_J",
		 "/-J LJ |! L--7/--J |! LJ L-7",
		 "|      |!    |!    |!      |",
		 "| /____JL__7 |! /__JL____7 |",
		 "| L--------J LJ L--------J |",
		 "|                          |",
		 "L--------------------------J",
	 },
	  {
		  "/------------7/------------7",
		  "|            |!        3   |",
		  "| /__7 /___7 |! /___7 /__7 |",
		  "| |  ! |   ! |! |   ! |  ! |",
		  "| L--J L---J LJ L---J L--J |",
		  "|                          |",
		  "| /__7 /7 /______7 /7 /__7 |",
		  "| L--J |! L--7/--J |! L--J |",
		  "|      |!    |!    |!      |",
		  "L____7 |L__7 |! /__J! /____J",
		  "#####! |/--J LJ L--7! |#####",
		  "#####! |!          |! |#####",
		  "#####! |! /__==__7 |! |#####",
		  "-----J LJ |      ! LJ L-----",
		  "          | #### !          ",
		  "_____7 /7 |      ! /7 /_____",
		  "#####! |! L______J |! |#####",
		  "#####! |!          |! |#####",
		  "#####! |! /______7 |! |#####",
		  "/----J LJ L--7/--J LJ L----7",
		  "|            |!            |",
		  "| /__7 /___7 |! /___7 /__7 |",
		  "| L-7! L---J LJ L---J |/-J |",
		  "|   |!                |!   |",
		  "L_7 |! /7 /______7 /7 |! /_J",
		  "/-J LJ |! L--7/--J |! LJ L-7",
		  "|      |!    |!    |!      |",
		  "| /____JL__7 |! /__JL____7 |",
		  "| L--------J LJ L--------J |",
		  "|                          |",
		  "L--------------------------J",
	  }
};
char Maze[31][29] = {
	"/------------7/------------7",
	"|............|!............|",
	"|./__7./___7.|!./___7./__7.|",
	"|o|  !.|   !.|!.|   !.|  !o|",
	"|.L--J.L---J.LJ.L---J.L--J.|",
	"|..........................|",
	"|./__7./7./______7./7./__7.|",
	"|.L--J.|!.L--7/--J.|!.L--J.|",
	"|......|!....|!....|!......|",
	"L____7.|L__7 |! /__J!./____J",
	"#####!.|/--J LJ L--7!.|#####",
	"#####!.|!          |!.|#####",
	"#####!.|! /__==__7 |!.|#####",
	"-----J.LJ |      ! LJ.L-----",
	"      .   |      !   .      ",
	"_____7./7 |      ! /7./_____",
	"#####!.|! L______J |!.|#####",
	"#####!.|!          |!.|#####",
	"#####!.|! /______7 |!.|#####",
	"/----J.LJ L--7/--J LJ.L----7",
	"|............|!............|",
	"|./__7./___7.|!./___7./__7.|",
	"|.L-7!.L---J.LJ.L---J.|/-J.|",
	"|o..|!........>.......|!..o|",
	"L_7.|!./7./______7./7.|!./_J",
	"/-J.LJ.|!.L--7/--J.|!.LJ.L-7",
	"|......|!....|!....|!......|",
	"|./____JL__7.|!./__JL____7.|",
	"|.L--------J.LJ.L--------J.|",
	"|..........................|",
	"L--------------------------J",
};


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;
	WndClass.lpszMenuName = MAKEINTRESOURCE(139);
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	hWndMain = hWnd; 

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT crt;
	static bool start, index_Flag;	//게임시작여부, 꼬리움직이기용 index의 flag
	static RECT clientR, powerUp[4]; //client Rect, 파워업아이템
	static int initcookie, precookie; //점수계산용
	int i, j, k = 0, number; //반복문에 사용할 변수
	

	switch (iMessage) {
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 40006:
			MessageBox(hWnd, _T("MENU1"), _T("TEST MENU1-1"), MB_OK);
			break;
		case 40007:
			MessageBox(hWnd, _T("MENU1"), _T("TEST MENU1-2"), MB_OK);
			break;
		case 40008:
			PostQuitMessage(0);
			break;
		}

	case WM_CREATE:
		hdc = GetDC(hWnd);
		isPower = false;

		for (i = 0;i < 31; i++) { //파워업아이템 생성
			for (j = 0;j < 29;j++) {
				if (Maze[i][j] == 'o') {
					SetRect(&powerUp[k], j * SIZE + 10, i * SIZE + 10, j * SIZE + 15, i * SIZE + 15);
					k++;
				}
			}
		}
		Vir_Pac_Position.x = Find(-1).x;	//처음 팩맨좌표
		Vir_Pac_Position.y = Find(-1).y;
		for (i = 0;i < 3;i++) { //처음 Ghost세팅
			Ghost_dir[i] = 4;
			Vir_Ghost_Position[i].x = Find(i).x;	
			Vir_Ghost_Position[i].y = Find(i).y;
			Save_Ghost_Dir[i] = 0;
			Ghost_Flag[i] = false;
			SetRect(&Ghost[i], Find(i).y * SIZE, Find(i).x * SIZE, Find(i).y * SIZE + SIZE, Find(i).x * SIZE + SIZE);
		}

		life = 3;
		initcookie = 241;
		index = 0;
		Pac_dir = 0;
		Save_Pac_Dir = 0;
		score = 0;
		start = false;
		Pac_Flag = false;
		
		GetClientRect(hWnd, &clientR);		
		SetRect(&crt, 0, 0, 750, 850);
		SetRect(&scoreR, 50, 20, 250, 40);
		SetRect(&explainR, 450, 20, 800, 40);
		for (i = 0;i < 36;i++) { //비트맵 불러오기
			hBit[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(101 + i));
		}
		
		AdjustWindowRect(&crt, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
		SetRect(&Pac, Find(-1).y * SIZE, Find(-1).x * SIZE, Find(-1).y * SIZE + SIZE, Find(-1).x * SIZE + SIZE);
		SetWindowPos(hWnd, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top,
			SWP_NOMOVE | SWP_NOZORDER);
		hWndMain = hWnd;
		
		ReleaseDC(hWnd, hdc);
		return 0;

	case WM_TIMER:
		hdc = GetDC(hWnd);

		switch (wParam) {
		case 1: //팩맨용
			srand(time(NULL));
			CanPac = Can_I_Go(Pac_dir, -1);	//갈수있는지(못가면 (-1,-1)리턴)
			

			if (start == false) {	//시작한걸로 변경
				Ad_Pac_Position = Pac;
				start = true;
			}
			if (Save_Pac_Dir != 0 && Can_I_Go(Save_Pac_Dir, -1).x != -1) {	//방향키눌렀는데 막혀서 못갔었고 지금은 갈수있음
				Pac_dir = Save_Pac_Dir;	//저장된 방향으로 변경
				Save_Pac_Dir = 0;
			}

			precookie = 0;

			for (i = 0;i < 31;i++) {	//쿠키개수 세서 점수환산
				for (j = 0;j < 29;j++) {
					if (Maze[i][j] == '.') precookie++;
				}
			}
			if (precookie == 0) Ready_To_Start(1);

			score = (initcookie - precookie) * 10;

			Move(-1); //이동
			
			for (i = 0;i < 4;i++) {  //파워업 아이템 먹음
				if (HitTest(Pac, powerUp[i]) == true) {
					SetRect(&powerUp[i], NULL, NULL, NULL, NULL);
					SetTimer(hWnd, 2, 5000, NULL);
					isPower = true;
				}
			}

			if (Find(-1).x == 14 && Find(-1).y == 0) { //가운데 통로로 이동
				Maze[Find(-1).x][Find(-1).y] = ' ';
				Maze[14][26] = '>';

				Vir_Pac_Position.x = Find(-1).x;	
				Vir_Pac_Position.y = Find(-1).y;

				SetRect(&Pac, Find(-1).y * SIZE, Find(-1).x * SIZE, Find(-1).y * SIZE + SIZE, Find(-1).x * SIZE + SIZE);
				Oldp_Pac.x = Pac.left;
				Oldp_Pac.y = Pac.top;
			}
			else if (Find(-1).x == 14 && Find(-1).y == 27) {
				Maze[Find(-1).x][Find(-1).y] = ' ';
				Maze[14][1] = '>';

				Vir_Pac_Position.x = Find(-1).x;	
				Vir_Pac_Position.y = Find(-1).y;

				SetRect(&Pac, Find(-1).y * SIZE, Find(-1).x * SIZE, Find(-1).y * SIZE + SIZE, Find(-1).x * SIZE + SIZE);
				Oldp_Pac.x = Pac.left;
				Oldp_Pac.y = Pac.top;
			}
			
			if (CenterPoint(Ad_Pac_Position).x == CenterPoint(Pac).x && CenterPoint(Ad_Pac_Position).y == CenterPoint(Pac).y && Pac_Flag == true) {	//팩맨이 한칸을 안가고 돌아옴
				Pac_Flag = false;
				Vir_Pac_Position = Find(-1);
			}

			Newp_Pac.x = Pac.left;	//팩맨 위치 변경
			Newp_Pac.y = Pac.top;

			if (abs(Newp_Pac.x - Oldp_Pac.x) + abs(Newp_Pac.y - Oldp_Pac.y) == SIZE) {	//25픽셀만큼 움직였으면 팩맨이 한칸 이동한걸로 간주
				Oldp_Pac = Newp_Pac;
				switch (Pac_dir) {
				case 1:
					if (!(CanPac.x == -1 && CanPac.y == -1)) {
						Maze[CanPac.x - 1][CanPac.y] = '>';	//맵에서 팩맨좌표 변경
						Maze[CanPac.x][CanPac.y] = ' ';
					}
					break;
				case 2:
					if (!(CanPac.x == -1 && CanPac.y == -1)) {
						Maze[CanPac.x + 1][CanPac.y] = '>';
						Maze[CanPac.x][CanPac.y] = ' ';
					}
					break;
				case 3:
					if (!(CanPac.x == -1 && CanPac.y == -1)) {
						Maze[CanPac.x][CanPac.y - 1] = '>';
						Maze[CanPac.x][CanPac.y] = ' ';
					}
					break;
				case 4:
					if (!(CanPac.x == -1 && CanPac.y == -1)) {
						Maze[CanPac.x][CanPac.y + 1] = '>';
						Maze[CanPac.x][CanPac.y] = ' ';
					}
					break;
				}
				Ad_Pac_Position = Pac;	//바뀐 좌표로 저장
				Pac_Flag = false;
			}
			break;
		case 2: //파워업아이템 끄기
			isPower = false;
			KillTimer(hWnd, 2);
			break;
		case 3: //유령맵에서 감옥문열기
			KillTimer(hWnd, 3); 
			if (GhostMaze[0][12][13] != '1') {
				GhostMaze[0][12][13] = ' ';
			}
			else
				SetTimer(hWnd, 3, 2000, NULL);			
			break;
		case 4:
			KillTimer(hWnd, 4);
			if (GhostMaze[1][12][13] != '2') {
				GhostMaze[1][12][13] = ' ';
			}
			else
				SetTimer(hWnd, 4, 2000, NULL);
			break;
		case 5:
			KillTimer(hWnd, 5);
			if (GhostMaze[2][12][13] != '3') {
				GhostMaze[2][12][13] = ' ';
			}
			else
				SetTimer(hWnd, 5, 2000, NULL);
			break;
		case 6: //꼬리흔들기용 index변경
			if (index == 0)
				index++;
			else if (index == 1) {
				if (index_Flag == false) {
					index_Flag = true;
					index = 2;
				}
				else {
					index_Flag = false;
					index = 0;
				}
			}
			else
				index--;
			break;
		case 7: //유령용
			for (i = 0;i < 3;i++) {
				CanGhost[i] = Can_I_Go(Ghost_dir[i], i);
				Move(i);
				if (Find(i).x == 14 && Find(i).y == 0) {
					GhostMaze[i][Find(i).x][Find(i).y] = ' ';
					GhostMaze[i][14][26] = 49 + i;

					Vir_Ghost_Position[i].x = Find(i).x;
					Vir_Ghost_Position[i].y = Find(i).y;

					SetRect(&Ghost[i], Find(i).y * SIZE, Find(i).x * SIZE, Find(i).y * SIZE + SIZE, Find(i).x * SIZE + SIZE);
					Oldp_Ghost[i].x = Ghost[i].left;
					Oldp_Ghost[i].y = Ghost[i].top;
				}
				else if (Find(i).x == 14 && Find(i).y == 27) {
					GhostMaze[i][Find(i).x][Find(i).y] = ' ';
					GhostMaze[i][14][1] = 49 + i;

					Vir_Ghost_Position[i].x = Find(i).x;	
					Vir_Ghost_Position[i].y = Find(i).y;

					SetRect(&Ghost[i], Find(i).y * SIZE, Find(i).x * SIZE, Find(i).y * SIZE + SIZE, Find(i).x * SIZE + SIZE);
					Oldp_Ghost[i].x = Ghost[i].left;
					Oldp_Ghost[i].y = Ghost[i].top;
				}
				if (Game_Over(Pac, Ghost[i])) { //팩맨과 유령이 만남
					if (isPower == false) { //파워업아이템이 없을시
						life--;
						if (life == 0)
							Ready_To_Start(0);
						else {
							Maze[Find(-1).x][Find(-1).y] = ' ';
							Maze[23][14] = '>';
							Vir_Pac_Position.x = Find(-1).x;
							Vir_Pac_Position.y = Find(-1).y;
							for (k = 0;k < 3;k++) {
								GhostMaze[k][Find(k).x][Find(k).y] = ' ';
							}
							GhostMaze[0][11][14] = '1';
							GhostMaze[1][1][23] = '2';
							GhostMaze[2][5][7] = '3';
							for (k = 0;k < 3;k++) {
								Ghost_Flag[k] = false;
								Vir_Ghost_Position[k].x = Find(k).x;
								Vir_Ghost_Position[k].y = Find(k).y;
								SetRect(&Ghost[k], Find(k).y * SIZE, Find(k).x * SIZE, Find(k).y * SIZE + SIZE, Find(k).x * SIZE + SIZE);
								Save_Ghost_Dir[k] = 0;
							}
							SetRect(&Pac, Find(-1).y * SIZE, Find(-1).x * SIZE, Find(-1).y * SIZE + SIZE, Find(-1).x * SIZE + SIZE);
							Save_Pac_Dir = 0;
							start = false;
							Pac_Flag = false;
						}
						Pac_dir = 0;
						InvalidateRect(hWnd, NULL, TRUE);
					}
					else { //파워업아이템이 있을시
						GhostMaze[i][Find(i).x][Find(i).y] = ' ';
						GhostMaze[i][15][13] = 49 + i;
						Ghost_Flag[i] = false;
						SetRect(&Ghost[i], Find(i).y * SIZE, Find(i).x * SIZE, Find(i).y * SIZE + SIZE, Find(i).x * SIZE + SIZE);
						Vir_Ghost_Position[i].x = Find(i).x;
						Vir_Ghost_Position[i].y = Find(i).y;
						Oldp_Ghost[i].x = Ghost[i].left;
						Oldp_Ghost[i].y = Ghost[i].top;
						Save_Ghost_Dir[i] = 0;
						Ghost_dir[i] = 4;
						if (i == 0)
							SetTimer(hWnd, 3, 3000, NULL);
						else if (i == 1)
							SetTimer(hWnd, 4, 3000, NULL);
						else
							SetTimer(hWnd, 5, 3000, NULL);
					}
				}
				if (CenterPoint(Ad_Ghost_Position[i]).x == CenterPoint(Ghost[i]).x && CenterPoint(Ad_Ghost_Position[i]).y == CenterPoint(Ghost[i]).y && Ghost_Flag[i] == true) {
					Ghost_Flag[i] = false;
					Vir_Ghost_Position[i] = Find(i);	
				}
				Newp_Ghost[i].x = Ghost[i].left;
				Newp_Ghost[i].y = Ghost[i].top;
				if (abs(Newp_Ghost[i].x - Oldp_Ghost[i].x) + abs(Newp_Ghost[i].y - Oldp_Ghost[i].y) == SIZE) {
					Oldp_Ghost[i] = Newp_Ghost[i];
					switch (Ghost_dir[i]) {
					case 1:
						if (!(CanGhost[i].x == -1 && CanGhost[i].y == -1)) {
							GhostMaze[i][CanGhost[i].x - 1][CanGhost[i].y] = 49 + i;
							GhostMaze[i][CanGhost[i].x][CanGhost[i].y] = ' ';
						}
						break;
					case 2:
						if (!(CanGhost[i].x == -1 && CanGhost[i].y == -1)) {
							GhostMaze[i][CanGhost[i].x + 1][CanGhost[i].y] = 49 + i;
							GhostMaze[i][CanGhost[i].x][CanGhost[i].y] = ' ';
						}
						break;
					case 3:
						if (!(CanGhost[i].x == -1 && CanGhost[i].y == -1)) {
							GhostMaze[i][CanGhost[i].x][CanGhost[i].y - 1] = 49 + i;
							GhostMaze[i][CanGhost[i].x][CanGhost[i].y] = ' ';
						}
						break;
					case 4:
						if (!(CanGhost[i].x == -1 && CanGhost[i].y == -1)) {
							GhostMaze[i][CanGhost[i].x][CanGhost[i].y + 1] = 49 + i;
							GhostMaze[i][CanGhost[i].x][CanGhost[i].y] = ' ';
						}
						break;
					}
					Ad_Ghost_Position[i] = Ghost[i];
					Ghost_Flag[i] = false;
				}
				for (k = 0;k < 3;k++) { //감옥문닫음
					if (Find(k).x == 11 && Find(k).y == 13)
						GhostMaze[k][12][13] = '=';
				}
				number = 0;
				if (GhostMaze[i][Find(i).x - 1][Find(i).y] == ' ')
					number++;
				if (GhostMaze[i][Find(i).x + 1][Find(i).y] == ' ')
					number++;
				if (GhostMaze[i][Find(i).x][Find(i).y - 1] == ' ')
					number++;
				if (GhostMaze[i][Find(i).x][Find(i).y + 1] == ' ')
					number++;
				if ((number > 2 || (Can_I_Go(Ghost_dir[i], i).x == -1)) && (!(Find(i).x == 14 && Find(i).y == 0) && !(Find(i).x == 14 && Find(i).y == 27)))
					Dir_Ghost(Pac, Ghost[i], i);
			}
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		ReleaseDC(hWnd, hdc);
		return 0;
	case WM_KEYDOWN: //방향키 입력
		switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			if (start == true) {
				Dir_Pac(wParam); //방향전환
			}
			break;
		case 'R': //재시작
			for (int k = 0;k < 3;k++) {
				GhostMaze[k][Find(k).x][Find(k).y] = ' ';
			}
			GhostMaze[0][11][14] = '1';
			GhostMaze[1][1][23] = '2';
			GhostMaze[2][5][7] = '3';
			for (int k = 0;k < 3;k++) {
				Ghost_Flag[k] = false;
				Vir_Ghost_Position[k].x = Find(k).x;
				Vir_Ghost_Position[k].y = Find(k).y;
				SetRect(&Ghost[k], Find(k).y * SIZE, Find(k).x * SIZE, Find(k).y * SIZE + SIZE, Find(k).x * SIZE + SIZE);
				Save_Ghost_Dir[k] = 0;
			}
			for (int temp = 0;temp < 31;temp++) { //맵초기화
				strcpy_s(Maze[temp], OriginalMaze[temp]); 
			}
			Pac_dir = 0;
			SendMessage(hWnd, WM_CREATE, 0, 0);
			break;
		case ' ': //스페이스바 누르면 게임시작
			mciSendString(_T("play pocketmonster.mp3 repeat"), NULL, 0, NULL); //노래반복재생
			if (start == false) {
				SetTimer(hWnd, 1, 30, NULL);
				SetTimer(hWnd, 7, 20, NULL);
				SetTimer(hWnd, 6, 300, NULL);
			}
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawScreen(hdc); //화면 그리기
		mem1dc = CreateCompatibleDC(hdc);
		oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit1);

		BitBlt(hdc, 0, 0, 750, 850, mem1dc, 0, 0, SRCCOPY);

		SelectObject(mem1dc, oldBit1);
		DeleteDC(mem1dc);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		KillTimer(hWnd, 3);
		KillTimer(hWnd, 4);
		KillTimer(hWnd, 5);
		KillTimer(hWnd, 6);
		KillTimer(hWnd, 7);
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

POINT CenterPoint(RECT &r) { //Rect의 가운데좌표 리턴
	POINT p;
	p.x = (r.left + r.right) / 2;
	p.y = (r.top + r.bottom) / 2;
	return p;
}

void Dir_Pac(int wParam) { //팩맨 방향 결정
	
	switch (wParam) {
	case VK_UP:
		if (Can_I_Go(1, -1).x != -1) {	//갈수있으면 방향변경
			Pac_dir = 1;
			Save_Pac_Dir = 0;
		}
		else  //못가면 방향은 그대로가고 눌린값 저장
			Save_Pac_Dir = 1;
		break;
	case VK_DOWN:
		if (Can_I_Go(2, -1).x != -1) {
			Pac_dir = 2;
			Save_Pac_Dir = 0;
		}
		else
			Save_Pac_Dir = 2;
		break;
	case VK_LEFT:
		if (Can_I_Go(3, -1).x != -1) {
			Pac_dir = 3;
			Save_Pac_Dir = 0;
		}
		else
			Save_Pac_Dir = 3;
		break;
	case VK_RIGHT:
		if (Can_I_Go(4, -1).x != -1) {
			Pac_dir = 4;
			Save_Pac_Dir = 0;
		}
		else
			Save_Pac_Dir = 4;
		break;
	}
}

void Dir_Ghost(RECT &Pac, RECT &Ghost, int Ghostnum) { //유령 방향변경
	int random;
	POINT Pacp, Ghostp;

	Pacp = CenterPoint(Pac);
	Ghostp = CenterPoint(Ghost);

	if (Pacp.x > Ghostp.x) {
		if (Pacp.y > Ghostp.y) {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 3)
					Ghost_dir[Ghostnum] = 2;
				else if (random >= 4 && random <= 6)
					Ghost_dir[Ghostnum] = 4;
				else if (random == 7)
					Ghost_dir[Ghostnum] = 1;
				else
					Ghost_dir[Ghostnum] = 3;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
		else if (Pacp.y < Ghostp.y) {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 3)
					Ghost_dir[Ghostnum] = 1;
				else if (random >= 4 && random <= 6)
					Ghost_dir[Ghostnum] = 4;
				else if (random == 7)
					Ghost_dir[Ghostnum] = 2;
				else
					Ghost_dir[Ghostnum] = 3;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
		else {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 4)
					Ghost_dir[Ghostnum] = 4;
				else if (random == 5)
					Ghost_dir[Ghostnum] = 1;
				else if (random == 6)
					Ghost_dir[Ghostnum] = 2;
				else
					Ghost_dir[Ghostnum] = 3;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
	}
	else if (Pacp.x < Ghostp.x) {
		if (Pacp.y > Ghostp.y) {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 3)
					Ghost_dir[Ghostnum] = 2;
				else if (random >= 4 && random <= 6)
					Ghost_dir[Ghostnum] = 3;
				else if (random == 7)
					Ghost_dir[Ghostnum] = 1;
				else
					Ghost_dir[Ghostnum] = 4;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
		else if (Pacp.y < Ghostp.y) {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 3)
					Ghost_dir[Ghostnum] = 1;
				else if (random >= 4 && random <= 6)
					Ghost_dir[Ghostnum] = 3;
				else if (random == 7)
					Ghost_dir[Ghostnum] = 2;
				else
					Ghost_dir[Ghostnum] = 4;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
		else {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 4)
					Ghost_dir[Ghostnum] = 3;
				else if (random == 5)
					Ghost_dir[Ghostnum] = 1;
				else if (random == 6)
					Ghost_dir[Ghostnum] = 2;
				else
					Ghost_dir[Ghostnum] = 4;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
	}
	else {
		if (Pacp.y < Ghostp.y) {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 4)
					Ghost_dir[Ghostnum] = 1;
				else if (random == 5)
					Ghost_dir[Ghostnum] = 3;
				else if (random == 6)
					Ghost_dir[Ghostnum] = 2;
				else
					Ghost_dir[Ghostnum] = 4;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
		else {
			while (1) {
				random = rand() % 8 + 1;

				if (random >= 1 && random <= 4)
					Ghost_dir[Ghostnum] = 2;
				else if (random == 5)
					Ghost_dir[Ghostnum] = 3;
				else if (random == 6)
					Ghost_dir[Ghostnum] = 1;
				else
					Ghost_dir[Ghostnum] = 4;

				if (Can_I_Go(Ghost_dir[Ghostnum], Ghostnum).x != -1 && Ghost_dir[Ghostnum] != Save_Ghost_Dir[Ghostnum])
					break;
			}
		}
	}
	if (Ghost_dir[Ghostnum] == 1)
		Save_Ghost_Dir[Ghostnum] = 2;
	else if (Ghost_dir[Ghostnum] == 2)
		Save_Ghost_Dir[Ghostnum] = 1;
	else if (Ghost_dir[Ghostnum] == 3)
		Save_Ghost_Dir[Ghostnum] = 4;
	else 
		Save_Ghost_Dir[Ghostnum] = 3;
}

void DrawScreen(HDC hdc) {	//더블버퍼 이용 화면 그리기
	static TCHAR str1[100], str2[100];	//메시지 출력용 변수
	int x, y;
	int iBit;	//비트맵 인덱스

	if (hBit1 == NULL)
		hBit1 = CreateCompatibleBitmap(hdc, 750, 850);

	mem1dc = CreateCompatibleDC(hdc);
	mem2dc = CreateCompatibleDC(mem1dc);
	oldBit1 = (HBITMAP)SelectObject(mem1dc, hBit1);
	for (y = 0;y < 31;y++) {	
		for (x = 0;x < 28;x++) {
			switch (Maze[y][x]) {
			case '.':	
				iBit = 0;
				break;
			case '_':
			case '-':
				iBit = 1;
				break;
			case '!':
			case '|':
				iBit = 2;
				break;
			case 'J':
				iBit = 3;
				break;
			case 'L':
				iBit = 4;
				break;
			case '7':
				iBit = 5;
				break;
			case '/':
				iBit = 6;
				break;
			case '>':
				Oldp_Pac.x = x * SIZE;
				Oldp_Pac.y = y * SIZE;
			case ' ':
			case '#':
				iBit = 7;
				break;
			case 'o':
				iBit = 8;
				break;
			}
			for (int i = 0;i < 3;i++) {
				if (GhostMaze[i][y][x] == 49 + i) {
					Oldp_Ghost[i].x = x * SIZE;
					Oldp_Ghost[i].y = y * SIZE;
					break;
				}
			}
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[iBit]);
			BitBlt(mem1dc, x*SIZE + 10, y*SIZE + 50, SIZE, SIZE, mem2dc, 0, 0, SRCCOPY);
		}
	}
	switch (Pac_dir) {
	case 1:
		if (isPower == false)
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[18 + index]);
		else
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[33 + index]);
		break;
	case 2:
	case 0:
		if (isPower == false)
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[9 + index]);
		else
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[24 + index]);
		break;
	case 3:
		if (isPower == false)
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[12 + index]);
		else
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[27 + index]);
		break;
	case 4:
		if (isPower == false)
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[15 + index]);
		else
			oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[30 + index]);
		break;	
	}
	BitBlt(mem1dc, Pac.left + 10, Pac.top + 50, SIZE, SIZE, mem2dc, 0, 0, SRCCOPY);

	for (int i = 0;i < 3;i++) {
		oldBit2 = (HBITMAP)SelectObject(mem2dc, hBit[21 + i]);
		BitBlt(mem1dc, Ghost[i].left + 10, Ghost[i].top + 50, SIZE, SIZE, mem2dc, 0, 0, SRCCOPY);
	}

	_stprintf_s(str1, _T("score : %4d, life : %2d        "), score, life);
	_stprintf_s(str2, _T("스페이스 : 게임시작, R : 재시작"));
	SetBkColor(mem1dc, RGB(0, 0, 0));
	SetTextColor(mem1dc, RGB(255, 255, 255));

	
	DrawText(mem1dc, str1, _tcslen(str1), &scoreR,
		DT_SINGLELINE | DT_LEFT | DT_TOP);
	DrawText(mem1dc, str2, _tcslen(str2), &explainR,
		DT_SINGLELINE | DT_LEFT | DT_TOP);

	SelectObject(mem2dc, oldBit2);
	DeleteDC(mem2dc);
	SelectObject(mem1dc, oldBit1);
	DeleteDC(mem1dc);
}

POINT Can_I_Go(int direction, int who) { //지정된 방향으로 갈수있는지 확인
	POINT ij;
	int i, j, k, l;
	j = Find(-1).x;
	i = Find(-1).y;
	l = Find(who).x;
 	k = Find(who).y;

	ij.x = -1;	//못갈경우 (-1,-1)리턴
	ij.y = -1;
	

	if (who == -1) {
		switch (direction) {
		case 1:
			if (Maze[j][i] == '>' && (Maze[j - 1][i] == ' ' || Maze[j - 1][i] == '.' || Maze[j - 1][i] == 'o')) {	//이동방향이 비어있고
				if (Maze[j - 2][i] == ' ' || Maze[j - 2][i] == '.' || Maze[j - 2][i] == 'o') {	//그 다음칸도 비어있고
					if (Maze[Vir_Pac_Position.x - 1][Vir_Pac_Position.y] == ' ' || Maze[Vir_Pac_Position.x - 1][Vir_Pac_Position.y] == '.' ||	//미리 보낸 사각형도 이동가능하면
						Maze[Vir_Pac_Position.x - 1][Vir_Pac_Position.y] == '>' || Maze[Vir_Pac_Position.x - 1][Vir_Pac_Position.y] == 'o') {
						ij.x = j;	//팩맨좌표 리턴
						ij.y = i;
					}
				}
				else {	//이동방향만 비었고 그다음칸은 막혀있으면
					ij.x = j;	//팩맨좌표 리턴
					ij.y = i;
				}
			}
			break;
		case 2:
			if (Maze[j][i] == '>' && (Maze[j + 1][i] == ' ' || Maze[j + 1][i] == '.' || Maze[j + 1][i] == 'o')) {
				if (Maze[j + 2][i] == ' ' || Maze[j + 2][i] == '.' || Maze[j + 2][i] == 'o') {
					if (Maze[Vir_Pac_Position.x + 1][Vir_Pac_Position.y] == ' ' || Maze[Vir_Pac_Position.x + 1][Vir_Pac_Position.y] == '.' ||
						Maze[Vir_Pac_Position.x + 1][Vir_Pac_Position.y] == '>' || Maze[Vir_Pac_Position.x + 1][Vir_Pac_Position.y] == 'o') {
						ij.x = j;
						ij.y = i;
					}
				}
				else {
					ij.x = j;
					ij.y = i;
				}
			}
			break;
		case 3:
			if (Maze[j][i] == '>' && (Maze[j][i - 1] == ' ' || Maze[j][i - 1] == '.' || Maze[j][i - 1] == 'o')) {
				if (Maze[j][i - 2] == ' ' || Maze[j][i - 2] == '.' || Maze[j][i - 2] == 'o') {
					if (Maze[Vir_Pac_Position.x][Vir_Pac_Position.y - 1] == ' ' || Maze[Vir_Pac_Position.x][Vir_Pac_Position.y - 1] == '.' ||
						Maze[Vir_Pac_Position.x][Vir_Pac_Position.y - 1] == '>' || Maze[Vir_Pac_Position.x][Vir_Pac_Position.y - 1] == 'o') {
						ij.x = j;
						ij.y = i;
					}
				}
				else {
					ij.x = j;
					ij.y = i;
				}
			}
			break;
		case 4:
			if (Maze[j][i] == '>' && (Maze[j][i + 1] == ' ' || Maze[j][i + 1] == '.' || Maze[j][i + 1] == 'o')) {
				if (Maze[j][i + 2] == ' ' || Maze[j][i + 2] == '.' || Maze[j][i + 2] == 'o') {
					if (Maze[Vir_Pac_Position.x][Vir_Pac_Position.y + 1] == ' ' || Maze[Vir_Pac_Position.x][Vir_Pac_Position.y + 1] == '.' ||
						Maze[Vir_Pac_Position.x][Vir_Pac_Position.y + 1] == '>' || Maze[Vir_Pac_Position.x][Vir_Pac_Position.y + 1] == 'o') {
						ij.x = j;
						ij.y = i;
					}
				}
				else {
					ij.x = j;
					ij.y = i;
				}
			}
			break;
		}
	
	}
	else {
		switch (direction) {
		case 1:
			if (GhostMaze[who][l][k] == 49 + who && GhostMaze[who][l - 1][k] == ' ') {	
				if (GhostMaze[who][l - 2][k] == ' ') {	
					if (GhostMaze[who][Vir_Ghost_Position[who].x - 1][Vir_Ghost_Position[who].y] == ' ' ||	
						GhostMaze[who][Vir_Ghost_Position[who].x - 1][Vir_Ghost_Position[who].y] == 49 + who) {
						ij.x = l;	
						ij.y = k;
					}
				}
				else {	
					ij.x = l;	
					ij.y = k;
				}
			}
			break;
		case 2:
			if (GhostMaze[who][l][k] == 49 + who && GhostMaze[who][l + 1][k] == ' ') {
				if (GhostMaze[who][l + 2][k] == ' ') {
					if (GhostMaze[who][Vir_Ghost_Position[who].x + 1][Vir_Ghost_Position[who].y] == ' ' ||
						GhostMaze[who][Vir_Ghost_Position[who].x + 1][Vir_Ghost_Position[who].y] == 49 + who) {
						ij.x = l;	
						ij.y = k;
					}
				}
				else {
					ij.x = l;	
					ij.y = k;
				}
			}
			break;
		case 3:
			if (GhostMaze[who][l][k] == 49 + who && GhostMaze[who][l][k - 1] == ' ') {	
				if (GhostMaze[who][l][k - 2] == ' ') {
					if (GhostMaze[who][Vir_Ghost_Position[who].x][Vir_Ghost_Position[who].y - 1] == ' ' ||
						GhostMaze[who][Vir_Ghost_Position[who].x][Vir_Ghost_Position[who].y - 1] == 49 + who) {
						ij.x = l;	
						ij.y = k;
					}
				}
				else {
					ij.x = l;	
					ij.y = k;
				}
			}
			break;
		case 4:
			if (GhostMaze[who][l][k] == 49 + who && GhostMaze[who][l][k + 1] == ' ') {	
				if (GhostMaze[who][l][k + 2] == ' ') {
					if (GhostMaze[who][Vir_Ghost_Position[who].x][Vir_Ghost_Position[who].y + 1] == ' ' ||	
						GhostMaze[who][Vir_Ghost_Position[who].x][Vir_Ghost_Position[who].y + 1] == 49 + who) {
						ij.x = l;	
						ij.y = k;
					}
				}
				else {
					ij.x = l;	
					ij.y = k;
				}
			}
			break;
		}

	}
	return ij;
}

POINT Find(int num) { //캐릭터 위치 찾기
	int p, q;
	POINT Here;
	Here.x = -1;
	Here.y = -1;
	for (q = 0;q < 31; q++) {
		for (p = 0;p < 28;p++) {
			if (num == -1) {
				if (Maze[q][p] == '>') {
					Here.x = q;
					Here.y = p;
					return Here;
				}
			}
			else
				if (GhostMaze[num][q][p] == 49 + num) {
					Here.x = q;
					Here.y = p;
					return Here;
			}
		}
	}
}

void Move(int who) { //움직이기
	if (who == -1) {
		switch (Pac_dir) {
		case 1:
			if (!(CanPac.x == -1 && CanPac.y == -1)) //갈수있으면 Rect 이동
				OffsetRect(&Pac, 0, -5);
			if (Ad_Pac_Position.top > Pac.top && Pac_Flag == false) { //위로 움직였고 플래그가 false이고
				if ((Maze[Find(-1).x - 1][Find(-1).y] == ' ' || Maze[Find(-1).x - 1][Find(-1).y] == '.' || Maze[Find(-1).x - 1][Find(-1).y] == 'o')) {	//맵의 팩맨위칸이 비어있음
					Vir_Pac_Position.x = Find(-1).x - 1;	//이동방향으로 미리 가있는 좌표
					Vir_Pac_Position.y = Find(-1).y;
					Pac_Flag = true;
				}
			}
			break;
		case 2:
			if (!(CanPac.x == -1 && CanPac.y == -1))
				OffsetRect(&Pac, 0, 5);
			if (Ad_Pac_Position.top < Pac.top && Pac_Flag == false) {
				if ((Maze[Find(-1).x + 1][Find(-1).y] == ' ' || Maze[Find(-1).x + 1][Find(-1).y] == '.' || Maze[Find(-1).x + 1][Find(-1).y] == 'o')) {
					Vir_Pac_Position.x = Find(-1).x + 1;
					Vir_Pac_Position.y = Find(-1).y;
					Pac_Flag = true;
				}
			}
			break;
		case 3:
			if (!(CanPac.x == -1 && CanPac.y == -1))
				OffsetRect(&Pac, -5, 0);
			if (Ad_Pac_Position.left > Pac.left && Pac_Flag == false) {
				if ((Maze[Find(-1).x][Find(-1).y - 1] == ' ' || Maze[Find(-1).x][Find(-1).y - 1] == '.' || Maze[Find(-1).x][Find(-1).y - 1] == 'o')) {
					Vir_Pac_Position.x = Find(-1).x;
					Vir_Pac_Position.y = Find(-1).y - 1;
					Pac_Flag = true;
				}
			}
			break;
		case 4:
			if (!(CanPac.x == -1 && CanPac.y == -1))
				OffsetRect(&Pac, 5, 0);
			if (Ad_Pac_Position.left < Pac.left && Pac_Flag == false) {
				if ((Maze[Find(-1).x][Find(-1).y + 1] == ' ' || Maze[Find(-1).x][Find(-1).y + 1] == '.' || Maze[Find(-1).x][Find(-1).y + 1] == 'o')) {
					Vir_Pac_Position.x = Find(-1).x;
					Vir_Pac_Position.y = Find(-1).y + 1;
					Pac_Flag = true;
				}
			}
			break;
		}
	}
	else {
		switch (Ghost_dir[who]) {
		case 1:
			if (!(CanGhost[who].x == -1 && CanGhost[who].y == -1))	
				OffsetRect(&Ghost[who], 0, -5);
			if (Ad_Ghost_Position[who].top > Ghost[who].top && Ghost_Flag[who] == false) {	
				if (GhostMaze[who][Find(who).x - 1][Find(who).y] == ' ') {	
					Vir_Ghost_Position[who].x = Find(who).x - 1;	
					Vir_Ghost_Position[who].y = Find(who).y;
					Ghost_Flag[who] = true;
				}
			}
			break;
		case 2:
			if (!(CanGhost[who].x == -1 && CanGhost[who].y == -1))
				OffsetRect(&Ghost[who], 0, 5);
			if (Ad_Ghost_Position[who].top < Ghost[who].top && Ghost_Flag[who] == false) {
				if (GhostMaze[who][Find(who).x + 1][Find(who).y] == ' ') {
					Vir_Ghost_Position[who].x = Find(who).x + 1;
					Vir_Ghost_Position[who].y = Find(who).y;
					Ghost_Flag[who] = true;
				}
			}
			break;
		case 3:
			if (!(CanGhost[who].x == -1 && CanGhost[who].y == -1))
				OffsetRect(&Ghost[who], -5, 0);
			if (Ad_Ghost_Position[who].left > Ghost[who].left && Ghost_Flag[who] == false) {
				if (GhostMaze[who][Find(who).x][Find(who).y - 1] == ' ') {
					Vir_Ghost_Position[who].x = Find(who).x;
					Vir_Ghost_Position[who].y = Find(who).y - 1;
					Ghost_Flag[who] = true;
				}
			}
			break;
		case 4:
			if (!(CanGhost[who].x == -1 && CanGhost[who].y == -1))
				OffsetRect(&Ghost[who], 5, 0);
			if (Ad_Ghost_Position[who].left < Ghost[who].left && Ghost_Flag[who] == false) {
				if (GhostMaze[who][Find(who).x][Find(who).y + 1] == ' ') {
					Vir_Ghost_Position[who].x = Find(who).x;
					Vir_Ghost_Position[who].y = Find(who).y + 1;
					Ghost_Flag[who] = true;
				}
			}
			break;
		}
	}
}

BOOL Game_Over(RECT &Pac, RECT &Ghost) { //팩맨과 유령이 만났는지 확인
	return HitTest(Pac, Ghost);
}

BOOL HitTest(RECT &rect1, RECT &rect2) { //두 Rect가 만났는지 확인
	RECT Hit;
	Hit.bottom = 0;
	Hit.top = 0;
	Hit.left = 0;
	Hit.right = 0;

	IntersectRect(&Hit, &rect1, &rect2);

	if (Hit.bottom != 0 || Hit.top != 0 || Hit.left != 0 || Hit.right != 0)
		return true;
	else
		return false;
}

void Ready_To_Start(int t) { //게임시작 준비세팅
	mciSendString(_T("stop pocketmonster.mp3"), NULL, 0, NULL);
	int number;
	KillTimer(hWndMain, 1);
	KillTimer(hWndMain, 6);
	KillTimer(hWndMain, 7);
	for (int temp = 0;temp < 31;temp++)
		strcpy_s(Maze[temp], OriginalMaze[temp]);
	if(t==0)	
		number = MessageBox(hWndMain, _T("다시 시작하시겠습니까?"), _T("Game Over"), MB_YESNO | MB_ICONQUESTION);
	else
		number = MessageBox(hWndMain, _T("다시 시작하시겠습니까?"), _T("Clear!"), MB_YESNO | MB_ICONQUESTION);

	if (number == IDNO)
		DestroyWindow(hWndMain);
	else {
		for (int k = 0;k < 3;k++) {
			GhostMaze[k][Find(k).x][Find(k).y] = ' ';
		}
		GhostMaze[0][11][14] = '1';
		GhostMaze[1][1][23] = '2';
		GhostMaze[2][5][7] = '3';
		SendMessage(hWndMain, WM_CREATE, 0, 0);
		InvalidateRect(hWndMain, NULL, TRUE);
	}
}