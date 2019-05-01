/*
 * Sphere.h
 *
 *  Created on: Jun 6, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include "IShape.h"
#include "Vector3D.h"
#include <set>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <omp.h>

class Sphere : public IShape
{
public:
	Sphere (const Vector3D &position, const Color &color, const float &radius,
					const float &refl = 0.0f, float spec = 0.0f, float diff = 0.0f)
			: IShape (position, color, refl, spec, diff), _radius (radius)
	{
	}

	virtual ~Sphere ()
	{
	}

	virtual bool intersect (const Ray &ray, float *t, Vector3D& normal,
													Color &pixelColor)
	{
		Point line = _position - ray.origin ();
		float tca = line.dot (ray.direction ());

		if (tca < 0) return false;

		float d2 = line.length2 () - tca * tca;

		if (d2 > _radius * _radius) return false;

		float thc = sqrt (_radius * _radius - d2);

		float t0 = tca - thc;
		float t1 = tca + thc;

		if (t0 > t1) std::swap (t0, t1);

		if (t0 > ray.farDistance ()) return false;

		*t = t0;
		normal = (ray.calculate (t0) - _position).normalized ();
		pixelColor = _color;

		return true;
	}

	virtual void generateItemBuffer(std::set<IShape*> *itemBuffer, Point origin, int width, int height, float fov) {
		// Assume view plane is at (0,0,19), towards (0,0,-1)
		Vector3D plane_normal(0,0,-1);
		Point plane_point(0,0,19);
		Vector3D cone_axis = (this->_position - origin).normalized();
		float cosTheta = cone_axis.dot(plane_normal);
		float sinAlpha = this->_radius / (this->_position - origin).length();
		float tanFov = tan (fov * 0.5 * M_PI / 180.0f);
		float cosAlpha = sqrt(1 - sinAlpha * sinAlpha);

		float t = plane_normal.dot(plane_point - origin);
		float b = cosTheta * cosTheta - sinAlpha * sinAlpha;
		float h = t/b;

		Point center = origin + h*cosTheta*cone_axis - h*sinAlpha*sinAlpha*plane_normal;
		Vector3D majorAxis = (cone_axis - cosTheta * plane_normal).normalized();
		Vector3D minorAxis = plane_normal.cross(majorAxis);
		float majorRadius = abs(h)*sinAlpha*cosAlpha;
		float minorRadius = t * sinAlpha / sqrt(abs(b));

		if (sinAlpha >= cosTheta) {
			std::cerr<<"intersection not ellipse"<<std::endl;
			exit(1);
		} else {
			float r = majorRadius > minorRadius ? majorRadius : minorRadius;
			float ratio = float(width)/height;
			int left = ((center.x() - r)/tanFov/ratio + 1)/2*width;
			int right = ((center.x() + r)/tanFov/ratio + 1)/2*width + 1;
			int up  = height - ((center.y() + r - 5)/tanFov + 1)/2*height;
			int down = height - ((center.y() - r - 5)/tanFov + 1)/2*height + 1;
			#pragma omp parallel for
			for (int j = up; j <= down; j++) {
				for (int i = left; i <= right; i++) {
					itemBuffer[j*width + i].insert(this);
				}
			}
		}

	}

	float radius () const
	{
		return _radius;
	}

protected:
	float _radius;

};

#endif /* SPHERE_H_ */
