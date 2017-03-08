#include "stdafx.h"
#include "color.hpp"
#include "defines.hpp"

namespace renderer {
	Color::Color() {
	};

	Color::Color(float r, float g, float b) {
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
	};

	Color::Color(const Color& c) {
		rgb[0] = c.r();
		rgb[1] = c.g();
		rgb[2] = c.b();
	};
	Color::Color(Color&& c) {
		rgb[0] = c.r();
		rgb[1] = c.g();
		rgb[2] = c.b();
	};

	Color& Color::operator = (const Color& c) {
		rgb[0] = c.r();
		rgb[1] = c.g();
		rgb[2] = c.b();
		return *this;
	};

	Color& Color::operator = (Color&& c) {
		rgb[0] = c.r();
		rgb[1] = c.g();
		rgb[2] = c.b();
		return *this;
	};

	Color::~Color() {};

	Color Color::operator + (const Color& c) const {
		return Color(rgb[0] + c.r(), rgb[1] + c.g(), rgb[2] + c.b());
	};

	Color Color::operator - (const Color& c) const {
		return Color(rgb[0] - c.r(), rgb[1] - c.g(), rgb[2] - c.b());
	};

	Color Color::operator * (float f) const {
		return Color(rgb[0] * f, rgb[1] * f, rgb[2] * f);
	};

	Color Color::operator / (float f) const {
		return Color(rgb[0] / f, rgb[1] / f, rgb[2] / f);
	};

	Color& Color::operator += (const Color& c) {
		rgb[0] += c.rgb[0];
		rgb[1] += c.rgb[1];
		rgb[2] += c.rgb[2];
		return *this;
	}

	Color& Color::operator *= (float f) {
		rgb[0] *= f;
		rgb[1] *= f;
		rgb[2] *= f;
		return *this;
	}

	// TODO float comparison
	bool Color::operator == (const Color& c) {
		return r() == c.r() && g() == c.g() && b() == c.b();
	};

	Color Color::Modulate(const Color& c) const {
		return Color(r() * c.r(), g() * c.g(), b() * c.b());
	};
	Color Color::clamp() const {
		return Color(
			Clamp(rgb[0], 0.f, 1.f),
			Clamp(rgb[1], 0.f, 1.f),
			Clamp(rgb[2], 0.f, 1.f));
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