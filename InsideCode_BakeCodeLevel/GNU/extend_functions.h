#ifndef EXTEND_FUNC_H
#define EXTEND_FUNC_H

#include "InsideCodeBake.h"

void exfunc_Compile(int* picb){
    InsideCode_Bake* icb = reinterpret_cast<InsideCode_Bake*>(icb);
}

void exfunc_Execute(int* pcontext){
    ICB_Context* cxt = reinterpret_cast<ICB_Context*>(pcontext);
}

void Init_Extend(){
    //확장을 입력.
    int i=0;
    extend_inst[i++] = exfunc_Execute;
}

#endif