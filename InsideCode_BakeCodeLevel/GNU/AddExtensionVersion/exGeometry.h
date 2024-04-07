#ifndef EXGEOMETRY_H
#define EXGEOMETRY_H

#include "ICB_Extension.h"

struct vec2f{
    float x;
    float y;
};

struct circle{
    vec2f center;
    float radius;
};

struct line2d{
    vec2f fp;
    vec2f lp;
};

struct angle2d{
    vec2f delta;
	float radian;
};

struct rectf4{
    vec2f fp;
    vec2f lp;
};

struct vec3f{
    float x;
    float y;
    float z;
};

struct sphere{
    vec3f center;
    float radius;
};

struct line3d{
    vec3f fp;
    vec3f lp;
};

struct cubef6{
    vec3f fp;
    vec3f lp;
};

struct angle3d{
    vec3f delta;
	vec2f radian;
};

void exGeometry__vec2f(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    vec2f v;
    v.x = *reinterpret_cast<float*>(icc->rfsp - 8);
    v.y = *reinterpret_cast<float*>(icc->rfsp - 4);

    *icc->spi -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = v;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

void exGeometry__circle(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    circle c = *reinterpret_cast<circle*>(icc->rfsp - 12);
    
    *icc->spi -= sizeof(circle);
    *reinterpret_cast<circle*>(icc->sp) = c;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

void exGeometry__line2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d l;
    l = *reinterpret_cast<line2d*>(icc->rfsp - 16);

    *icc->spi -= sizeof(line2d);
    *reinterpret_cast<line2d*>(icc->sp) = l;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

void exGeometry__angle2d_delta(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry__angle2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_cross_CircleAndLine(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_pos_in_LineAndRatioAB(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_poses_in_Bezier1F(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_distance2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_isPosInRect2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_isPosInPolygonRange(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_addAngle2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_cross_SphereAndLine(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_pos_in_LineAndRatioAB(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_poses_in_Bezier1F(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_get_distance(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_isPosInCube3d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

void exGeometry_addAngle3d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

ICB_Extension* Init_exGeometry(){
    //확장을 입력.
    ICB_Extension* ext = (ICB_Extension*)fm->_New(sizeof(ICB_Extension), true);
    ext->exfuncArr.NULLState();
    ext->exfuncArr.Init(8, false);
    ext->exstructArr.NULLState();
    ext->exstructArr.Init(8, false);

    bake_Extension("exGeometry.txt", ext);
    
    ext->exfuncArr[0]->start_pc = reinterpret_cast<byte8*>(exAdd_add);

    return ext;
}

#endif