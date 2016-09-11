#ifndef RENDERER_COLOR_HPP
#define RENDERER_COLOR_HPP

#include "base.hpp"

namespace renderer {
	class Color {
		float rgb[3] = { 0 };
	public:
		Color();
		Color(float r, float g, float b);
		Color(const Color &);
		Color(Color &&);
		Color& operator = (const Color&);
		Color& operator = (Color&&);
		~Color();
		Color operator + (const Color&) const;
		Color operator - (const Color&) const;
		Color operator * (float f) const;
		Color operator / (float f) const;
		Color& operator += (const Color& p);
		Color& operator *= (float f);
		bool operator == (const Color&);
		Color Modulate (const Color&) const;
		Color clamp() const;
		inline float r() const { return rgb[0]; };
		inline float g() const { return rgb[1]; };
		inline float b() const { return rgb[2]; };
		int rInt() const;
		int gInt() const;
		int bInt() const;
	public:
		static const Color Black;
		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
	};
}

#endif // RENDERER_COLOR_HPP
