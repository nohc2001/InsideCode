#include "InsideCodeBake.h"
#include "ICB_Extension.h"
#include "exGeometry.h"

extern FM_System0* fm;

word_base_sen_sys InsideCode_Bake::wbss;
type_data* InsideCode_Bake::basictype[basictype_max];
operator_data InsideCode_Bake::basicoper[basicoper_max];
ofstream InsideCode_Bake::icl;
uint32_t InsideCode_Bake::icl_optionFlag;

int main() {
    fm = new FM_System0();
    fm->SetHeapData(4096, 4096, 65536, 1048576);

    InsideCode_Bake::SetICLFlag(ICL_FLAG::ICB_StaticInit, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::Create_New_ICB_Extension_Init, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::Create_New_ICB_Extension_Init__Bake_Extension, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_AddTextBlocks, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_ScanStructTypes, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_AddStructTypes, true);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_ScanCodes, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_GlobalMemoryInit, false);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::BakeCode_CompileCodes, true);
    InsideCode_Bake::SetICLFlag(ICL_FLAG::Create_New_ICB_Context, false);

    InsideCode_Bake::StaticInit();
    ICB_Extension* ext = Init_exGeometry();
    InsideCode_Bake icb;
    icb.init(40960);
    icb.extension.push_back(ext);
    icb.bake_code("code.txt");

    vecarr<ICB_Context*> exectxs;
    ICB_Context* ctx = (ICB_Context*)fm->_New(sizeof(ICB_Context), true);
    ctx->SetICB(&icb, 40960); // 40KB

    exectxs.NULLState();
    exectxs.Init(2, false);
    exectxs.push_back(ctx);
    execute(exectxs, 1, code_control, true);
}