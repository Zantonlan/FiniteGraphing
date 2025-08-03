#include <windows.h>
#include <commdlg.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct _RTL_OSVERSIONINFOW {
	ULONG dwOSVersionInfoSize;
	ULONG dwMajorVersion;
	ULONG dwMinorVersion;
	ULONG dwBuildNumber;
	ULONG dwPlatformId;
	WCHAR szCSDVERSION[128];
} RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;

typedef LONG(WINAPI *RtlGetVersionPtr)(RTL_OSVERSIONINFOW *lpVersionInformation);

typedef struct point {
	COLORREF color;
	int x;
	int y;
} point;

typedef struct line {
	COLORREF color;
	point *start;
	point *end;
} line;

int screenWidth;
int screenHeight;

bool bldngGrdStr = true;
char *gridStr = NULL;
int inLen = 4;

int gridSize;
int spacing;
bool showGrid = false;
point **grid;

int numOfLines = 0;
line **lines;
COLORREF currLineClr = RGB(0, 255, 0);

bool doingClick = false;
int clickNum = 0;
int clickX;
int clickY;
line *currLine;

void freeGrid() {
	for (int i = 0; i < gridSize; i++) {
		free(grid[i]);
	}
	free(grid);
}

void freeLines() {
	for (int i = 0; i < numOfLines; i++) {
		free(lines[i]);
	}
	free(lines);
	numOfLines = 0;
}

bool checkLine(point* p1, point* p2) {
	MessageBoxA(NULL, "Checking line", "Status Update", MB_OK);
	if (p1 == p2) {
		return false;
	}
	MessageBoxA(NULL, "Passed same-point test", "Status Update", MB_OK);
	
	if (numOfLines > 0) {
		MessageBoxA(NULL, "Running already exists test", "Status Update", MB_OK);
		for (int i = 0; i < numOfLines; i++) {
			point* lp1 = lines[i]->start;
			point* lp2 = lines[i]->end;
			if ((lp1 == p1 && lp2 == p2) || (lp2 == p1 && lp1 == p2)) {
				return false;
			}
		}
	}
	MessageBoxA(NULL, "Passed already exists test", "Status Update", MB_OK);

	return true;
}

COLORREF clrDlg(HWND owner) {
	CHOOSECOLOR cc;
	static COLORREF cstmClrs[16];
	COLORREF chosenClr = RGB(0,0,0);

	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = owner;
	cc.lpCustColors = cstmClrs;
	cc.rgbResult = chosenClr;
	cc.Flags = CC_FULLOPEN|CC_RGBINIT;

	if (ChooseColor(&cc)) {
		return cc.rgbResult;
	} else {
		return CLR_INVALID;
	}
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			if (showGrid) {
				char* tempTxt = malloc(1);
				sprintf(tempTxt, "%ix%i", gridSize, gridSize);
				SetTextColor(hdc, RGB(255, 255, 255));
				SetBkMode(hdc, TRANSPARENT);
				TextOut(hdc, 0, 0, "Hello, Windows!", strlen("Hello, Windows!"));
				TextOut(hdc, 110, 0, tempTxt, strlen(tempTxt));

				//draw grid
				for (int i = 0; i < gridSize; i++) {
					for (int j = 0; j < gridSize; j++) {
						int x = grid[i][j].x;
						int y = grid[i][j].y;
						Ellipse(hdc, x-2, y-2, x+2, y+2);

						/*
						if (i == 0) {
							if (j == 0) {
								MoveToEx(hdc, x, y, NULL);
							} else if (j == 1) {
								HPEN newPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
								HPEN oldPen = SelectObject(hdc, newPen);
								LineTo(hdc, x, y);
								SelectObject(hdc, oldPen);
								DeleteObject(newPen);
							}
						} else if (i == 1) {
							if (j == 0) {
								MoveToEx(hdc, x, y, NULL);
							} else if (j == 1) {
								HPEN newPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
								HPEN oldPen = SelectObject(hdc, newPen);
								LineTo(hdc, x, y);
								SelectObject(hdc, oldPen);
								DeleteObject(newPen);
							}
						}
						*/
					}
				}

				//draw lines
				if (numOfLines > 0) {
					for (int i = 0; i < numOfLines; i++) {
						int x1 = lines[i]->start->x;
						int y1 = lines[i]->start->y;
						int x2 = lines[i]->end->x;
						int y2 = lines[i]->end->y;

						MoveToEx(hdc, x1, y1, NULL);
						HPEN newPen = CreatePen(PS_SOLID, 1, lines[i]->color);
						HPEN oldPen = SelectObject(hdc, newPen);
						LineTo(hdc, x2, y2);
						SelectObject(hdc, oldPen);
						DeleteObject(newPen);
					}
				}
			}

			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
		{
			HDC hdc = (HDC)wparam;
			RECT rc;
			GetClientRect(hwnd, &rc);
			FillRect(hdc, &rc, CreateSolidBrush(RGB(0, 0, 0))); //CreateSolidBrush(GetSysColor(COLOR_WINDOW))
			return 1;
		}
		case WM_CHAR:
		{
			if (wparam == VK_BACK) {
				if (inLen > 0) {
					inLen--;
				}
			} else if (wparam == VK_RETURN) {
				if (bldngGrdStr) {
					char *tempPtr = realloc(gridStr, inLen+1);
					gridStr = tempPtr;
					gridStr[inLen] = '\0';
					inLen++;

					gridStr +=4;
					bldngGrdStr = false;
				}

				MessageBoxA(NULL, gridStr, "What you typed", MB_OK);

				gridSize = atoi(gridStr);

				char *testStr = malloc(2);
				testStr[0] = (char)gridSize;
				testStr[1] = 0;

				MessageBoxA(NULL, testStr, "Int test", MB_OK);

				printf("Grid size: %i", gridSize);
				fflush(stdout);

				//make grid table
				grid = malloc(sizeof(point*)*gridSize);
				spacing = (int)floor(800/(gridSize));
				for (int i = 0; i < gridSize; i++) {
					point *currColumn = malloc(sizeof(point)*gridSize);
					for (int j = 0; j < gridSize; j++) {
						currColumn[j].color = RGB(255, 255, 255);
						currColumn[j].x = spacing*i + spacing/2;
						currColumn[j].y = spacing*j + spacing/2;
					}
					grid[i] = currColumn;
				}

				showGrid = true;
				InvalidateRect(hwnd, NULL, true);
				return 0;
			} else if (wparam >= 48 && wparam <= 57 && bldngGrdStr) {
				MessageBox(NULL, "You typed something!", "I knew it", MB_OK);
				char *tempPtr = realloc(gridStr, inLen+1);
				gridStr = tempPtr;
				gridStr[inLen] =(char)wparam;
				inLen++;
			} else if (wparam == 'c' || wparam == 'C') {
				MessageBoxA(NULL, "Choose a new line color.", "Status Update", MB_OK);
				COLORREF lineColor = clrDlg(hwnd);
				if (lineColor != CLR_INVALID) {
					currLineClr = lineColor;
				} else {
					MessageBoxA(NULL, "Invalid color selected.", "Error", MB_OK|MB_ICONERROR);
				}
			}
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			if (showGrid) {
				int mx = LOWORD(lparam);
				int my = HIWORD(lparam);

				point *best = NULL;
				int bestDist = 800;
				for (int i = 0; i < gridSize; i++) {
					for (int j = 0; j < gridSize; j++) {
						int dx = grid[i][j].x - mx;
						int dy = grid[i][j].y - my;
						int dist = sqrt(pow(dx, 2) + pow(dy, 2));
						
						if (dist < bestDist) {
							bestDist = dist;
							best = &grid[i][j];
						}
					}
				}

				clickNum++;

				if (clickNum == 1) {
					currLine = malloc(sizeof(line));
					currLine->color = currLineClr;
					currLine->start = best;
				} else if (clickNum == 2) {
					currLine->end = best;
					if (checkLine(currLine->start, currLine->end)) {
						numOfLines++;
						clickNum = 0;
						MessageBoxA(NULL, "Line went through", "Line Check", MB_OK);
						lines = realloc(lines, sizeof(line*)*numOfLines);
						lines[numOfLines-1] = currLine;
						doingClick = true;
						InvalidateRect(hwnd, NULL, true);
					} else {
						clickNum = 0;
						MessageBoxA(NULL, "Line didn't go through", "Line Check", MB_OK);
						free(currLine);
					}
				}
			}
			return 0;
		}
		case WM_DESTROY:
		{
			freeGrid();
			freeLines();
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
	//ensure nobody's running this on Windows 11.
	freopen("log.txt", "w", stdout);
	freopen("log.txt", "w", stderr);
	bool error = false;
	long major, minor, build;

	HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
	if (hMod) {
		RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
		if (fxPtr != NULL) {
			RTL_OSVERSIONINFOW rovi = {0};
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (fxPtr(&rovi) == 0) {
				major = (long)rovi.dwMajorVersion;
				minor = (long)rovi.dwMinorVersion;
				build = (long)rovi.dwBuildNumber;
			}
		} else {
			goto Error500;
		}
	} else {
		goto Error500;
	}
	goto Error500Ovr;
	Error500:
		MessageBox(NULL, "A critical system error has caused the program to stop running. Make sure your Windows installation isn't corrupted, then try again.", "CRITICAL ERROR!", MB_OK|MB_ICONERROR);
		return 404;
	Error500Ovr:

	//RTL_OSVERSIONINFOW *infoStruct;
	//RtlGetVersion(infoStruct);

	if (major > 10 || (major == 10 && build >= 22000) || (major == 10 && minor > 0)) {
		MessageBox(NULL, "404 Error\nRequired libraries not found. Latest supported version: Windows 10 22H2. Earliest supported version: Windows XP", "Error 404", MB_OK|MB_ICONERROR);
		return 404;
	}
	
	//actual stuff
	WNDCLASSEX wc = {0};
	HWND hwnd;
	MSG msg;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = "Calc";
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassEx(&wc)) return 0;

	hwnd = CreateWindowA(
		"Calc",
		"Finite Graphing",
		WS_CAPTION|WS_OVERLAPPED|WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 800,
		NULL, NULL, hInst, NULL);

	if (!hwnd) return 0;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}