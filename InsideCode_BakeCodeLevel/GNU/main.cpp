#define FM_GET_NONRELEASE_HEAPPTR
#define FM_NONRELEASE_HEAPCHECK

#include "InsideCodeBake.h"
#include "ICB_Extension.h"
#include "exGeometry.h"

extern FM_System0 *fm;

word_base_sen_sys InsideCode_Bake::wbss;
type_data InsideCode_Bake::basictype[basictype_max];
operator_data InsideCode_Bake::basicoper[basicoper_max];
ofstream InsideCode_Bake::icl;
uint32_t InsideCode_Bake::icl_optionFlag;

int main(){
    fm = new FM_System0();
	fm->SetHeapData(4096, 4096, 65536, 1048576);
    fm->_tempPushLayer();

    InsideCode_Bake::SetICLFlag(ICL_FLAG::ICB_StaticInit, false);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::Create_New_ICB_Extension_Init, false);
    {
        InsideCode_Bake::SetICLFlag(ICL_FLAG::Create_New_ICB_Extension_Init__Bake_Extension, false);
    }
    
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_AddTextBlocks, false);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_ScanStructTypes, false);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_AddStructTypes, false);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_ScanCodes, false);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_GlobalMemoryInit, false);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes, false);
    {
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__add_var, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__set_var, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__if__sen, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__while__, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__block__, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__addfunc, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__usefunc, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__return_, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__struct__, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__break__, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__continue, false);
        InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes__adsetvar, false);
    }

    InsideCode_Bake::SetICLFlag(ICL_FLAG::Create_New_ICB_Context, false);

    InsideCode_Bake::StaticInit();

    ICB_Extension* ext = Init_exGeometry();
    fm->dbg_fm1_lifecheck_charprint();

    InsideCode_Bake icb;
	icb.init();
    icb.extension.push_back(ext);

	icb.bake_code("code.txt");

	fmvecarr<ICB_Context *> exectxs;
    ICB_Context* ctx = (ICB_Context*)fm->_New(sizeof(ICB_Context), true);
    ctx->SetICB(&icb, 40960); // 40KB
	
    exectxs.NULLState();
	exectxs.Init(2, false, true);
    exectxs.push_back(ctx);
	execute_switch(exectxs, 1, code_control, true);

    fm->_tempPopLayer();

    ctx->Release();
    fm->_Delete((byte8*)ctx, sizeof(ICB_Context));

    icb.Release();

    ext->Release();
    fm->_Delete((byte8*)ext, sizeof(ICB_Extension));

    exectxs.release();
    exectxs.NULLState();

    InsideCode_Bake::wbss.Release();

    fm->dbg_fm1_lifecheck_charprint();

    fm->RECORD_NonReleaseHeapData();
    return 0;
}