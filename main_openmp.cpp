/*
 * main.cpp
 *
 *  Created on: Jun 5, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#include "IShape.h"
#include "Vector3D.h"
#include "Color.h"
#include "Light.h"
#include "util.h"

#include <set>
#include <math.h>
#include <string>
#include <iostream>
#include <sys/time.h>

#define MAX_DEPTH 20

extern const float bias;


int main (int argc, char** argv)
{
  timeval t_start, t_end;
  double elapsed_time;

	if (argc < 3)
	{
		printf ("Usage: ./RayTracerT3 <widht> <height> [<fov>]\n");
		return 0;
	}

	int width = atoi (argv[1]);;
	int height = atoi (argv[2]);;


	float fov = 60.0;
	if (argc >= 4)
	{
		fov = atof (argv[3]);
	}

	std::set<IShape *> sceneShapes;
	std::set<Light *> sceneLights;
	std::set<IShape *> *itemBuffer = new std::set<IShape *>[width * height];

	Point origin (0.0f, 5.0f, 20.0f);

	initScene (sceneShapes, sceneLights, origin, width, height, fov, itemBuffer);

	float tanFov = tan (fov * 0.5 * M_PI / 180.0f);

	float aspectratio = float (width) / float (height);

	std::string filename = "output/openmp.ppm";
	Image *image = new Image[width * height];

	printf ("Rendering scene:\n");
	printf ("Width: %d \nHeight: %d\nFov: %.2f\n", width, height, fov);

  	gettimeofday (&t_start, NULL);

  	double avg_d = 0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:avg_d)
#endif
	for (int y = 0; y < height; y++)
	{
		printf ("\r%.2f%%", float(y)/height * 100);
		int sum_d = 0;

		for (int x = 0; x < width; x++)
		{
      		Color color;
			float yu = (1 - 2 * ((y + 0.5) * 1 / height)) * tanFov;
			float xu = (2 * ((x + 0.5) * 1 / float (width)) - 1) * tanFov
					* aspectratio;
			Ray ray (origin, Vector3D (xu, yu, -1));
			int rtn_d = 0;
			color = trace (ray, sceneShapes, sceneLights, 0, &rtn_d, itemBuffer[y * width + x]);
			sum_d += rtn_d;
      		image[y * width + x] = color;
		}
		avg_d += (double)sum_d/width;
	}

  gettimeofday (&t_end, NULL);
  elapsed_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
  elapsed_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0;

  printf ("\r100.00%%");
	printf ("\nFinished!\n");
  printf ("Rendering time: %.3f s\n", elapsed_time/1000.0);
  	avg_d /= height;
	printf("Average depth: %f\n", avg_d);

  writePPMFile (image, filename.c_str (), width, height);

	std::set<IShape*>::iterator it = sceneShapes.begin ();
	while (it != sceneShapes.end ())
	{
		free (*it);
		sceneShapes.erase (it);
		it++;
	}

	std::set<Light*>::iterator it2 = sceneLights.begin ();
	while (it2 != sceneLights.end ())
	{
		free (*it2);
		sceneLights.erase (it2);
		it2++;
	}

	delete image;
	delete [] itemBuffer;
	return 0;
}
