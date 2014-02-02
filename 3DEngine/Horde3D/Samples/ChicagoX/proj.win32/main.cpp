#include "main.h"
#include "../AppDelegate.h"
#include "CCEGLView.h"
#include <stdio.h>

USING_NS_CC;

//int APIENTRY _tWinMain(HINSTANCE hInstance,
//                       HINSTANCE hPrevInstance,
//                       LPTSTR    lpCmdLine,
//                       int       nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    printf("HERE!!!!");
//    // create the application instance
//    AppDelegate app;
//    CCEGLView* eglView = CCEGLView::sharedOpenGLView();
//    eglView->setViewName("Sample Knight");
//    eglView->setFrameSize(1024, 768);
//    return CCApplication::sharedApplication()->run();
//}

int main()
{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);

    printf("HERE!!!!");
    // create the application instance
    AppDelegate app;
    CCEGLView* eglView = CCEGLView::sharedOpenGLView();
    printf("SETTING VIEW NAME !!\n");
    eglView->setViewName("Sample Knight");
    printf("SETTING FRAME SIZE !!\n");
    eglView->setFrameSize(1024, 768);
    return CCApplication::sharedApplication()->run();
}