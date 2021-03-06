/* FILENAME: PRIM.C
 * PROGRAMMER: AO5
 * PURPOSE: Primitive handle module.
 * LAST UPDATE: 13.06.2015
 */

#include "anim.h"
#include "render.h"
#include "image.h"

/* ������� ��������� ��������� ��� �������� */
MATR AO5_RndPrimMatrConvert = AO5_UNIT_MATR;

/* ���� ����� �������� �� ��������� */
COLOR AO5_RndPrimDefaultColor = {1, 1, 1, 1};

/* ������� �������� ���������.
 * ���������:
 *   - ��������� �� ��������:
 *       ao5PRIM *Prim;
 *   - ��� ��������� (AO5_PRIM_***):
 *       ao5PRIM_TYPE Type;
 *   - ���������� ������ � ��������:
 *       INT NoofV, NoofI;
 *   - ������ ������:
 *       ao5VERTEX *Vertices;
 *   - ������ ��������:
 *       INT *Indices;
 * ������������ ��������: ���.
 *   (BOOL) TRUE ��� ������, ����� FALSE.
 */
VOID AO5_PrimCreate( ao5PRIM *Prim, ao5PRIM_TYPE Type,
                     INT NoofV, INT NoofI, ao5VERTEX *Vertices, INT *Indices)
{
  Prim->Type = Type;
  Prim->NumOfI = NoofI;
  /* ������� ������ OpenGL */
  glGenVertexArrays(1, &Prim->VA);
  glGenBuffers(1, &Prim->VBuf);
  glGenBuffers(1, &Prim->IBuf);

  /* ������ �������� ������ ������ */
  glBindVertexArray(Prim->VA);
  /* ������ �������� ����� ������ */
  glBindBuffer(GL_ARRAY_BUFFER, Prim->VBuf);
  /* ������� ������ */
  glBufferData(GL_ARRAY_BUFFER, sizeof(ao5VERTEX) * NoofV, Vertices, GL_STATIC_DRAW);
  /* ������ �������� ����� �������� */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Prim->IBuf);
  /* ������� ������ */
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INT) * NoofI, Indices, GL_STATIC_DRAW);

  /* ������ ������� ������ */
  /*                    layout,
   *                       ���������� ���������,
   *                          ���,
   *                                    ���� �� �����������,
   *                                           ������ � ������ ������ �������� ������,
   *                                                           �������� � ������ �� ������ ������ */
  glVertexAttribPointer(0, 3, GL_FLOAT, FALSE, sizeof(ao5VERTEX), (VOID *)0); /* ������� */
  glVertexAttribPointer(1, 2, GL_FLOAT, FALSE, sizeof(ao5VERTEX), (VOID *)sizeof(VEC)); /* �������� */
  glVertexAttribPointer(2, 3, GL_FLOAT, FALSE, sizeof(ao5VERTEX), (VOID *)(sizeof(VEC) + sizeof(ao5UV))); /* ������� */
  glVertexAttribPointer(3, 4, GL_FLOAT, FALSE, sizeof(ao5VERTEX), (VOID *)(sizeof(VEC) * 2 + sizeof(ao5UV))); /* ���� */

  /* �������� ������ ��������� (layout) */
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  /* ����������� �� ������� ������ */
  glBindVertexArray(0);
} /* End of 'AO5_PrimCreate' function */

/* ������� �������� ���������.
 * ���������:
 *   - ��������� �� ��������:
 *       ao5PRIM *Prim;
 * ������������ ��������: ���.
 */
VOID AO5_PrimFree( ao5PRIM *Prim )
{
  /* ������ �������� ������ ������ */
  glBindVertexArray(Prim->VA);
  /* "���������" ������ */
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &Prim->VBuf);
  glDeleteBuffers(1, &Prim->IBuf);
  /* ������ ���������� ������ ������ */
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &Prim->VA);

  memset(Prim, 0, sizeof(ao5PRIM));
} /* End of 'AO5_PrimFree' function */

/* ������� ��������� ���������.
 * ���������:
 *   - ��������� �� ��������:
 *       ao5PRIM *Prim;
 * ������������ ��������: ���.
 */
VOID AO5_PrimDraw( ao5PRIM *Prim )
{
  INT loc;
  MATR M;

  AO5_RndMatrWorldViewProj = MatrMulMatr(MatrMulMatr(AO5_RndMatrWorld, AO5_RndMatrView), AO5_RndMatrProj);

  /* ��������� ��� ��������, ���� ��� �������� */
  glLoadMatrixf(AO5_RndMatrWorldViewProj.A[0]);

  /* ������ ������������ */
  glBindVertexArray(Prim->VA);
  glUseProgram(AO5_RndProg);

  loc = glGetUniformLocation(AO5_RndProg, "MatrWorld");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, AO5_RndMatrWorld.A[0]);
  loc = glGetUniformLocation(AO5_RndProg, "MatrView");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, AO5_RndMatrView.A[0]);
  loc = glGetUniformLocation(AO5_RndProg, "MatrProj");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, AO5_RndMatrProj.A[0]);
  loc = glGetUniformLocation(AO5_RndProg, "MatrWVP");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, AO5_RndMatrWorldViewProj.A[0]);

  M = MatrTranspose(MatrInverse(MatrMulMatr(AO5_RndMatrWorld, AO5_RndMatrView)));
  loc = glGetUniformLocation(AO5_RndProg, "MatrWVInverse");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, M.A[0]);

  M = MatrTranspose(MatrInverse(AO5_RndMatrWorld));
  loc = glGetUniformLocation(AO5_RndProg, "MatrWInverse");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, M.A[0]);

  M = MatrMulMatr(AO5_RndMatrWorld, AO5_RndMatrView);
  loc = glGetUniformLocation(AO5_RndProg, "MatrWV");
  if (loc != -1)
    glUniformMatrix4fv(loc, 1, FALSE, M.A[0]);

  loc = glGetUniformLocation(AO5_RndProg, "Time");
  if (loc != -1)
    glUniform1f(loc, AO5_Anim.Time);

  /* ���������� ��������� */
  loc = glGetUniformLocation(AO5_RndProg, "Ka");
  if (loc != -1)
    glUniform3fv(loc, 1, &AO5_MtlLib[Prim->MtlNo].Ka.X);
  loc = glGetUniformLocation(AO5_RndProg, "Kd");
  if (loc != -1)
    glUniform3fv(loc, 1, &AO5_MtlLib[Prim->MtlNo].Kd.X);
  loc = glGetUniformLocation(AO5_RndProg, "Ks");
  if (loc != -1)
    glUniform3fv(loc, 1, &AO5_MtlLib[Prim->MtlNo].Ks.X);
  loc = glGetUniformLocation(AO5_RndProg, "Kp");
  if (loc != -1)
    glUniform1f(loc, AO5_MtlLib[Prim->MtlNo].Kp);
  loc = glGetUniformLocation(AO5_RndProg, "Kt");
  if (loc != -1)
    glUniform1f(loc, AO5_MtlLib[Prim->MtlNo].Kt);

  loc = glGetUniformLocation(AO5_RndProg, "IsTextureUse");
  if (AO5_MtlLib[Prim->MtlNo].TexId == 0)
    glUniform1f(loc, 0);
  else
  {
    glUniform1f(loc, 1);
    glBindTexture(GL_TEXTURE_2D, AO5_MtlLib[Prim->MtlNo].TexId);
  }

  glEnable(GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex(0xFFFFFFFF);
  if (Prim->Type == AO5_PRIM_GRID)
    glDrawElements(GL_TRIANGLE_STRIP, Prim->NumOfI, GL_UNSIGNED_INT, NULL);
  else
    glDrawElements(GL_TRIANGLES, Prim->NumOfI, GL_UNSIGNED_INT, NULL);

  glUseProgram(0);
  glBindVertexArray(0);
} /* End of 'AO5_PrimDraw' function */

/* ������� �������� ��������� ���������.
 * ���������:
 *   - ��������� �� ��������:
 *       AO5PRIM *Prim;
 *   - ������� �����:
 *       VEC Loc;
 *   - ����������� �������-�������:
 *       VEC Du, Dv;
 *   - ���������:
 *       INT N, M;
 * ������������ ��������:
 *   (BOOL) TRUE ��� ������, ����� FALSE.
 */
BOOL AO5_PrimCreatePlane( ao5PRIM *Prim, VEC Loc, VEC Du, VEC Dv, INT N, INT M )
{
  INT i, j;
  VEC norm;
  INT *Ind, *iptr;
  ao5VERTEX *V, *ptr;

  memset(Prim, 0, sizeof(ao5PRIM));

  if ((V = malloc(sizeof(ao5VERTEX) * N * M +
                  sizeof(INT) * ((2 * M + 1) * (N - 1)))) == NULL)
    return FALSE;
  Ind = (INT *)(V + N * M);

  /* ��������� ������� */
  norm = VecNormalize(VecCrossVec(Du, Dv));
  for (ptr = V, i = 0; i < N; i++)
    for (j = 0; j < M; j++, ptr++)
    {
      ptr->P = VecAddVec(Loc,
                 VecAddVec(VecMulNum(Du, j / (M - 1.0)),
                           VecMulNum(Dv, i / (N - 1.0))));
      ptr->C = AO5_RndPrimDefaultColor;
      ptr->N = norm;
      ptr->T = UVSet(j / (M - 1.0), i / (N - 1.0));
    }

  /* ��������� ������� */
  for (iptr = Ind, i = 0; i < N - 1; i++)
  {
    for (j = 0; j < M; j++)
    {
      /* ������� */
      *iptr++ = i * M + j + M;
      /* ������ */
      *iptr++ = i * M + j;
    }
    /* ��������� ������ ������� ��������� */
    *iptr++ = 0xFFFFFFFF;
  }

  AO5_PrimCreate(Prim, AO5_PRIM_GRID, M * N, (2 * M + 1) * (N - 1), V, Ind);

  free(V);

  return TRUE;
} /* End of 'AO5_PrimCreatePlane' function */

/* ������� �������� ��������� �����.
 * ���������:
 *   - ��������� �� ��������:
 *       AO5PRIM *Prim;
 *   - ����� �����:
 *       VEC �;
 *   - ������ �����:
 *       FLT R;
 *   - ���������:
 *       INT N, M;
 * ������������ ��������:
 *   (BOOL) TRUE ��� ������, ����� FALSE.
 */
BOOL AO5_PrimCreateSphere( ao5PRIM *Prim, VEC C, FLT R, INT N, INT M )
{
  INT i, j;
  INT *Ind, *iptr;
  ao5VERTEX *V, *ptr;

  memset(Prim, 0, sizeof(ao5PRIM));

  if ((V = malloc(sizeof(ao5VERTEX) * N * M +
                  sizeof(INT) * ((2 * M + 1) * (N - 1)))) == NULL)
    return FALSE;
  Ind = (INT *)(V + N * M);

  /* ��������� ������� */
  for (ptr = V, i = 0; i < N; i++)
    for (j = 0; j < M; j++, ptr++)
    {
      DBL
        theta = PI * i / (N - 1.0), phi = 2 * PI * j / (M - 1.0),
        x = sin(theta) * sin(phi),
        y = cos(theta),
        z = sin(theta) * cos(phi);

      ptr->P = VecAddVec(C, VecSet(R * x, R * y, R * z));
      ptr->C = AO5_RndPrimDefaultColor;
      ptr->N = VecSet(x, y, z);
      ptr->T = UVSet(j / (M - 1.0), i / (N - 1.0));
    }

  /* ��������� ������� */
  for (iptr = Ind, i = 0; i < N - 1; i++)
  {
    for (j = 0; j < M; j++)
    {
      /* ������� */
      *iptr++ = i * M + j + M;
      /* ������ */
      *iptr++ = i * M + j;
    }
    /* ��������� ������ ������� ��������� */
    *iptr++ = 0xFFFFFFFF;
  }

  AO5_PrimCreate(Prim, AO5_PRIM_GRID, M * N, (2 * M + 1) * (N - 1), V, Ind);

  free(V);

  return TRUE;
} /* End of 'AO5_PrimCreateSphere' function */

/* ������� �������� ��������� ����� �����.
 * ���������:
 *   - ��������� �� ��������:
 *       AO5PRIM *Prim;
 *   - ������� �����:
 *       VEC Loc;
 *   - ����������� �������-�������:
 *       VEC Du, Dv;
 *   - ������� �� ������:
 *       DBL Scale;
 *   - ��� ����� � ��������:
 *       CHAR *FileName;
 * ������������ ��������:
 *   (BOOL) TRUE ��� ������, ����� FALSE.
 */
BOOL AO5_PrimCreateHeightField( ao5PRIM *Prim, VEC Loc, VEC Du, VEC Dv, DBL Scale, CHAR *FileName )
{
  INT i, j;
  VEC norm;
  INT *Ind, *iptr;
  ao5VERTEX *V, *ptr;
  INT N, M;
  IMAGE Img;

  memset(Prim, 0, sizeof(ao5PRIM));

  if (!ImageLoad(&Img, FileName))
    return FALSE;
  M = Img.W;
  N = Img.H;

  if ((V = malloc(sizeof(ao5VERTEX) * N * M +
                  sizeof(INT) * ((2 * M + 1) * (N - 1)))) == NULL)
    return FALSE;
  Ind = (INT *)(V + N * M);

  /* ��������� ������� */
  norm = VecNormalize(VecCrossVec(Du, Dv));
  for (ptr = V, i = 0; i < N; i++)
    for (j = 0; j < M; j++, ptr++)
    {
      DWORD icolor = ImageGetP(&Img, j, i);
      DBL
        r = ((icolor >> 16) & 0xFF) / 255.0,
        g = ((icolor >> 8) & 0xFF) / 255.0,
        b = ((icolor >> 0) & 0xFF) / 255.0,
        H = r * 0.30 + g * 0.59 + b * 0.11;

      ptr->P = VecAddVec(Loc,
                 VecAddVec(VecAddVec(
                             VecMulNum(Du, j / (M - 1.0)),
                             VecMulNum(Dv, i / (N - 1.0))),
                             VecMulNum(norm, H * Scale)));
      ptr->C = AO5_RndPrimDefaultColor;
      ptr->N = norm;
      ptr->T = UVSet(j / (M - 1.0), i / (N - 1.0));
    }
  ImageFree(&Img);
  /* ��������� ������� */
  for (iptr = Ind, i = 0; i < N - 1; i++)
  {
    for (j = 0; j < M; j++)
    {
      /* ������� */
      *iptr++ = i * M + j + M;
      /* ������ */
      *iptr++ = i * M + j;
    }
    /* ��������� ������ ������� ��������� */
    *iptr++ = 0xFFFFFFFF;
  }

  /* ���������� �������� */
  
  /* �������� ������� ���� ������ */
  for (i = 0; i < N * M; i++)
    V[i].N = VecSet(0, 0, 0);

  /* ��������� �� ���� ������������� */
  for (i = 0; i < N - 1; i++)
    for (j = 0; j < M - 1; j++)
    {
      ao5VERTEX
        *Vij = V + i * M + j,
        *Vi1j = V + (i + 1) * M + j,
        *Vij1 = V + i * M + j + 1,
        *Vi1j1 = V + (i + 1) * M + j + 1;

      /* ������� ����������� */
      norm = VecNormalize(VecCrossVec(VecSubVec(Vij->P, Vi1j->P), VecSubVec(Vi1j1->P, Vi1j->P)));
      Vij->N = VecAddVec(Vij->N, norm);
      Vi1j->N = VecAddVec(Vi1j->N, norm);
      Vi1j1->N = VecAddVec(Vi1j1->N, norm);

      /* ������ ����������� */
      norm = VecNormalize(VecCrossVec(VecSubVec(Vi1j1->P, Vij1->P), VecSubVec(Vij->P, Vij1->P)));
      Vij->N = VecAddVec(Vij->N, norm);
      Vij1->N = VecAddVec(Vij1->N, norm);
      Vi1j1->N = VecAddVec(Vi1j1->N, norm);
    }

  /* ��������� ������� ���� ������ */
  for (i = 0; i < N * M; i++)
    V[i].N = VecNormalize(V[i].N);

  AO5_PrimCreate(Prim, AO5_PRIM_GRID, M * N, (2 * M + 1) * (N - 1), V, Ind);

  free(V);

  return TRUE;
} /* End of 'AO5_PrimCreatePlane' function */


/* END OF 'PRIM.C' FILE */
