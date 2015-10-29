#include "color.hpp"
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

	Color Color::operator + (const Color& c) {
		return Color(rgb[0] + c.r(), rgb[1] + c.g(), rgb[2] + c.b());
	};

	Color Color::operator - (const Color& c) {
		return Color(rgb[0] - c.r(), rgb[1] - c.g(), rgb[2] - c.b());
	};

	Color Color::operator * (float f) {
		return Color(rgb[0] * f, rgb[1] * f, rgb[2] * f);
	};

	Color Color::operator / (float f) {
		return Color(rgb[0] / f, rgb[1] / f, rgb[2] / f);
	};

	bool Color::operator == (const Color& c) {
		return rgb[0] == c.r() && rgb[1] == c.g() && rgb[2] == c.b();
	};
	Color Color::Modulate(const Color& c) {
		return Color(rgb[0] * c.r(), rgb[1] * c.g(), rgb[2] * c.b());
	};


	const PtrColor Color::Black = std::make_shared<Color>(0.f, 0.f, 0.f);
	const PtrColor Color::White = std::make_shared<Color>(1.0f, 1.0f, 1.0f);
	const PtrColor Color::Red = std::make_shared<Color>(1.0f, 0.f, 0.f);
	const PtrColor Color::Green = std::make_shared<Color>(0.f, 1.0f, 0.f);
	const PtrColor Color::Blue = std::make_shared<Color>(0.f, 0.f, 1.0f);


}