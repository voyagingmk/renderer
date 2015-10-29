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
		Color operator + (const Color&);
		Color operator - (const Color&);
		Color operator * (float f);
		Color operator / (float f);
		bool operator == (const Color&);
		Color modulate(const Color&);
		inline float r() const { return rgb[0]; };
		inline float g() const { return rgb[1]; };
		inline float b() const { return rgb[2]; };
	public:
		static const PtrColor Black;
		static const PtrColor White;
		static const PtrColor Red;
		static const PtrColor Green;
		static const PtrColor Blue;
	};
}

#endif // RENDERER_COLOR_HPP
