//version 0.1
#ifndef EXGEOMETRY_H
#define EXGEOMETRY_H

#include "ICB_Extension.h"
#include <math.h>

#define __isin(A, B, C) ((A < B && B < C) || (C < B && B < A))

constexpr float PI = 3.141592f;

struct vec2f {
    float x;
    float y;

    vec2f() {
        x = 0;
        y = 0;
    }

    vec2f(float X, float Y) {
        x = X;
        y = Y;
    }
};

struct circle {
    vec2f center;
    float radius;
};

struct line2d {
    vec2f fp;
    vec2f lp;
};

struct angle2d {
    vec2f delta;
    float radian;
};

struct rect4f {
    vec2f fp;
    vec2f lp;
};

struct vec3f {
    float x;
    float y;
    float z;
};

struct sphere {
    vec3f center;
    float radius;
};

struct line3d {
    vec3f fp;
    vec3f lp;
};

struct cubef6 {
    vec3f fp;
    vec3f lp;
};

struct angle3d {
    vec3f delta;
    vec2f radian;
};

//vec2f _vec2f(float x, float y);
void exGeometry__vec2f(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    vec2f v;
    v.x = *reinterpret_cast<float*>(icc->rfsp - 8);
    v.y = *reinterpret_cast<float*>(icc->rfsp - 4);

    icc->sp -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = v;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//circle _circle(vec2f cen, float rad);
void exGeometry__circle(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    circle c = *reinterpret_cast<circle*>(icc->rfsp - 12);

    icc->sp -= sizeof(circle);
    *reinterpret_cast<circle*>(icc->sp) = c;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//line2d _line2d(vec2f FP, vec2f LP);
void exGeometry__line2d(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d l;
    l = *reinterpret_cast<line2d*>(icc->rfsp - 16);

    icc->sp -= sizeof(line2d);
    *reinterpret_cast<line2d*>(icc->sp) = l;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//angle2d _angle2d_delta(float deltaX, float deltaY);
void exGeometry__angle2d_delta(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    float dx = *reinterpret_cast<float*>(icc->rfsp - 8);
    float dy = *reinterpret_cast<float*>(icc->rfsp - 4);

    angle2d a;
    float l = sqrtf(dx * dx + dy * dy);
    dx = dx / l;
    dy = dy / l;
    float angle = (asinf(fabsf(dy)) + acosf(fabsf(dx))) / 2.0f;

    if (dy > 0) {
        if (dx > 0) {
            a.radian = angle;
        }
        else {
            a.radian = PI - angle;
        }
    }
    else {
        if (dx > 0) {
            a.radian = 2 * PI - angle;
        }
        else {
            a.radian = PI + angle;
        }
    }
    a.delta.x = dx;
    a.delta.y = dy;

    icc->sp -= sizeof(angle2d);
    *reinterpret_cast<angle2d*>(icc->sp) = a;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//angle2d _angle2d(float radian);
void exGeometry__angle2d(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    float radian = *reinterpret_cast<float*>(icc->rfsp - 4);

    angle2d a;
    if (radian > 2.0f * PI) {
        int a = radian / (2.0f * PI);
        radian = radian - 2.0f * (float)a * PI;
    }
    if (radian < 0) {
        int a = -radian / (2.0f * PI);
        radian = ((float)(a + 1) * 2.0f * PI) - radian;
    }
    a.radian = radian;
    a.delta.x = cosf(radian);
    a.delta.y = sinf(radian);

    icc->sp -= sizeof(angle2d);
    *reinterpret_cast<angle2d*>(icc->sp) = a;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//vec2f[2] get_cross_CircleAndLine(circle c, line2d l);
void exGeometry_get_cross_CircleAndLine(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    circle cir = *reinterpret_cast<circle*>(icc->rfsp - (sizeof(circle) + sizeof(line2d)));
    line2d l = *reinterpret_cast<line2d*>(icc->rfsp - sizeof(line2d));

    vec2f p1, p2;

    float xr = l.lp.x - l.fp.x;
    float yr = l.lp.y - l.fp.y;
    if (fabsf(xr) > fabsf(yr)) {
        float yrDxr = yr / xr;
        float ycy = yrDxr * l.fp.x - l.fp.y + cir.center.y;
        float A = yrDxr * yrDxr + 1;
        float B = -2.0f * (cir.center.x + ycy * yrDxr);
        float C = cir.center.x * cir.center.x + ycy * ycy - cir.radius * cir.radius;
        float inSqrt = B * B - 4.0f * A * C;
        if (inSqrt < 0) {
            p1 = vec2f(NAN, NAN);
            p2 = vec2f(NAN, NAN);
        }
        else if (inSqrt == 0) {
            float rx1 = -B * (2.0f * A);
            float ry1 = yrDxr * (rx1 - l.fp.x) + l.fp.y;
            if (__isin(l.fp.x, rx1, l.lp.x) && __isin(l.fp.y, ry1, l.lp.y)) {
                p1 = vec2f(rx1, ry1);
                p2 = vec2f(NAN, NAN);
            }
            else {
                p1 = vec2f(NAN, NAN);
                p2 = vec2f(NAN, NAN);
            }
        }
        else {
            float sqrtv = sqrtf(inSqrt);
            vec2f rp1 = vec2f((-B + sqrtv) / (2 * A), 0);
            rp1.y = yrDxr * (rp1.x - l.fp.x) + l.fp.y;
            vec2f rp2 = vec2f((-B - sqrtv) / (2 * A), 0);
            rp2.y = yrDxr * (rp2.x - l.fp.x) + l.fp.y;

            if (__isin(l.fp.x, rp1.x, l.lp.x) && __isin(l.fp.y, rp1.y, l.lp.y)) {
                p1 = rp1;
            }
            else {
                p1 = vec2f(NAN, NAN);
            }

            if (__isin(l.fp.x, rp2.x, l.lp.x) && __isin(l.fp.y, rp2.y, l.lp.y)) {
                p2 = rp1;
            }
            else {
                p2 = vec2f(NAN, NAN);
            }
        }
    }
    else {
        float xrDyr = xr / yr;
        float xcx = xrDyr * l.fp.y - l.fp.x + cir.center.x;
        float A = xrDyr * xrDyr + 1;
        float B = -2.0f * (cir.center.y + xcx * xrDyr);
        float C = cir.center.y * cir.center.y + xcx * xcx - cir.radius * cir.radius;
        float inSqrt = B * B - 4.0f * A * C;
        if (inSqrt < 0) {
            p1 = vec2f(NAN, NAN);
            p2 = vec2f(NAN, NAN);
        }
        else if (inSqrt == 0) {
            float ry1 = -B * (2.0f * A);
            float rx1 = xrDyr * (ry1 - l.fp.y) + l.fp.x;
            if (__isin(l.fp.x, rx1, l.lp.x) && __isin(l.fp.y, ry1, l.lp.y)) {
                p1 = vec2f(rx1, ry1);
                p2 = vec2f(NAN, NAN);
            }
            else {
                p1 = vec2f(NAN, NAN);
                p2 = vec2f(NAN, NAN);
            }
        }
        else {
            float sqrtv = sqrtf(inSqrt);
            vec2f rp1 = vec2f(0, (-B + sqrtv) / (2 * A));
            rp1.x = xrDyr * (rp1.y - l.fp.y) + l.fp.x;
            vec2f rp2 = vec2f(0, (-B - sqrtv) / (2 * A));
            rp2.x = xrDyr * (rp2.y - l.fp.y) + l.fp.x;

            if (__isin(l.fp.x, rp1.x, l.lp.x) && __isin(l.fp.y, rp1.y, l.lp.y)) {
                p1 = rp1;
            }
            else {
                p1 = vec2f(NAN, NAN);
            }

            if (__isin(l.fp.x, rp2.x, l.lp.x) && __isin(l.fp.y, rp2.y, l.lp.y)) {
                p2 = rp1;
            }
            else {
                p2 = vec2f(NAN, NAN);
            }
        }
    }

    icc->sp -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = p2;
    icc->sp -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = p1;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//vec2f get_pos_in_LineAndRatioAB(line2d l, float A, float B);
void exGeometry_get_pos_in_LineAndRatioAB(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d L = *reinterpret_cast<line2d*>(icc->rfsp - 24);
    float A = *reinterpret_cast<float*>(icc->rfsp - 8);
    float B = *reinterpret_cast<float*>(icc->rfsp - 4);
    float invtotal = 1 / (A + B);
    A = A * invtotal;
    B = B * invtotal;
    vec2f rp;
    rp.x = B * L.fp.x + A * L.lp.x;
    rp.y = B * L.fp.y + A * L.lp.y;

    icc->sp -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = rp;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//vec2f* get_poses_in_Bezier1F(vec2f p0, vec2f p1, vec2f factor, uint sizeOfVertex);
void exGeometry_get_poses_in_Bezier1F(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    vec2f s = *reinterpret_cast<vec2f*>(icc->rfsp - 28);
    vec2f e = *reinterpret_cast<vec2f*>(icc->rfsp - 20);
    vec2f factor = *reinterpret_cast<vec2f*>(icc->rfsp - 12);
    uint Siz = *reinterpret_cast<uint*>(icc->rfsp - 4);
    vec2f sf, ef;
    sf = s;
    ef = factor;
    float t = 0;
    float delta = 1.0f / (float)Siz;
    icc->sp -= sizeof(vec2f) * Siz;
    vec2f* out = (vec2f*)icc->sp;
    for (int i = 0; i < Siz; ++i) {
        sf.x = (1.0f - t) * s.x + t * factor.x;
        sf.y = (1.0f - t) * s.y + t * factor.y;
        ef.x = (1.0f - t) * factor.x + t * e.x;
        ef.y = (1.0f - t) * factor.y + t * e.y;
        out[i].x = (1.0f - t) * sf.x + t * ef.x;
        out[i].y = (1.0f - t) * sf.y + t * ef.y;
        t += delta;
    }
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//float get_distance2d(line2d l);
void exGeometry_get_distance2d(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d L = *reinterpret_cast<line2d*>(icc->rfsp - 16);
    float distx = (L.lp.x - L.fp.x);
    float disty = (L.lp.y - L.fp.y);
    float dist = sqrtf(distx * distx + disty * disty);

    *reinterpret_cast<float*>(&icc->_as[0]) = dist;
    icc->_as.move_pivot(-1);
}

//bool isPosInRect2d(vec2f pos, rect4f rt);
void exGeometry_isPosInRect2d(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    vec2f pos = *reinterpret_cast<vec2f*>(icc->rfsp - 24);
    rect4f rt = *reinterpret_cast<rect4f*>(icc->rfsp - 16);
    bool b = rt.fp.x < pos.x && pos.x < rt.lp.x;
    b = b || (rt.fp.y < pos.y && pos.y < rt.lp.y);
    *reinterpret_cast<bool*>(&icc->_as[0]) = b;
    icc->_as.move_pivot(-1);
}

//angle2d addAngle(angle2d A, angle2d B);
void exGeometry_addAngle2d(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    angle2d A = *reinterpret_cast<angle2d*>(icc->rfsp - 24);
    angle2d B = *reinterpret_cast<angle2d*>(icc->rfsp - 12);
    angle2d R;
    R.radian = A.radian + B.radian;
    R.radian -= 2 * PI * floorf((R.radian) / (2 * PI));
    R.delta.x = cosf(R.radian);
    R.delta.y = sinf(R.radian);

    icc->sp -= sizeof(angle2d);
    *reinterpret_cast<angle2d*>(icc->sp) = R;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//vec2f get_cross_line(line2d A, line2d B);
void exGeometry_get_cross_line(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d sl1 = *reinterpret_cast<line2d*>(icc->rfsp - 32);
    line2d sl2 = *reinterpret_cast<line2d*>(icc->rfsp - 16);

    vec2f sl1Rate, sl2Rate;
    sl1Rate.x = sl1.lp.x - sl1.fp.x;
    sl1Rate.y = sl1.lp.y - sl1.fp.y;
    sl2Rate.x = sl2.lp.x - sl2.fp.x;
    sl2Rate.y = sl2.lp.y - sl2.fp.y;

    vec2f cross;
    if (sl1Rate.x * sl2Rate.x != 0 || sl1Rate.y * sl2Rate.y != 0) {
        cross.x = (sl1.fp.x * sl1Rate.y / sl1Rate.x - sl1.fp.y - sl2.fp.x * sl2Rate.y / sl2Rate.x + sl2.fp.y) / (sl1Rate.y / sl1Rate.x - sl2Rate.y / sl2Rate.x);
        cross.y = (sl1Rate.y / sl1Rate.x) * (cross.x - sl1.fp.x) + sl1.fp.y;
    }
    else {
        if (sl1Rate.x == 0 || sl2Rate.x == 0) {
            cross.x = (sl1Rate.x == 0) ? sl1.fp.x : sl2.fp.x;
            if (sl1Rate.y == 0 || sl2Rate.y == 0) {
                cross.y = (sl1Rate.y == 0) ? sl1.fp.y : sl2.fp.y;
            }
            else {
                cross.y = (sl1Rate.x == 0) ? ((sl2Rate.y / sl2Rate.x) * (cross.x - sl2.fp.x) + sl2.fp.y) : ((sl1Rate.y / sl1Rate.x) * (cross.x - sl1.fp.x) + sl1.fp.y);
            }
        }
        else {
            cross.y = (sl1Rate.y == 0) ? sl1.fp.y : sl2.fp.y;
            cross.x = (sl1Rate.y == 0) ? ((sl2Rate.x / sl2Rate.y) * (cross.y - sl2.fp.y) + sl2.fp.x) : ((sl1Rate.x / sl1Rate.y) * (cross.y - sl1.fp.y) + sl1.fp.x);
        }
    }
    //return cross;

    bool b = true;
    vec2f temp_mm;
    temp_mm.x = (sl1.fp.x < sl1.lp.x) ? sl1.fp.x : sl1.lp.x;
    temp_mm.y = (sl1.fp.x < sl1.lp.x) ? sl1.lp.x : sl1.fp.x;
    b = b && (temp_mm.x <= cross.x && cross.x <= temp_mm.y);

    temp_mm.x = (sl1.fp.y < sl1.lp.y) ? sl1.fp.y : sl1.lp.y;
    temp_mm.y = (sl1.fp.y < sl1.lp.y) ? sl1.lp.y : sl1.fp.y;
    b = b && (temp_mm.x <= cross.y && cross.y <= temp_mm.y);

    temp_mm.x = (sl2.fp.x < sl2.lp.x) ? sl2.fp.x : sl2.lp.x;
    temp_mm.y = (sl2.fp.x < sl2.lp.x) ? sl2.lp.x : sl2.fp.x;
    b = b && (temp_mm.x <= cross.x && cross.x <= temp_mm.y);

    temp_mm.x = (sl2.fp.y < sl2.lp.y) ? sl2.fp.y : sl2.lp.y;
    temp_mm.y = (sl2.fp.y < sl2.lp.y) ? sl2.lp.y : sl2.fp.y;
    b = b && (temp_mm.x <= cross.y && cross.y <= temp_mm.y);

    if (b == false) {
        cross.x = NAN;
        cross.y = NAN;
    }

    icc->sp -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = cross;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//bool isNAN(float f);
void exGeometry_isNAN(int* pcontext) {
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    float f = *reinterpret_cast<float*>(icc->rfsp - 4);
    bool r;
    if (_isnanf(f)) {
        r = true;
    }
    else {
        r = false;
    }
    icc->_as[0] = (uint64_t)(r);
    icc->_as.move_pivot(-1);
}

ICB_Extension* Init_exGeometry() {
    //확장을 입력.
    ofstream& icl = InsideCode_Bake::icl;
    icl << "Create_New_ICB_Extension_Init : exGeometry Initialization...";

    bool icldetail = InsideCode_Bake::GetICLFlag(ICL_FLAG::Create_New_ICB_Extension_Init);
    if (icldetail) icl << "start" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Allocate Extension Data Memory...";
    ICB_Extension* ext = (ICB_Extension*)fm->_New(sizeof(ICB_Extension), true);
    ext->exfuncArr.NULLState();
    ext->exfuncArr.Init(32, false);
    ext->exstructArr.NULLState();
    ext->exstructArr.Init(32, false);
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension...";
    bake_Extension("exGeometry.txt", ext);
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers start" << endl;
    int i = -1;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry__vec2f);//0
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry__circle);//1
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry__line2d);//2
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry__angle2d_delta);//3
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry__angle2d);//4
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_get_cross_CircleAndLine);//5
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_get_pos_in_LineAndRatioAB);//6
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_get_poses_in_Bezier1F);//7
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_get_distance2d);//8
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_isPosInRect2d);//9
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_addAngle2d);//10
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_get_cross_line);//11
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Set_Function_Pointers function name : " << ext->exfuncArr[i + 1]->name.c_str() << "...";
    ext->exfuncArr[++i]->start_pc = reinterpret_cast<byte8*>(exGeometry_isNAN);//12
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init Allocate Extension Data Memory ";
    icl << "finish" << endl;
    return ext;
}

#endif