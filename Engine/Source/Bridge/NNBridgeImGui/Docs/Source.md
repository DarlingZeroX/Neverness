# CImGui
## 项目引用
https://github.com/cimgui/cimgui/tree/207fca2d361179c349f3c9d1893b8274f4bbfebf
## 源码导入方法
1. 将cimgui文件夹放入项目中
2. 修改imgui头文件引用和freetype头文件引用为cimgui的路径
3. 替换cimconfig.h
```cpp
#pragma once

#undef NDEBUG

#if defined(_WIN32) || defined(_WIN64)

#if defined(NN_BRIDGE_IMGUI_EXPORT)
#define NN_BRIDGE_IMGUI_API __declspec(dllexport)
#elif defined(NN_BRIDGE_IMGUI_STATIC)
#define NN_BRIDGE_IMGUI_API
#else
#define NN_BRIDGE_IMGUI_API __declspec(dllimport)
#endif

#else

#if defined(VG_PACKAGE_API_EXPORT)
#define NN_BRIDGE_IMGUI_API __attribute__((visibility("default")))
#else
#define NN_BRIDGE_IMGUI_API
#endif

#endif
```

3. cimgui.h包含cimconfig.h后，修改CIMGUI_API定义
```cpp
#define CIMGUI_API EXTERN API
```
 为
```cpp
#define CIMGUI_API EXTERN NN_BRIDGE_IMGUI_API
```
4. 有些imgui internal类和函数也需要加上API定义