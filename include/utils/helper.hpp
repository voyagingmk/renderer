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

}
#endif
