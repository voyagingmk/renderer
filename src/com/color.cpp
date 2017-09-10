#include "stdafx.h"
#include "com/color.hpp"
#include "utils/defines.hpp"

namespace renderer {
	Color::Color() {
	};

	Color::Color(float r, float g, float b, float a) {
		rgba[0] = r;
		rgba[1] = g;
        rgba[2] = b;
        rgba[3] = a;
	};

	Color::Color(const Color& c) {
		rgba[0] = c.r();
		rgba[1] = c.g();
        rgba[2] = c.b();
        rgba[3] = c.a();
	};
	Color::Color(Color&& c) {
		rgba[0] = c.r();
		rgba[1] = c.g();
        rgba[2] = c.b();
        rgba[3] = c.a();
	};

	Color& Color::operator = (const Color& c) {
		rgba[0] = c.r();
		rgba[1] = c.g();
        rgba[2] = c.b();
        rgba[3] = c.a();
		return *this;
	};

	Color& Color::operator = (Color&& c) {
		rgba[0] = c.r();
		rgba[1] = c.g();
        rgba[2] = c.b();
        rgba[3] = c.a();
		return *this;
	};

	Color::~Color() {};

	Color Color::operator + (const Color& c) const {
		return Color(rgba[0] + c.r(), rgba[1] + c.g(), rgba[2] + c.b(), rgba[3] + c.a());
	};

	Color Color::operator - (const Color& c) const {
		return Color(rgba[0] - c.r(), rgba[1] - c.g(), rgba[2] - c.b(), rgba[3] - c.a());
	};

	Color Color::operator * (float f) const {
		return Color(rgba[0] * f, rgba[1] * f, rgba[2] * f, rgba[3] * f);
	};

	Color Color::operator / (float f) const {
		const float _f = 1.0f / f;
		return Color(rgba[0] * _f, rgba[1] * _f, rgba[2] * _f, rgba[3] * _f);
	};

	Color& Color::operator += (const Color& c) {
		rgba[0] += c.rgba[0];
		rgba[1] += c.rgba[1];
        rgba[2] += c.rgba[2];
        rgba[3] += c.rgba[3];
		return *this;
	}

	Color& Color::operator *= (float f) {
		rgba[0] *= f;
		rgba[1] *= f;
        rgba[2] *= f;
        rgba[3] *= f;
        return *this;
	}

	// TODO float comparison
	bool Color::operator == (const Color& c) {
		return r() == c.r() && g() == c.g() && b() == c.b() && a() == c.a();
	};

	Color Color::Modulate(const Color& c) const {
		return Color(r() * c.r(), g() * c.g(), b() * c.b(), a() * c.a());
	};
	Color Color::clamp() const {
		return Color(
			Clamp(rgba[0], 0.f, 1.f),
			Clamp(rgba[1], 0.f, 1.f),
            Clamp(rgba[2], 0.f, 1.f),
            Clamp(rgba[3], 0.f, 1.f));
	};

	int Color::rInt() const {
		return toInt(r());
	}
	int Color::gInt() const {
		return toInt(g());
	}
	int Color::bInt() const {
		return toInt(b());
    }
    int Color::aInt() const {
        return toInt(a());
    }

	int Color::toInt(float v) const {
		if (Enable_GammaCorrect) {
			return Clamp(255.f * GammaCorrect(v), 0.f, 255.f);
		}
		if (Enable_HDR) {
			// ACESToneMapping
			const float adapted_lum = 1.f;
			const float A = 2.51f;
			const float B = 0.03f;
			const float C = 2.43f;
			const float D = 0.59f;
			const float E = 0.14f;
			v *= adapted_lum;
			v = (v * (A * v + B)) / (v * (C * v + D) + E);
		}
		return min(int(v * 255), 255);
	}

	const Color Color::Black = Color(0.f, 0.f, 0.f);
	const Color Color::White = Color(1.0f, 1.0f, 1.0f);
	const Color Color::Red = Color(1.0f, 0.f, 0.f);
	const Color Color::Green = Color(0.f, 1.0f, 0.f);
	const Color Color::Blue = Color(0.f, 0.f, 1.0f);


}
