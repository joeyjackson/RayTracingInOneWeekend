#pragma once

#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class image {
public:
	image(int w, int h) : width(w), height(h), idx(0), m_size(w * h * 3) {
		data = new unsigned char[m_size];
	}

	image(const image& old) = delete;
	image& operator=(const image& old) = delete;

	~image() {
		delete[] data;
	}

	bool addByte(unsigned char b) {
		if (idx >= m_size) {
			return false;
		}
		data[idx++] = b;
		return true;
	}

	bool addPixel(unsigned char r, unsigned char g, unsigned char b) {
		return addByte(r) && addByte(g) && addByte(b);
	}

	void outPPM(std::ostream& out) {
		out << "P3\n" << width << " " << height << "\n255\n";
		for (int i = 0; i < m_size; i += 3) {
			out << static_cast<int>(data[i]) << ' ' 
				<< static_cast<int>(data[i + 1]) << ' '
				<< static_cast<int>(data[i + 2]) << '\n';
		}
	}

	bool outPNG(const char* filename) {
		return stbi_write_png(filename, width, height, 3 /* RGB */, data, width * 3);
	}

private:
	int m_size;
	int width;
	int height;
	unsigned char* data;
	int idx;
};