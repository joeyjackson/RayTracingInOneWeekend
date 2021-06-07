#pragma once

#include <iostream>
#include "vec3.h"
#include "util.h"
#include "image.h"

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    double scale = 1.0 / samples_per_pixel;
    r = std::sqrt(r * scale);
    g = std::sqrt(g * scale);
    b = std::sqrt(b * scale);

    out << static_cast<int>(256 * clamp(r, 0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0, 0.999)) << '\n';
}

bool write_color_to_image(image& im, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    double scale = 1.0 / samples_per_pixel;
    r = std::sqrt(r * scale);
    g = std::sqrt(g * scale);
    b = std::sqrt(b * scale);

    return im.addPixel(
        static_cast<unsigned char>(256 * clamp(r, 0, 0.999)),
        static_cast<unsigned char>(256 * clamp(g, 0, 0.999)),
        static_cast<unsigned char>(256 * clamp(b, 0, 0.999))
    );
}