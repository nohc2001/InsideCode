class ICB_Context;
typedef void (*exfunction)(ICB_Context*);
extern exfunction *extend_inst;

void exfunc(ICB_Context* ic){
    
}

void Init_Extend(){
    //확장을 입력.
    int i=0;
    extend_inst[i++] = exfunc;
}