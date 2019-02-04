#pragma once


float integrate_rect(float a, float b, int n, float (*f)(float));
float integrate_trap(float a, float b, int n, float (*f)(float));