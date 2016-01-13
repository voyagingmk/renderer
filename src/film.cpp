#include "stdafx.h"
#include "film.hpp"

namespace renderer {
	void ImageFilm::set(int x, int y, int r, int g, int b) {
		img.atXYZC(x, y, 0, 0) = r;
		img.atXYZC(x, y, 0, 0) = g;
		img.atXYZC(x, y, 0, 0) = b;
	}

}