/* FILE NAME: T02CLOCK
*  PROGRAMMER: AO5
*  DATE: 02.06.2015
*  PURPOSE: WinAPI windowed application sample*/

#include <windows.h>
#include <math.h>

/* ��� ������ ���� */
#define WND_CLASS_NAME "My window class"

#define Pi 3.14159265358979323846

/* ������ ������ */
LRESULT CALLBACK MyWindowFunc( HWND hWnd, UINT Msg,
                               WPARAM wParam, LPARAM lParam );

/* ������� ������� ���������.
 *   - ���������� ���������� ����������:
 *       HINSTANCE hInstance;
 *   - ���������� ����������� ���������� ����������
 *     (�� ������������ � ������ ���� NULL):
 *       HINSTANCE hPrevInstance;
 *   - ��������� ������:
 *       CHAR *CmdLine;
 * ������������ ��������:
 *   (INT) ��� �������� � ������������ �������.
 *   0 - ��� ������.
 */
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    CHAR *CmdLine, INT ShowCmd )
{
  WNDCLASS wc;
  HWND hWnd;
  MSG msg;
  /* HINSTANCE hIns = LoadLibrary("shell32"); */

  /* ����������� ������ ���� */
  wc.style = CS_VREDRAW | CS_HREDRAW; /* ����� ����: ��������� ��������������
                                       * ��� ��������� ������������� ���
                                       * ��������������� ��������
                                       * ��� ����� CS_DBLCLKS ��� ����������
                                       * ��������� �������� ������� */
  wc.cbClsExtra = 0; /* �������������� ���������� ���� ��� ������ */
  wc.cbWndExtra = 0; /* �������������� ���������� ���� ��� ���� */
  wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 0));
  wc.hCursor = LoadCursor(NULL, IDC_HAND); /* �������� ������� (����������) */
  wc.hIcon = LoadIcon(NULL, IDI_ASTERISK); /* �������� ����������� (���������) */
  wc.hInstance = hInstance; /* ���������� ����������, ��������������� ����� */
  wc.lpszMenuName = NULL; /* ��� ������� ���� */
  wc.lpfnWndProc = MyWindowFunc; /* ��������� �� ������� ��������� */
  wc.lpszClassName = WND_CLASS_NAME;

  /* ����������� ������ � ������� */
  if (!RegisterClass(&wc))
  {
    MessageBox(NULL, "Error register window class", "ERROR", MB_OK);
    return 0;
  }

  /* �������� ���� */
  hWnd =
    CreateWindow(WND_CLASS_NAME,    /* ��� ������ ���� */
      "Title",                      /* ��������� ���� */
      WS_OVERLAPPEDWINDOW,          /* ����� ���� - ���� ������ ���� */
      CW_USEDEFAULT, CW_USEDEFAULT, /* ������� ���� (x, y) - �� ��������� */
      1000, 1000,                   /* ������� ���� (w, h) - �� ��������� */
      NULL,                         /* ���������� ������������� ���� */
      NULL,                         /* ���������� ������������ ���� */
      hInstance,                    /* ���������� ���������� */
      NULL);                        /* ��������� �� �������������� ��������� */

  ShowWindow(hWnd, ShowCmd);
  UpdateWindow(hWnd);

  /* ������ ����� ��������� ���� */
  while (GetMessage(&msg, NULL, 0, 0))
    /* �������� ��������� � ������� ���� */
    DispatchMessage(&msg);

  return 30;
} /* End of 'WinMain' function */

/* ������� ��������� ��������� ����.
 * ���������:
 *   - ���������� ����:
 *       HWND hWnd;
 *   - ����� ��������� (��. WM_***):
 *       UINT Msg;
 *   - �������� ��������� ('word parameter'):
 *       WPARAM wParam;
 *   - �������� ��������� ('long parameter'):
 *       LPARAM lParam;
 * ������������ ��������:
 *   (LRESULT) - � ����������� �� ���������.
 */
VOID DrawArrow( HDC hDC, INT X1, INT Y1, INT Len, DOUBLE Angle )
{
  DOUBLE si = sin(Angle * Pi / 180), co = cos(Angle * Pi / 180);
  MoveToEx(hDC, X1, Y1, NULL);
  LineTo(hDC, X1 + Len * si, Y1 - Len * co);
}

VOID DrawHand(HDC hDC, INT Xc, INT Yc, INT L, INT W, FLOAT Angle)
{
  INT i;
  POINT pnts[]=
  {
    {0, -W}, {-W, 0}, {0, L}, {W, 0}
  };
  POINT pntdraw[sizeof pnts / sizeof pnts[0]];
  DOUBLE co = cos(Angle), si = sin(Angle);
  for(i = 0; i < sizeof pnts / sizeof pnts[0]; i++)
  {
    pntdraw[i].x = Xc - pnts[i].x * co - pnts[i].y * si;
    pntdraw[i].y = Yc - pnts[i].y * co + pnts[i].x * si;
  }
  Polygon(hDC, pntdraw, sizeof pnts / sizeof pnts[0]);
}

LRESULT CALLBACK MyWindowFunc( HWND hWnd, UINT Msg,
                               WPARAM wParam, LPARAM lParam )
{ 


  HDC hDC;
  PAINTSTRUCT ps;
  SYSTEMTIME st;
  HFONT hFnt, hOldFnt;
  CHAR Buf[100];

  static INT w, h;
  static BITMAP bm;
  static HDC hMemDC, hMemDCClock;
  static HBITMAP hBm, hBmClock;

  switch (Msg)
  {
  case WM_CREATE:
    SetTimer(hWnd, 1, 100, NULL);
    hBmClock = LoadImage(NULL, "clock3.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject(hBmClock, sizeof(bm), &bm);

    hDC = GetDC(hWnd);
    hMemDC = CreateCompatibleDC(hDC);
    hMemDCClock = CreateCompatibleDC(hDC);
    ReleaseDC(hWnd, hDC);

    SelectObject(hMemDCClock, hBmClock);
    return 0;

  case WM_SIZE:
    w = LOWORD(lParam);
    h = HIWORD(lParam);

    if(hBm != NULL)
      DeleteObject(hBm);

    hDC = GetDC(hWnd);
    hBm = CreateCompatibleBitmap(hDC, w, h);
    ReleaseDC(hWnd, hDC);

    SelectObject(hMemDC, hBm);
    SendMessage(hWnd, WM_TIMER, 1, 0);
    return 0;
  
  case WM_TIMER:
    
    SelectObject(hMemDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hMemDC, RGB(0, 0, 0));
    Rectangle(hMemDC, 0, 0, w + 1, h + 1);

    StretchBlt(hMemDC, 0, 0, w, h,
      hMemDCClock, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    GetLocalTime(&st);
    /*DrawArrow(hMemDC, w / 2, h / 2, 450, st.wSecond * 6);
    DrawArrow(hMemDC, w / 2, h / 2, 400, st.wMinute * 6);
    DrawArrow(hMemDC, w / 2, h / 2, 300, (st.wHour % 12) * 30);
    */

    DrawHand(hMemDC, w / 2, h / 2, 450, 10, 270 - st.wSecond * 6 * Pi / 180);
    DrawHand(hMemDC, w / 2, h / 2, 400, 20, 270 - st.wMinute * 6 * Pi / 180);
    DrawHand(hMemDC, w / 2, h / 2, 300, 25, 270 - (st.wHour % 12) * 6 * Pi / 180);

    hFnt = CreateFont(50, 0, 0, 0, FW_BOLD, FALSE, FALSE,
      FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
      VARIABLE_PITCH | FF_ROMAN, "");
    hOldFnt = SelectObject(hMemDC, hFnt);
    

    SetTextColor(hMemDC, RGB(255, 0, 0));
    SetBkMode(hMemDC, TRANSPARENT);
    TextOut(hMemDC, w / 4 + 30, h / 2, Buf,
      sprintf(Buf, "%02d:%02d:%02d (%02d.%02d.%d)",
        st.wHour, st.wMinute, st.wSecond,
        st.wDay, st.wMonth, st.wYear));

    DeleteObject(hFnt);

    InvalidateRect(hWnd, NULL, TRUE);
    return 0;

  case WM_PAINT:
    hDC = BeginPaint(hWnd, &ps);
    BitBlt(hDC, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY);
    EndPaint(hWnd, &ps);
    return 0;

  case WM_ERASEBKGND:
    BitBlt((HDC)wParam, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY);
    return 0;

  /*case WM_CLOSE:
    if (MessageBox(hWnd, "Are you shure to exit from program?",
          "Exit", MB_YESNO | MB_ICONQUESTION) == IDNO)
      return 0;
    break;  */
  
  case WM_DESTROY:
    DeleteObject(hBm);
    DeleteDC(hMemDC);
    DeleteDC(hMemDCClock);
    DeleteObject(hBmClock);
    KillTimer(hWnd, 1);
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hWnd, Msg, wParam, lParam);
} /* End of 'MyWindowFunc' function */


/* END OF 'T02CLOCK.C' FILE */