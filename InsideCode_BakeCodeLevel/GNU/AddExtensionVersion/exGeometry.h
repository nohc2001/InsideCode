#ifndef EXGEOMETRY_H
#define EXGEOMETRY_H

#include "ICB_Extension.h"
#include <math.h>

constexpr float PI = 3.141592f;

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

//vec2f _vec2f(float x, float y);
void exGeometry__vec2f(int* pcontext){
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
void exGeometry__circle(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    circle c = *reinterpret_cast<circle*>(icc->rfsp - 12);
    
    icc->sp -= sizeof(circle);
    *reinterpret_cast<circle*>(icc->sp) = c;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//line2d _line2d(vec2f FP, vec2f LP);
void exGeometry__line2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d l;
    l = *reinterpret_cast<line2d*>(icc->rfsp - 16);

    icc->sp -= sizeof(line2d);
    *reinterpret_cast<line2d*>(icc->sp) = l;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//angle2d _angle2d_delta(float deltaX, float deltaY);
void exGeometry__angle2d_delta(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    float dx = *reinterpret_cast<float*>(icc->rfsp - 8);
    float dy = *reinterpret_cast<float*>(icc->rfsp - 4);

    angle2d a;
	float l = sqrtf(dx*dx + dy*dy);
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
void exGeometry__angle2d(int* pcontext){
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
void exGeometry_get_cross_CircleAndLine(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    circle cir = *reinterpret_cast<circle*>(icc->rfsp - (sizeof(circle) + sizeof(line2d)));
    line2d l = *reinterpret_cast<line2d*>(icc->rfsp - sizeof(line2d));

    vec2f p1;
    vec2f p2;

    float xr = l.lp.x - l.fp.x;
    float yr = l.lp.y - l.fp.y;
    if(fabsf(xr) > fabsf(yr)){
        float yrxr = yr / xr;
        float a = yrxr*yrxr+1;
        float temp = yrxr * l.fp.x - l.fp.y + cir.radius;
        float b = -2.0f*(temp - cir.center.x);
        float c = cir.center.x*cir.center.x + temp*temp - cir.radius*cir.radius;
        float isnanf = b*b - 4*a*c;
        if(isnanf > 0){
            float sn = sqrtf(isnanf);
            p1.x = -0.5f * (b - sn) / a;
            p1.y = yrxr*(p1.x-l.fp.x)+l.fp.y;
            p2.x = -0.5f * (b + sn) / a;
            p2.y = yrxr*(p2.x-l.fp.x)+l.fp.y;

            float min = (p1.x > p2.x) ? p2.x : p1.x;
            float max = (p1.x > p2.x) ? p1.x : p2.x;
            bool b1 = min < p1.x && p1.x < max;
            bool b2 = min < p2.x && p2.x < max;
            if(!b1){
                p1.x = NAN;
                p1.y = NAN;
            }
            if(!b2){
                p2.x = NAN;
                p2.y = NAN;
            }
        }
        else if(isnanf == 0){
            p1.x = -0.5f * b / a;
            p1.y = yrxr*(p1.x-l.fp.x)+l.fp.y;
            p2.x = NAN;
            p2.y = NAN;

            float min = (p1.x > p2.x) ? p2.x : p1.x;
            float max = (p1.x > p2.x) ? p1.x : p2.x;
            bool b1 = min < p1.x && p1.x < max;
            if(!b1){
                p1.x = NAN;
                p1.y = NAN;
            }
        }
        else{
            p1.x = NAN;
            p1.y = NAN;
            p2.x = NAN;
            p2.y = NAN;
        }
    }
    else{
        float xryr = xr / yr;
        float a = xryr*xryr+1;
        float temp = xryr * l.fp.y - l.fp.x + cir.radius;
        float b = -2.0f*(temp - cir.center.y);
        float c = cir.center.y*cir.center.y + temp*temp - cir.radius*cir.radius;
        float isnanf = b*b - 4*a*c;
        if(isnanf > 0){
            float sn = sqrtf(isnanf);
            p1.y = -0.5f * (b - sn) / a;
            p1.x = xryr*(p1.x-l.fp.x)+l.fp.y;
            p2.y = -0.5f * (b + sn) / a;
            p2.x = xryr*(p2.x-l.fp.x)+l.fp.y;
        }
        else if(isnanf == 0){
            p1.y = -0.5f * b / a;
            p1.x = xryr*(p1.x-l.fp.x)+l.fp.y;
            p2.y = NAN;
            p2.x = NAN;
        }
        else{
            p1.x = NAN;
            p1.y = NAN;
            p2.x = NAN;
            p2.y = NAN;
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
void exGeometry_get_pos_in_LineAndRatioAB(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d L = *reinterpret_cast<line2d*>(icc->rfsp - 24);
    float A = *reinterpret_cast<float*>(icc->rfsp - 8);
    float B = *reinterpret_cast<float*>(icc->rfsp - 4);
    float invtotal = 1/(A+B);
    A = A * invtotal;
    B = B * invtotal;
    vec2f rp;
    rp.x = B*L.fp.x + A*L.lp.x;
    rp.y = B*L.fp.y + A*L.lp.y;

    icc->sp -= sizeof(vec2f);
    *reinterpret_cast<vec2f*>(icc->sp) = rp;
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
}

//vec2f* get_poses_in_Bezier1F(vec2f p0, vec2f p1, vec2f factor, uint sizeOfVertex);
void exGeometry_get_poses_in_Bezier1F(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    vec2f s = *reinterpret_cast<vec2f*>(icc->rfsp - 28);
    vec2f e = *reinterpret_cast<vec2f*>(icc->rfsp - 20);
    vec2f factor = *reinterpret_cast<vec2f*>(icc->rfsp - 12);
    uint Siz = *reinterpret_cast<uint*>(icc->rfsp - 4);

    float t = 0;
    float delta = 1 / (float)Siz;
    fm->_tempPushLayer();
    icc->sp -= sizeof(vec2f)*Siz;
    vec2f* output = (vec2f*)icc->sp;
    float it = (1.0f);
    vec2f sf, ef;
    sf = s;
    ef = e;
    const float fdx = factor.x * delta;
    const float fdy = factor.y * delta;
    const float sfdx = fdx - s.x * delta;
    const float sfdy = fdy - s.y * delta;
    const float efdx = e.x * delta - fdx;
    const float efdy = e.y * delta - fdy;
    const float outaddx = efdx - sfdx;
    const float outaddy = efdy - sfdy;
    const float SdeltaOx = Siz * sfdx;
    const float SdeltaOy = Siz * sfdy;

    float save_ox = 0;
    float save_oy = 0;
    float prev_sfx=0, prev_sfy=0;
    float prev_efx=0, prev_efy=0;
    for(int i=0;i<Siz;++i){
        //sf.x += sfdx;
        //sf.y += sfdy;
        //ef.x += efdx;
        //ef.y += efdy;
        //output[i].x = it*sf.x + t*ef.x;
        //output[i].y = it*sf.y + t*ef.y;
        output[i].x = output[i-1].x - save_ox + SdeltaOx - prev_sfx + prev_efx;
        output[i].y = output[i-1].y - save_oy + SdeltaOy - prev_sfy + prev_efy;

        save_ox += outaddx;
        save_oy += outaddy;
        prev_sfx += sfdx;
        prev_sfy += sfdy;
        prev_efx += efdx;
        prev_efy += efdy;
    }
    icc->_as[0] = icc->sp - icc->mem;
    icc->_as.move_pivot(-1);
    fm->_tempPopLayer();
}

//float get_distance2d(line2d l);
void exGeometry_get_distance2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    line2d L = *reinterpret_cast<line2d*>(icc->rfsp - 16);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//bool isPosInRect2d(vec2f pos, rect4f rt);
void exGeometry_isPosInRect2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//bool isPosInPolygonRange(vec2f pos, vec2f* polygon, uint polygon_vertex_siz);
void exGeometry_isPosInPolygonRange(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//angle2d addAngle(angle2d A, angle2d B);
void exGeometry_addAngle2d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//vec2f get_cross_line(line2d A, line2d B);
void exGeometry_get_cross_line(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//vec3f[2] get_cross_SphereAndLine(sphere c, line3d l);
void exGeometry_get_cross_SphereAndLine(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//vec3f get_pos_in_LineAndRatioAB(line3d l, float A, float B);
void exGeometry_get_pos_in_LineAndRatioAB(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//vec3f* get_poses_in_Bezier1F(vec3f p0, vec3f p1, vec3f factor, uint sizeOfVertex);
void exGeometry_get_poses_in_Bezier1F(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//float get_distance(line3d l);
void exGeometry_get_distance(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//bool isPosInCube3d(vec3f pos, cubef6 cb);
void exGeometry_isPosInCube3d(int* pcontext){
    ICB_Context* icc = reinterpret_cast<ICB_Context*>(pcontext);
    int a = *reinterpret_cast<int*>(icc->rfsp - 8);
    int b = *reinterpret_cast<int*>(icc->rfsp - 4);
    icc->_as[0] = a+b;
    icc->_as.move_pivot(-1);
}

//angle3d addAngle(angle3d A, angle3d B);
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