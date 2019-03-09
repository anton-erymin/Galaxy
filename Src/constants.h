#pragma once

constexpr uint32_t cWindowWidth = 1400;
constexpr uint32_t cWindowHeight = 800;

constexpr char* cWindowCaption = "Galaxy Model 0.1";

/* 
Measure units:
Length - kiloparsec
Mass - 10^10 solar masses
Time - 4.7 million years
Gravitational constant in these units is 1.
*/

constexpr double cG = 6.67408e-11;                // m^3 / (kg * sec^2)
constexpr double cParsec = 3.08567758149137e+16;  // m
constexpr double cKiloParsec = 1000 * cParsec;    // m
constexpr double cSolarMass = 1.9885e+30;         // kg
constexpr double cMassUnit = 1e+10 * cSolarMass;  // kg

constexpr float cSoftFactor = 0.05f;

constexpr float cRenderFps = 30;
constexpr float cFrameTime = 1.0f / cRenderFps;

// Параметры по умолчанию для стандартной модели

#define         GLX_TOTAL_MASS          500.0f;

#define			GLX_DISK_RADIUS			15.0f						// Радиус галактики
#define			GLX_BULGE_RADIUS		1.0f						// Радиус ядра галактики
#define			GLX_HALO_RADIUS			7.0f						// Радиус гало из темной материи
#define			GLX_DISK_THICKNESS		0.0f						// Толщина диска галактики
#define         GLX_DISK_MASS_RATIO     0.5f

#define			GLX_HALO_MASS			20.0f						// Масса гало

#define			GLX_UNIVERSE_SIZE			50.0f						// Размеры пространства симуляции

#define			GLX_BULGE_NUM			3000						// Количесто частиц в балдже
#define			GLX_DISK_NUM			7000						// Количесто частиц в диске