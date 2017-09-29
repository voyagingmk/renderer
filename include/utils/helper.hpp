#ifndef RENDERER_UTILS_HELPER_HPP
#define RENDERER_UTILS_HELPER_HPP

#include "base.hpp"
#include "ecs/ecs.hpp"
#include "com/color.hpp"

namespace renderer{

    static Color parseColor(nlohmann::json &c)
    {
        if (c.is_null())
        {
            return Color::White;
        }
        if (c.is_string())
        {
            if (c == "Red")
                return Color::Red;
            else if (c == "White")
                return Color::White;
            else if (c == "Black")
                return Color::Black;
            else if (c == "Green")
                return Color::Green;
            else if (c == "Blue")
                return Color::Blue;
            else
                return Color::White;
        }
        else
        {
            return Color((float)c[0] / 255.0f, (float)c[1] / 255.0f, (float)c[2] / 255.0f);
        }
    }

	static float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	template<typename T>
	static T random0_1() {
		static std::uniform_real_distribution<T> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		static std::default_random_engine generator;
		return randomFloats(generator);
	}

	static void generateSampleKernel(std::vector<Vector3dF>& ssaoKernel) {
		for (unsigned int i = 0; i < 64; ++i)
		{
			Vector3dF sample(
				random0_1<float>() * 2.0 - 1.0, 
				random0_1<float>() * 2.0 - 1.0, 
				random0_1<float>());
			sample = sample.Normalize();
			sample *= random0_1<float>();
			float scale = float(i) / 64.0;

			// scale samples s.t. they're more aligned to center of kernel
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
	}

}
#endif
