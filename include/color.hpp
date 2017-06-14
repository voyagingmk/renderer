#ifndef RENDERER_COLOR_HPP
#define RENDERER_COLOR_HPP

#include "base.hpp"

namespace renderer {
	class Color {
		float rgba[4] = { 0 };
	public:
		Color();
		Color(float r, float g, float b, float a = 1.0f);
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
		inline float r() const { return rgba[0]; };
		inline float g() const { return rgba[1]; };
        inline float b() const { return rgba[2]; };
        inline float a() const { return rgba[3]; };
		int rInt() const;
		int gInt() const;
        int bInt() const;
        int aInt() const;
	private:
		int toInt(float v) const;
	public:
		static const Color Black;
		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
	};
}

#endif // RENDERER_COLOR_HPP
