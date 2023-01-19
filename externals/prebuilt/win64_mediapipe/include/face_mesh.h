// #ifndef FACE_MESH_H
// #define FACE_MESH_H
// 
// #include <iostream>
// #include <windows.h>
// 
// #ifdef FACE_MESH_EXPORTS
// #define FACE_MESH_API __declspec(dllexport)
// #else
// #define FACE_MESH_API __declspec(dllimport)
// #endif
// 
// BOOL APIENTRY DllMain(HMODULE hModule, DWORD Reason_For_Call, LPVOID lpReserved);
// 
// void FACE_MESH_API showSomething();
// 
// FACE_MESH_API float* startGraph();
// void FACE_MESH_API updateGraph();
// void FACE_MESH_API stopGraph();
// 
// #endif