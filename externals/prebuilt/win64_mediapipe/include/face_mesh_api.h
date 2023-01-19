#ifndef FACE_MESH_API_H
#define FACE_MESH_API_H

#ifdef FACE_MESH_EXPORTS
#define FACE_MESH_API __declspec(dllexport)
#else
#define FACE_MESH_API __declspec(dllimport)
#endif

extern "C" FACE_MESH_API int FACE_MESH_INIT();
extern "C" FACE_MESH_API int FACE_MESH_UPDATE();
extern "C" FACE_MESH_API int FACE_MESH_END();
extern "C" FACE_MESH_API float* FACE_MESH_GET_FACE_LANDMARKS();
extern "C" FACE_MESH_API int FACE_MESH_GET_FACE_LANDMARKS_SIZE();

#endif 
