#ifndef EXADD_H
#define EXADD_H

#include "ICB_Extension.h"

void exAdd_add(int* pcontext){
    ICB_Context* cxt = reinterpret_cast<ICB_Context*>(pcontext);
    float a = *reinterpret_cast<float*>(cxt->rfsp - 8);
    float b = *reinterpret_cast<float*>(cxt->rfsp - 4);
    cxt->_as[0] = a+b;
    cxt->_as.move_pivot(-1);
}

ICB_Extension* Init_exAdd(){
    //확장을 입력.
    ICB_Extension* ext = (ICB_Extension*)fm->_New(sizeof(ICB_Extension), true);
    ext->exfuncArr.NULLState();
    ext->exfuncArr.Init(8, false);
    ext->exstructArr.NULLState();
    ext->exstructArr.Init(8, false);

    bake_Extension("exAdd.txt", ext);
    
    ext->exfuncArr[0]->start_pc = reinterpret_cast<byte8*>(exAdd_add);

    return ext;
}

#endif