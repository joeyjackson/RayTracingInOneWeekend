#include <iostream>
#include <fstream>
#include <string>

#include "color.h"
#include "ray.h"
#include "util.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "vec3.h"
#include "material.h"
#include "image.h"

hittable_list random_scene() {
	hittable_list world;

	auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(std::make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				std::shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = std::make_shared<lambertian>(albedo);
					world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = std::make_shared<metal>(albedo, fuzz);
					world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = std::make_shared<dielectric>(1.5);
					world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = std::make_shared<dielectric>(1.5);
	world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

hittable_list simple_scene() {
	hittable_list world;

	auto ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto blue = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto glass = std::make_shared<dielectric>(1.5);
	auto brass = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

	world.add(std::make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, ground));
	world.add(std::make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, blue));
	world.add(std::make_shared<sphere>(point3(4.0, 0.0, 1.0), 0.5, glass));
	world.add(std::make_shared<sphere>(point3(4.0, 0.0, 1.0), -0.45, glass));
	world.add(std::make_shared<sphere>(point3(3.0, 0.0, -1.0), 0.5, brass));

	return world;
}

double hit_sphere(const point3& center, double radius, const ray& r) {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) {
		return -1;
	} else {
		return (-half_b - std::sqrt(discriminant)) / a;
	}
}

color ray_color(const ray& r, const hittable_list& world, int depth) {
	if (depth <= 0)
		return color(0, 0, 0);

	hit_record rec;

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	double t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}



enum file_type { PPM = 0, PNG };
static const char* file_type_strings[] = { "PPM", "PNG" };

struct config {
	std::string output_path;
	file_type output_type;

	void print_config() {
		std::cerr << "Output Path: " << (output_path == "" ? "stdout" : output_path) << std::endl;
		std::cerr << "Output Type: " << file_type_strings[output_type] << std::endl << std::endl;
	}

	bool validate() {
		if (output_path == "" && output_type == PNG) {
			std::cerr << "Must specify output file with PNG file type" << std::endl;
			return false;
		}
		return true;
	}
};

bool parseFlags(int argc, char** argv, config& conf) {
	conf.output_path = ""; // default stdout
	conf.output_type = PPM; // default PPM

	enum flag_type { NONE = 0, FILE_PATH, FILE_TYPE };
	static const char* flag_msg_string[] = { "", "-o", "-t" };
	flag_type curr_flag = NONE;
	// skip first arg (binary name)
	for (int i = 1; i < argc; ++i)
	{
		std::string token = argv[i];
		if (token.empty()) continue;
		if (token[0] == '-') {
			if (curr_flag != NONE) {
				std::cerr << "Missing flag value: " << flag_msg_string[curr_flag] << std::endl;
				return false;
			}
			if (token == "-o") {
				curr_flag = FILE_PATH;
				continue;
			}
			if (token == "-t") {
				curr_flag = FILE_TYPE;
				continue;
			}
			std::cerr << "Invalid flag: " << token << std::endl;
			return false;
		} else {
			switch (curr_flag)
			{
			case NONE:
				std::cerr << "Did not specify flag for value: " << token << std::endl;
				return false;
			case FILE_PATH:
				conf.output_path = token;
				curr_flag = NONE;
				break;
			case FILE_TYPE:
				if (token == "ppm") {
					conf.output_type = PPM;
				} else if (token == "png") {
					conf.output_type = PNG;
				} else {
					std::cerr << "invalid file type: " << token << "\t (expected: ppm, png)" << std::endl;
					return false;
				}
				curr_flag = NONE;
				break;
			default:
				std::cerr << "" << std::endl;
				return false;
			}
		}
	}
	if (curr_flag != NONE) {
		std::cerr << "Missing flag value: " << flag_msg_string[curr_flag] << std::endl;
		return false;
	}
}

int main(int argc, char** argv) {
	config conf;
	if (!parseFlags(argc, argv, conf)) {
		return 1;
	}
	if (!conf.validate()) {
		return 1;
	}
	conf.print_config();

	// Image
	const auto aspect_ratio = 3.0 / 2.0;
	const int image_width = 1200;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 500;
	const int max_depth = 50;

	// World
	auto world = random_scene();

	// Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;

	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	// Render
	std::cerr << "Rendering..." << std::endl;
	image im(image_width, image_height);

	for (int j = image_height - 1; j >= 0; j--) {
		std::cerr << "\rScanlines remaining: " << j << " " << std::flush;
		for (int i = 0; i < image_width; i++) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++) {
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			write_color_to_image(im, pixel_color, samples_per_pixel);
		}
	}
	std::cerr << "\nWriting..." << std::endl;
	switch (conf.output_type)
	{
	case PPM:
		if (conf.output_path == "") {
			im.outPPM(std::cout);
		} else {
			std::ofstream fh;
			fh.open(conf.output_path);
			im.outPPM(fh);
			fh.close();
		}
		break;
	case PNG:
		if (!im.outPNG(conf.output_path.c_str())) {
			std::cerr << "Error writing PNG" << std::endl;
			return 1;
		}
		break;
	}
	std::cerr << "Done." << std::endl;
	return 0;
}
