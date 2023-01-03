#include <iostream>
#include <windows.h>

HANDLE H{GetStdHandle(STD_OUTPUT_HANDLE)};

void setColor(int color)
{
    SetConsoleTextAttribute(H, color);
}

void resetColor()
{
    SetConsoleTextAttribute(H, 7); // default white
}
