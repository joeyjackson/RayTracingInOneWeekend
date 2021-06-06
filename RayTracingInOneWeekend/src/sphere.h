#pragma once

#include <memory>
#include "material.h"
#include "hittable.h"

class sphere : public hittable {
public:
    sphere() {}
    sphere(point3 cen, double r, std::shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

public:
    point3 center;
    double radius;
	std::shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) {
		return false;
	} 
	double sqrtd = std::sqrt(discriminant);
	double t = (-half_b - sqrtd) / a;
	if (t < t_min || t_max < t) {
		t = (-half_b + sqrtd) / a;
		if (t < t_min || t_max < t) {
			return false;
		}
	}

	rec.t = t;
	rec.p = r.at(rec.t);
	vec3 outward_normal = (rec.p - center) / radius;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;
	return true;
}