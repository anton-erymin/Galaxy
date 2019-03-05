#include "UIOverlay.h"

#include "AntTweakBar.h"
#include "GL\freeglut.h"


struct UIOverlayImpl
{
    TwBar* bar;
};

void UIOverlay::Init()
{
    impl = new UIOverlayImpl();

    TwInit(TwGraphAPI::TW_OPENGL_CORE, nullptr);

    TwGLUTModifiersFunc(glutGetModifiers);

    impl->bar = TwNewBar("Galaxy");
    TwDefine(" Galaxy size='400 300' valueswidth=fit color='0 0 100' alpha=50"); // change default tweak bar size and color
}

void UIOverlay::Draw()
{
    TwDraw();
}

void UIOverlay::OnWindowSize(int width, int height)
{
    TwWindowSize(width, height);
}

int UIOverlay::OnKeyboard(unsigned char key, int x, int y)
{
    return TwEventKeyboardGLUT(key, x, y);
}

int UIOverlay::OnMousePressed(int button, int state, int x, int y)
{
    return TwEventMouseButtonGLUT(button, state, x, y);
}

int UIOverlay::OnMousePassiveMove(int x, int y)
{
    return TwEventMouseMotionGLUT(x, y);
}

int UIOverlay::OnMouseWheel(int button, int dir, int x, int y)
{
    return 0;
}

int UIOverlay::OnSpecialFunc(int key, int x, int y)
{
    return TwEventSpecialGLUT(key, x, y);
}

void UIOverlay::Text(const char* name, const char* text)
{
    TwAddVarRO(impl->bar, name, TW_TYPE_CSSTRING(256), text, "");
}

void UIOverlay::ReadonlyInt(const char* name, const int32_t* value)
{
    TwAddVarRO(impl->bar, name, TW_TYPE_INT32, value, "");
}

void UIOverlay::ReadonlyFloat(const char* name, const float* value, uint8_t precision)
{
    std::string strPrecision = "precision=" + std::to_string(precision);
    TwAddVarRO(impl->bar, name, TW_TYPE_FLOAT, value, strPrecision.c_str());
}

void UIOverlay::Checkbox(const char* name, bool* value, const char* key)
{
    std::string def;
    if (key)
    {
        def = "key=" + std::string(key);
    }
    TwAddVarRW(impl->bar, name, TW_TYPE_BOOLCPP, value, def.c_str());
}

void UIOverlay::SliderFloat(const char* name, float* value, float min, float max, float step)
{
    std::string def = "min=" + std::to_string(min) + " max=" + std::to_string(max) + " step=" + std::to_string(step);
    TwAddVarRW(impl->bar, name, TW_TYPE_FLOAT, value, def.c_str());
}

void UIOverlay::Separator()
{
    TwAddSeparator(impl->bar, nullptr, "");
}