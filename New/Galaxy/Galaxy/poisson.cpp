#include "poisson.h"


bool poisson1d(int numIter, float min, float max, int n, float *data, float (*f)(float x))
{
	if (numIter <= 0)	return false;
	if (min >= max)		return false;
	if (n < 3)			return false;
	if (!data)			return false;
	if (!f)				return false;


	float h = (max - min) / (n - 1);
	float h2 = h * h;
	float nu;
	float relax = 1.5f;

	for (int iter = 0; iter < numIter; iter++)
	{
		for (int i = 1; i < n - 1; i++)
		{
			nu = 0.5f * (data[i - 1] + data[i + 1] - h2 * f(min + i * h));
			data[i] += relax * (nu - data[i]);
		}
	}

	return true;
}



bool poisson2d(int numIter, float min, float max, int n, float **data, float (*f)(float x, float y))
{
	if (numIter <= 0)	return false;
	if (min >= max)		return false;
	if (n < 3)			return false;
	if (!data)			return false;
	if (!f)				return false;


	float h = (max - min) / (n - 1);
	float h2 = h * h;
	float nu;
	float relax = 1.3f;

	for (int iter = 0; iter < numIter; iter++)
	{
		for (int i = 1; i < n - 1; i++)
		{
			for (int j = 1; j < n - 1; j++)
			{
				nu = 0.25f * (data[i - 1][j] + data[i + 1][j] + data[i][j - 1] + data[i][j + 1] - h2 * f(min + i * h, min + j * h));
				data[i][j] += relax * (nu - data[i][j]);
			}
		}
	}

	return true;
}



bool poisson3d(int numIter, float min, float max, int n, float ***data, float (*f)(float x, float y, float z))
{
	if (numIter <= 0)	return false;
	if (min >= max)		return false;
	if (n < 3)			return false;
	if (!data)			return false;
	if (!f)				return false;


	float h = (max - min) / (n - 1);
	float h2 = h * h;
	float nu;
	float relax = 1.3f;
	float factor = 1.0f / 6.0f;

	for (int iter = 0; iter < numIter; iter++)
	{
		for (int i = 1; i < n - 1; i++)
		{
			for (int j = 1; j < n - 1; j++)
			{
				for (int k = 1; k < n - 1; k++)
				{
					nu = data[i - 1][j][k] + data[i + 1][j][k] +
						 data[i][j - 1][k] + data[i][j + 1][k] +
						 data[i][j][k - 1] + data[i][j][k + 1];
					nu -= h2 * f(min + i * h, min + j * h, min + k * h);
					nu *= factor;
					data[i][j][k] += relax * (nu - data[i][j][k]);
				}
			}
		}
	}


	return true;
}