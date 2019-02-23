#pragma once

#include <memory>

struct UIOverlayImpl;

class UIOverlay
{
public:
    void Init();
    void Draw();
    void OnWindowSize(int width, int height);
    int OnKeyboard(unsigned char key, int x, int y);
    int OnMousePressed(int button, int state, int x, int y);
    int OnMousePassiveMove(int x, int y);
    int OnMouseWheel(int button, int dir, int x, int y);
    int OnSpecialFunc(int key, int x, int y);

    void Text(const char* name, const char* text);
    void ReadonlyInt(const char* name, const int32_t* value);
    void ReadonlyFloat(const char* name, const float* value, uint8_t precision = 2);

private:
    UIOverlayImpl* impl;
}; 
