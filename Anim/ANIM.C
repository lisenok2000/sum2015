/* FILENAME: ANIM.C
 * PROGRAMMER: AO5
 * PURPOSE: Animation system module.
 * LAST UPDATE: 10.06.2015
 */

#include "anim.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm")

/* ��������� �������� ��� ��������� */
#define AO5_GET_AXIS_VALUE(Axis) \
  (2.0 * (ji.dw ## Axis ## pos - jc.w ## Axis ## min) / (jc.w ## Axis ## max - jc.w ## Axis ## min) - 1.0)

/* ��������� �������� �������� */
ao5ANIM AO5_Anim;

/* ������ ��� ������������� �� ������� */
static INT64
  TimeFreq,  /* ������ ��������� � ������� */
  TimeStart, /* ����� ������ �������� */
  TimeOld,   /* ����� �������� ����� */
  TimePause, /* ����� ������� � ����� */
  TimeFPS;   /* ����� ��� ������ FPS */
static INT
  FrameCounter; /* ������� ������ */

/* ����������� ������� ���������� */
static INT
  AO5_MouseOldX, AO5_MouseOldY;

/* ������� ������������� ��������.
 * ���������:
 *   - ���������� ����:
 *       HWND hWnd;
 * ������������ ��������:
 *   (BOOL) TRUE ��� ������, ����� FALSE.
 */
BOOL AO5_AnimInit( HWND hWnd )
{
  INT i;
  LARGE_INTEGER li;
  POINT pt;
  PIXELFORMATDESCRIPTOR pfd = {0};

  memset(&AO5_Anim, 0, sizeof(ao5ANIM));
  AO5_Anim.hWnd = hWnd;
  AO5_Anim.hDC = GetDC(hWnd);

  /* �������������� ����� ����� */
  AO5_Anim.W = 30;
  AO5_Anim.H = 30;
  AO5_Anim.NumOfUnits = 0;

  /*** ������������� OpenGL ***/

  /* ��������� ������ ����� */
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 32;
  i = ChoosePixelFormat(AO5_Anim.hDC, &pfd);
  DescribePixelFormat(AO5_Anim.hDC, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
  SetPixelFormat(AO5_Anim.hDC, i, &pfd);

  /* ������� �������� ���������� */
  AO5_Anim.hGLRC = wglCreateContext(AO5_Anim.hDC);
  /* ������ �������� ��������� */
  wglMakeCurrent(AO5_Anim.hDC, AO5_Anim.hGLRC);

  /* �������������� ���������� */
  if (glewInit() != GLEW_OK ||
      !(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader))
  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(AO5_Anim.hGLRC);
    ReleaseDC(AO5_Anim.hWnd, AO5_Anim.hDC);
    memset(&AO5_Anim, 0, sizeof(ao5ANIM));
    return FALSE;
  }

  AO5_RndProg = AO5_ShaderLoad("TEST");

  glActiveTexture(GL_TEXTURE0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glActiveTexture(GL_TEXTURE1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  /* ������������� ������� */
  QueryPerformanceFrequency(&li);
  TimeFreq = li.QuadPart;
  QueryPerformanceCounter(&li);
  TimeStart = TimeOld = TimeFPS = li.QuadPart;
  AO5_Anim.IsPause = FALSE;
  FrameCounter = 0;

  /* ������������� ����� */
  GetCursorPos(&pt);
  ScreenToClient(AO5_Anim.hWnd, &pt);
  AO5_MouseOldX = pt.x;
  AO5_MouseOldY = pt.y;
  GetKeyboardState(AO5_Anim.KeysOld);
  return TRUE;
} /* End of 'AO5_AnimInit' function */

/* ������� ��������������� ��������.
 * ���������: ���.
 * ������������ ��������: ���.
 */
VOID AO5_AnimClose( VOID )
{
  INT i;

  /* ������������ ������ ��-��� �������� �������� */
  for (i = 0; i < AO5_Anim.NumOfUnits; i++)
  {
    AO5_Anim.Units[i]->Close(AO5_Anim.Units[i], &AO5_Anim);
    free(AO5_Anim.Units[i]);
    AO5_Anim.Units[i] = NULL;
  }

  AO5_ShaderFree(AO5_RndProg);
  AO5_RndProg = 0;
  /* ������ �������� ��������� */
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(AO5_Anim.hGLRC);
  /* ������� ������� GDI */
  ReleaseDC(AO5_Anim.hWnd, AO5_Anim.hDC);
} /* End of 'AO5_AnimClose' function */

/* ������� ��������� ��������� ������� ������� ������.
 * ���������:
 *   - ����� ������ ������� ������:
 *       INT W, H;
 * ������������ ��������: ���.
 */
VOID AO5_AnimResize( INT W, INT H )
{
  /* ���������� ������� */
  AO5_Anim.W = W;
  AO5_Anim.H = H;

  glViewport(0, 0, W, H);

  /* ������������� ���������� ������������� */
  if (W > H)
    AO5_RndWp = (DBL)W / H * 3, AO5_RndHp = 3;
  else
    AO5_RndHp = (DBL)H / W * 3, AO5_RndWp = 3;

  AO5_RndMatrProj = MatrFrustum(-AO5_RndWp / 2, AO5_RndWp / 2,
                                -AO5_RndHp / 2, AO5_RndHp / 2,
                                AO5_RndProjDist, 800);

} /* End of 'AO5_AnimResize' function */

/* ������� ���������� ����� ��������.
 * ���������: ���.
 * ������������ ��������: ���.
 */
VOID AO5_AnimRender( VOID )
{
  INT i;
  LARGE_INTEGER li;
  POINT pt;

  /*** ���������� ������� ***/
  QueryPerformanceCounter(&li);
  AO5_Anim.GlobalTime = (DBL)(li.QuadPart - TimeStart) / TimeFreq;
  AO5_Anim.GlobalDeltaTime = (DBL)(li.QuadPart - TimeOld) / TimeFreq;

  if (!AO5_Anim.IsPause)
    AO5_Anim.DeltaTime = AO5_Anim.GlobalDeltaTime;
  else
  {
    TimePause += li.QuadPart - TimeOld;
    AO5_Anim.DeltaTime = 0;
  }

  AO5_Anim.Time = (DBL)(li.QuadPart - TimePause - TimeStart) / TimeFreq;

  /* ��������� FPS */
  if (li.QuadPart - TimeFPS > TimeFreq)
  {
    AO5_Anim.FPS = FrameCounter / ((DBL)(li.QuadPart - TimeFPS) / TimeFreq);
    TimeFPS = li.QuadPart;
    FrameCounter = 0;
  }

  /* ����� "��������" ����� */
  TimeOld = li.QuadPart;

  /*** ���������� ����� ***/

  /* ���������� */
  GetKeyboardState(AO5_Anim.Keys);
  for (i = 0; i < 256; i++)
    AO5_Anim.Keys[i] >>= 7;
  for (i = 0; i < 256; i++)
    AO5_Anim.KeysClick[i] = AO5_Anim.Keys[i] && !AO5_Anim.KeysOld[i];
  memcpy(AO5_Anim.KeysOld, AO5_Anim.Keys, sizeof(AO5_Anim.KeysOld));

  /* ���� */
  /* ������ */
  AO5_Anim.MsWheel = AO5_MouseWheel;
  AO5_MouseWheel = 0;
  /* ���������� ������� */
  GetCursorPos(&pt);
  ScreenToClient(AO5_Anim.hWnd, &pt);
  AO5_Anim.MsX = pt.x;
  AO5_Anim.MsY = pt.y;
  /* ������������� ����������� */
  AO5_Anim.MsDeltaX = pt.x - AO5_MouseOldX;
  AO5_Anim.MsDeltaY = pt.y - AO5_MouseOldY;
  AO5_MouseOldX = pt.x;
  AO5_MouseOldY = pt.y;

  /* �������� */
  if ((i = joyGetNumDevs()) > 0)
  {
    JOYCAPS jc;

    /* ��������� ����� ���������� � �������� */
    if (joyGetDevCaps(JOYSTICKID2, &jc, sizeof(jc)) == JOYERR_NOERROR)
    {
      JOYINFOEX ji;

      /* ��������� �������� ��������� */
      ji.dwSize = sizeof(JOYCAPS);
      ji.dwFlags = JOY_RETURNALL;
      if (joyGetPosEx(JOYSTICKID2, &ji) == JOYERR_NOERROR)
      {
        /* ������ */
        memcpy(AO5_Anim.JButsOld, AO5_Anim.JButs, sizeof(AO5_Anim.JButs));
        for (i = 0; i < 32; i++)
          AO5_Anim.JButs[i] = (ji.dwButtons >> i) & 1;
        for (i = 0; i < 32; i++)
          AO5_Anim.JButsClick[i] = AO5_Anim.JButs[i] && !AO5_Anim.JButsOld[i];

        /* ��� */
        AO5_Anim.JX = AO5_GET_AXIS_VALUE(X);
        AO5_Anim.JY = AO5_GET_AXIS_VALUE(Y);
        if (jc.wCaps & JOYCAPS_HASZ)
          AO5_Anim.JZ = AO5_GET_AXIS_VALUE(Z);
        if (jc.wCaps & JOYCAPS_HASU)
          AO5_Anim.JU = AO5_GET_AXIS_VALUE(U);
        if (jc.wCaps & JOYCAPS_HASV)
          AO5_Anim.JV = AO5_GET_AXIS_VALUE(V);
        if (jc.wCaps & JOYCAPS_HASR)
          AO5_Anim.JR = AO5_GET_AXIS_VALUE(R);

        if (jc.wCaps & JOYCAPS_HASPOV)
        {
          if (ji.dwPOV == 0xFFFF)
            AO5_Anim.JPOV = 0;
          else
            AO5_Anim.JPOV = ji.dwPOV / 4500 + 1;
        }
      }
    }
  }

  /* ����� �� ��������� ��������� �������� */
  for (i = 0; i < AO5_Anim.NumOfUnits; i++)
    AO5_Anim.Units[i]->Response(AO5_Anim.Units[i], &AO5_Anim);

  /* ������� ���� */
  glClearColor(0.3, 0.5, 0.7, 1);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* ��������� �������� */
  for (i = 0; i < AO5_Anim.NumOfUnits; i++)
  {
    static DBL time = 5;

    time += AO5_Anim.GlobalDeltaTime;
    if (time > 5)
    {
      time = 0;
      AO5_ShaderFree(AO5_RndProg);
      AO5_RndProg = AO5_ShaderLoad("TEST");
    }

    AO5_RndMatrWorld = MatrIdentity();

    AO5_Anim.Units[i]->Render(AO5_Anim.Units[i], &AO5_Anim);
  }
  glFinish();
  FrameCounter++;
} /* End of 'AO5_AnimRender' function */

/* ������� ������ ����� ��������.
 * ���������: ���.
 * ������������ ��������: ���.
 */
VOID AO5_AnimCopyFrame( VOID )
{
  /* ����� ������ ����� */
  SwapBuffers(AO5_Anim.hDC);
} /* End of 'AO5_AnimCopyFrame' function */

/* ������� ���������� � ������� ������� ��������.
 * ���������:
 *   - ����������� ������ ��������:
 *       AO5UNIT *Unit;
 * ������������ ��������: ���.
 */
VOID AO5_AnimAddUnit( ao5UNIT *Unit )
{
  if (AO5_Anim.NumOfUnits < AO5_MAX_UNITS)
  {
    AO5_Anim.Units[AO5_Anim.NumOfUnits++] = Unit;
    Unit->Init(Unit, &AO5_Anim);
  }
} /* End of 'AO5_AnimAddUnit' function */


/* ������� ������������ �/�� �������������� ������
 * � ������ ���������� ���������.
 * ���������: ���.
 * ������������ ��������: ���.
 */
VOID AO5_AnimFlipFullScreen( VOID )
{
  static BOOL IsFullScreen = FALSE; /* ������� ����� */
  static RECT SaveRC;               /* ����������� ������ */

  if (!IsFullScreen)
  {
    RECT rc;
    HMONITOR hmon;
    MONITORINFOEX moninfo;

    /* ��������� ������ ������ ���� */
    GetWindowRect(AO5_Anim.hWnd, &SaveRC);

    /* ���������� � ����� �������� ��������� ���� */
    hmon = MonitorFromWindow(AO5_Anim.hWnd, MONITOR_DEFAULTTONEAREST);

    /* �������� ���������� ��� �������� */
    moninfo.cbSize = sizeof(moninfo);
    GetMonitorInfo(hmon, (MONITORINFO *)&moninfo);

    /* ��������� � ������ ����� */
    rc = moninfo.rcMonitor;

    AdjustWindowRect(&rc, GetWindowLong(AO5_Anim.hWnd, GWL_STYLE), FALSE);

    SetWindowPos(AO5_Anim.hWnd, HWND_TOPMOST,
      rc.left, rc.top,
      rc.right - rc.left, rc.bottom - rc.top,
      SWP_NOOWNERZORDER);
    IsFullScreen = TRUE;
  }
  else
  {
    /* ��������������� ������ ���� */
    SetWindowPos(AO5_Anim.hWnd, HWND_TOP,
      SaveRC.left, SaveRC.top,
      SaveRC.right - SaveRC.left, SaveRC.bottom - SaveRC.top,
      SWP_NOOWNERZORDER);
    IsFullScreen = FALSE;
  }
} /* End of 'AO5_AnimFlipFullScreen' function */

/* ������� ������ �� ��������.
 * ���������: ���.
 * ������������ ��������: ���.
 */
VOID AO5_AnimDoExit( VOID )
{
  PostMessage(AO5_Anim.hWnd, WM_CLOSE, 0, 0);
} /* End of 'AO5_AnimDoExit' function */

/* ������� ��������� ����� ��������.
 * ���������:
 *   - ���� �����:
 *       BOOL NewPauseFlag;
 * ������������ ��������: ���.
 */
VOID AO5_AnimSetPause( BOOL NewPauseFlag )
{
  AO5_Anim.IsPause = NewPauseFlag;
} /* End of 'AO5_AnimSetPause' function */

/* END OF 'ANIM.C' FILE */
