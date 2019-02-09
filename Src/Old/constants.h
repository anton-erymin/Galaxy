#pragma once


// ���������



#define		GLX_RENDER_GALO			1
#define		GLX_RENDER_DISK			2
#define		GLX_RENDER_TREE			4

#define		GLX_RENDER_ALL			7



/* ������� ���������:
����� - ����������
����� - 10^10 ��������� ����
����� - 4.7 ���. ���
�������������� ���������� � ���� ��������� G = 1
*/


// ��������� �� ��������� ��� ����������� ������

#define			GLX_DISK_RADIUS			15.0f						// ������ ���������
#define			GLX_BULGE_RADIUS		0.5f						// ������ ���� ���������
#define			GLX_HALO_RADIUS			30.0f						// ������ ���� �� ������ �������
#define			GLX_DISK_THICKNESS		0.3f						// ������� ����� ���������

#define	 		GLX_STAR_MASS			1.0e-7f						// ����� ������
#define			GLX_BULGE_MASS			1.0f						// ����� ������
#define			GLX_HALO_MASS			20.0f						// ����� ����

#define			UNIVERSE_SIZE			400.0f						// ������� ������������ ���������

#define			GLX_BULGE_NUM			200						// ��������� ������ � ������
#define			GLX_DISK_NUM			1000						// ��������� ������ � �����