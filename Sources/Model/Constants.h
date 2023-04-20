#pragma once

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
constexpr double cSecondsPerHour = 3600.0f;
constexpr double cHoursPerDay = 24.0f;
constexpr double cDaysPerYear = 365.0f;

constexpr float cSoftFactor = 0.02f;
constexpr float cDefaultOpeningAngle = 0.5f;

constexpr float cRadialVelocityFactor = 0.7f;

constexpr float GLX_TOTAL_MASS          = 1.0f;
constexpr float GLX_DISK_RADIUS			= 0.1f;//5.0f		// Disk radius
constexpr float GLX_BULGE_RADIUS		= 0.01f;			// Bulge radius
constexpr float GLX_HALO_RADIUS			= 5.0f;				// Halo radius
constexpr float GLX_DISK_THICKNESS		= 0.000f;			// Disk thickness
constexpr float GLX_DISK_MASS_RATIO     = 1.0f;             // Disk mass ratio
constexpr float GLX_HALO_MASS			= 20.0f;			// Halo mass
constexpr float GLX_BLACK_HOLE_MASS     = 0.0f;             // Black hole mass
constexpr size_t GLX_BULGE_NUM			= 0;				// Number of particles in bulge
constexpr size_t GLX_DISK_NUM			= 50;				// Number of particles in disk
