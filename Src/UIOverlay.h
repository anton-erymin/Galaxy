#pragma once

#include <memory>
#include <string>

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
    void Checkbox(const char* name, bool* value, const char* key = nullptr);
    void SliderUint(const char* name, uint32_t* value);
    void SliderFloat(const char* name, float* value);
    void SliderFloat(const char* name, float* value, float min, float max, float step = 0.1f);
    void Separator();
    void Group(const char* group);
    void Button(const char* name, void(*callback)(void*), const char* key = nullptr);

private:
    UIOverlayImpl* impl;

    std::string currentGroup;
}; 
