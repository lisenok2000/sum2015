/* FILENAME: UWORLD.C
 * PROGRAMMER: AO5
 * PURPOSE: World view unit handle module.
 * LAST UPDATE: 10.06.2015
 */

#include <string.h>

#include "anim.h"


typedef struct tagao5UNIT_WORLD
{
  AO5_UNIT_BASE_FIELDS;

  ao5PRIM Sky; /* ������ ��� ����������� ���� */
  ao5PRIM Water; /* ������ ��� ����������� ���� */
  ao5PRIM Island[2]; /* ������ ��� ����������� ������� */
  INT NumOfIsl;         
  INT TextId;  /* Id �������� */
} ao5UNIT_WORLD;
/* ������� ��-��������� ������������� ������� ��������.
 * ���������:
 *   - ��������� �� "����" - ��� ������ ��������:
 *       ao5UNIT *Uni;
 *   - ��������� �� �������� ��������:
 *       ao5ANIM *Ani;
 * ������������ ��������: ���.
 */
static VOID AO5_AnimUnitInit( ao5UNIT_WORLD *Uni, ao5ANIM *Ani )
{
  INT i;
  Uni->NumOfIsl = 2;
  AO5_MtlLib[1].TexId = AO5_TextureLoad("sky.bmp");
  AO5_MtlLib[2].TexId = AO5_TextureLoad("water3.bmp");
  AO5_MtlLib[3].TexId = AO5_TextureLoad("TEX1.bmp");
  AO5_MtlLib[4].TexId = AO5_TextureLoad("grass.bmp");
  Uni->Sky.Prog = AO5_ShaderLoad("SKY");
  Uni->Water.Prog = AO5_ShaderLoad("WATER");
  Uni->Island[0].Prog = AO5_ShaderLoad("ISLANDS");
  for(i = 1; i < Uni->NumOfIsl; i++)
    Uni->Island[i].Prog = Uni->Island[0].Prog;
  AO5_PrimCreateHeightField(&Uni->Water, VecSet(-50, 0, 50), VecSet(0, 0, -100), VecSet(100, 0, 0), 1, "water10.bmp");
  AO5_PrimCreateHeightField(&Uni->Island[0], VecSet(-20, 0, -20), VecSet(0, 0, -40), VecSet(40, 0, 0), -15, "NHM1C.bmp");
  AO5_PrimCreateHeightField(&Uni->Island[1], VecSet(10, 0.2, 20), VecSet(0, 0, -50), VecSet(50, 0, 0), -5, "NHM5C.bmp");
  AO5_PrimCreateSphere(&Uni->Sky, VecSet(0, 0, 0), 500, 30, 30);
  Uni->Sky.MtlNo = 1;
  Uni->Water.MtlNo = 2;
  Uni->Island[0].MtlNo = 3;
  Uni->Island[1].MtlNo = 4;
} /* End of 'AO5_AnimUnitInit' function */

/* ������� ��-��������� ��������������� ������� ��������.
 * ���������:
 *   - ��������� �� "����" - ��� ������ ��������:
 *       ao5UNIT *Uni;
 *   - ��������� �� �������� ��������:
 *       ao5ANIM *Ani;
 * ������������ ��������: ���.
 */
static VOID AO5_AnimUnitClose( ao5UNIT_WORLD *Uni, ao5ANIM *Ani )
{
  INT i;
  AO5_ShaderFree(Uni->Sky.Prog);
  AO5_PrimFree(&Uni->Sky);
  AO5_ShaderFree(Uni->Water.Prog);
  AO5_PrimFree(&Uni->Water);
  AO5_ShaderFree(Uni->Island[0].Prog);
  for(i = 0; i < Uni->NumOfIsl; i++)
    AO5_PrimFree(&Uni->Island[i]);
} /* End of 'AO5_AnimUnitClose' function */

/* ������� ��-��������� ���������� ����������� ���������� ������� ��������.
 * ���������:
 *   - ��������� �� "����" - ��� ������ ��������:
 *       ao5UNIT *Uni;
 *   - ��������� �� �������� ��������:
 *       ao5ANIM *Ani;
 * ������������ ��������: ���.
 */
static VOID AO5_AnimUnitResponse( ao5UNIT_WORLD *Uni, ao5ANIM *Ani )
{
} /* End of 'AO5_AnimUnitResponse' function */

/* ������� ��-��������� ���������� ������� ��������.
 * ���������:
 *   - ��������� �� "����" - ��� ������ ��������:
 *       ao5UNIT *Uni;
 *   - ��������� �� �������� ��������:
 *       ao5ANIM *Ani;
 * ������������ ��������: ���.
 */
static VOID AO5_AnimUnitRender( ao5UNIT_WORLD *Uni, ao5ANIM *Ani )
{
  INT i;
  static DBL time = 5;
  time += AO5_Anim.GlobalDeltaTime;
  if (time > 5)
    {
      time = 0;
      AO5_ShaderFree(Uni->Sky.Prog);
      Uni->Sky.Prog = AO5_ShaderLoad("SKY");
      AO5_ShaderFree(Uni->Water.Prog);
      Uni->Water.Prog = AO5_ShaderLoad("WATER");
      AO5_ShaderFree(Uni->Island[0].Prog);
      Uni->Island[0].Prog = AO5_ShaderLoad("ISLANDS");
      for(i = 0; i < Uni->NumOfIsl; i++) 
        Uni->Island[i].Prog = Uni->Island[0].Prog;
    }
  AO5_PrimDraw(&Uni->Sky);
  AO5_PrimDraw(&Uni->Water);
  for(i = 0; i < Uni->NumOfIsl; i++) 
    AO5_PrimDraw(&Uni->Island[i]);
} /* End of 'AO5_AnimUnitRender' function */

/* ������� �������� ���� ��������.
 * ���������:
 *   - ������ ��������� ������� ��������:
 *       INT Size;
 * ������������ ��������:
 *   (ao5UNIT *) ��������� �� ��������� ������ ��������.
 */
ao5UNIT * AO5_UnitWorldCreate( VOID )
{
  ao5UNIT *Uni;

  if ((Uni = (VOID *)AO5_AnimUnitCreate(sizeof(ao5UNIT_WORLD))) == NULL)
    return NULL;
  /* ��������� ���� ��-��������� */
  Uni->Init = AO5_AnimUnitInit;
  Uni->Close = AO5_AnimUnitClose;
  Uni->Response = AO5_AnimUnitResponse;
  Uni->Render = AO5_AnimUnitRender;
  return Uni;
} /* End of 'AO5_AnimUnitCreate' function */

/* END OF 'UWORLD.C' FILE */
