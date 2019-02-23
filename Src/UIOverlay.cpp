#include "UIOverlay.h"

#include "AntTweakBar.h"
#include "GL\freeglut.h"

float g_Zoom;

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
    //TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLUT and OpenGL.' "); // Message added to the help bar.
    //TwDefine(" Controls size='200 400' color='96 216 224' "); // change default tweak bar size and color
    TwDefine(" Galaxy size='400 300' valueswidth=fit color='0 0 100' alpha=0"); // change default tweak bar size and color
    

    /*TwAddVarRW(impl->bar, "Zoom", TW_TYPE_FLOAT, &g_Zoom, 
        " min=0.01 max=2.5 step=0.01 keyIncr=z keyDecr=Z help='Scale the object (1=original size).' ");

    TwAddVarRO(impl->bar, "", TW_TYPE_STDSTRING, &gpu, "");

    TwSetTopBar(impl->bar);*/
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
