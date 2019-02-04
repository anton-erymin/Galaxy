#pragma once


// Константы



#define		GLX_RENDER_GALO			1
#define		GLX_RENDER_DISK			2
#define		GLX_RENDER_TREE			4

#define		GLX_RENDER_ALL			7



/* Единицы измерения:
длина - килопарсек
масса - 10^10 солнечных масс
время - 4.7 млн. лет
Гравитационная постоянная в этих единницах G = 1
*/


// Параметры по умолчанию для стандартной модели

#define			GLX_DISK_RADIUS			15.0f						// Радиус галактики
#define			GLX_BULGE_RADIUS		0.5f						// Радиус ядра галактики
#define			GLX_HALO_RADIUS			30.0f						// Радиус гало из темной материи
#define			GLX_DISK_THICKNESS		0.3f						// Толщина диска галактики

#define	 		GLX_STAR_MASS			1.0e-7f						// Масса звезды
#define			GLX_BULGE_MASS			1.0f						// Масса балджа
#define			GLX_HALO_MASS			20.0f						// Масса гало

#define			UNIVERSE_SIZE			400.0f						// Размеры пространства симуляции

#define			GLX_BULGE_NUM			200						// Количесто частиц в балдже
#define			GLX_DISK_NUM			1000						// Количесто частиц в диске