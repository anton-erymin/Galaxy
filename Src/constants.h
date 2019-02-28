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

constexpr double cG = 6.67408e-11;                // m^3 / (kg * sec^2)
constexpr double cParsec = 3.08567758149137e+16;  // m
constexpr double cKiloParsec = 1000 * cParsec;    // m
constexpr double cSolarMass = 1.9885e+30;         // kg
constexpr double cMassUnit = 1e+10 * cSolarMass;  // kg

// Параметры по умолчанию для стандартной модели

#define			GLX_DISK_RADIUS			15.0f						// Радиус галактики
#define			GLX_BULGE_RADIUS		0.5f						// Радиус ядра галактики
#define			GLX_HALO_RADIUS			15.0f						// Радиус гало из темной материи
#define			GLX_DISK_THICKNESS		0.3f						// Толщина диска галактики

#define	 		GLX_STAR_MASS			1.0e-7f						// Масса звезды
#define			GLX_BULGE_MASS			1.0f						// Масса балджа
#define			GLX_HALO_MASS			20.0f						// Масса гало

#define			UNIVERSE_SIZE			50.0f						// Размеры пространства симуляции

#define			GLX_BULGE_NUM			200						// Количесто частиц в балдже
#define			GLX_DISK_NUM			10000						// Количесто частиц в диске