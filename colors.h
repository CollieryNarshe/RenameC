#ifndef COLORS
#define COLORS
#include <cstdint>

namespace Color
{
    inline constexpr int16_t purple{5};
    inline constexpr int16_t yellow{6};
    inline constexpr int16_t default_white{7};
    inline constexpr int16_t blue{9};
    inline constexpr int16_t green{10};
    inline constexpr int16_t cyan{11};
    inline constexpr int16_t red{12};
    inline constexpr int16_t pink{13};
    inline constexpr int16_t white{15};
}


void setColor(int color);
void resetColor();

#endif