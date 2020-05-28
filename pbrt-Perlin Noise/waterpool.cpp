
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// Modification to Sphere Shape to Implement WaterPool*
// by Kevin M. Smith 3-2-2019
//

// shapes/sphere.cpp*
#include "shapes/waterpool.h"
#include "efloat.h"
#include "paramset.h"
#include "sampling.h"
#include "stats.h"

namespace pbrt {

// Sphere Method Definitions
Bounds3f WaterPool::ObjectBound() const {
    return Bounds3f(Point3f(-radius, -radius/4, zMin),		//looks like a box, so changed the y value to be smaller so that it looks
                    Point3f(radius, radius/4, zMax));		//more like a flat box or "plane" if you will
}

//  Note: create PBRT parameters for these
//

#define MAX_RAY_STEPS 1000
#define DIST_THRESHOLD .01
#define MAX_DISTANCE 100
#define NORMAL_EPS .01

//  Template Method
//
bool WaterPool::Intersect(const Ray &r, Float *tHit, SurfaceInteraction *isect,
                           bool testAlphaTexture) const {	

    Vector3f dir = Normalize(r.d);  // ray direction vectors are not normalized
                                    // in PBRT by default (KMS)
    bool hit = false;
	

	//the ray marching algorithm
    Point3f point = r.o;
    for (int i = 0; i < (int)maxray; i++) {
        Float dist = sdf(point);  
        
        if (dist < distthres) 
		{
            hit = true;
            break;
		}
		else if (dist > maxdist)
		{
            break;
		}
		else
		{
			point += dir*dist;
		}
    }

	Vector3f dpdu = Vector3f(0, 0, 0);
	Vector3f dpdv = Vector3f(0, 0, 0);
    Vector3f defaultNorm = Normalize(Vector3f(0, 1, 0));
    Vector3f normal = GetNormalRM(point, (float)eps, defaultNorm);								
    CoordinateSystem(normal, &dpdu, &dpdv);								//messing with these won't render anything 

	float mult = 10 * (float)distthres;
	Vector3f pError(mult*point.x, mult*point.y, mult*point.z);	//(10*(float)distthres)*(Vector3f)point

    if (hit && tHit != nullptr && isect != nullptr) {
        // Thiis where you return your SurfaceInteraction structure and your
        // tHit Important Note: You must check for null pointer as Intersect is
        // called by IntersectP() with null values for these parameters.
        *isect = (*ObjectToWorld)(
            SurfaceInteraction(point, pError, Point2f(0, 0), -r.d, dpdu, dpdv,
                               Normal3f(0, 0, 0), Normal3f(0, 0, 0), r.time, this));
		
		Vector3f distance = point - Point3f(0, 0, 0);
		*tHit = distance.Length();
		
    }
    return hit;
}

//  Template Method
//
Float WaterPool::sdf(const Point3f &pos) const {
	Float noise = 0;
	Point3f located = Point3f(0, -2, 0);		//the location of the waterpool
	Float ampl = amplitude;
	Float freq = frequency;
	for (int i = 0; i < octave; i++)
	{
		noise += ampl/2 * (Noise(freq * pos));
		ampl /= 2;
		freq *= 2;
	}
	return pos.y - (located.y + noise);
}

// Get Normal using Gradient (Finite Distances Methods )  - See class slides.
//  Note if the normal you calculate has zero length, return the defaultNormal
//
Vector3f WaterPool::GetNormalRM(const Point3f &p, float eps,
                                 const Vector3f &defaultNormal) const {

	Float dp = sdf(p);
    Vector3f n(dp - sdf(Point3f(p.x - eps, p.y, p.z)),
               dp - sdf(Point3f(p.x, p.y - eps, p.z)),
               dp - sdf(Point3f(p.x, p.y, p.z - eps)));

	if (n.Length() == 0)
	{
		printf("defaultNormal ");
        return defaultNormal;
	}
	else
	{
        return Normalize(n);
	}
}

Float WaterPool::Area() const { return phiMax * radius * (zMax - zMin); }

// These functions are stubbed
//
Interaction WaterPool::Sample(const Point2f &u, Float *pdf) const {
    LOG(FATAL) << "WaterPool::Sample not implemented.";
    return Interaction();
}

Interaction WaterPool::Sample(const Interaction &ref, const Point2f &u,
                               Float *pdf) const {
    LOG(FATAL) << "WaterPool::Sample not implemented.";
    return Interaction();
}

std::shared_ptr<Shape> CreateWaterPoolShape(const Transform *o2w,
                                             const Transform *w2o,
                                             bool reverseOrientation,
                                             const ParamSet &params) {
    Float radius = params.FindOneFloat("radius", 1.f);
    Float zmin = params.FindOneFloat("zmin", -radius);
    Float zmax = params.FindOneFloat("zmax", radius);
    Float phimax = params.FindOneFloat("phimax", 360.f);
    Float maxray = params.FindOneFloat("maxray", 1000);
    Float distthres = params.FindOneFloat("distthres", 0.01);
    Float maxdist = params.FindOneFloat("maxdist", 100);
    Float eps = params.FindOneFloat("eps", 0.01);
	Float amplitude = params.FindOneFloat("amplitude", 3.0);
	Float frequency = params.FindOneFloat("frequency", 0.08);
	int octave = params.FindOneInt("octave", 8);
    return std::make_shared<WaterPool>(o2w, w2o, reverseOrientation, radius,
                                        zmin, zmax, phimax, maxray, distthres, maxdist, eps, amplitude, frequency, octave);
}

}  // namespace pbrt
