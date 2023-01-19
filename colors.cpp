#include <iostream>
#include <windows.h>
#include <cstdint>

HANDLE H{GetStdHandle(STD_OUTPUT_HANDLE)};

void setColor(std::int16_t color)
{
    SetConsoleTextAttribute(H, color);
}

void resetColor()
{
    SetConsoleTextAttribute(H, 7); // default white
}
