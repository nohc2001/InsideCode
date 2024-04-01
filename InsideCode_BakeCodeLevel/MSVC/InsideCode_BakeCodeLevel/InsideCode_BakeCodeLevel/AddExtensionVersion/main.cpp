#include "InsideCodeBake.h"
#include "ICB_Extension.h"
#include "exAdd.h"

extern FM_System0 *fm;

word_base_sen_sys InsideCode_Bake::wbss;
type_data *InsideCode_Bake::basictype[basictype_max];
operator_data InsideCode_Bake::basicoper[basicoper_max];

int main(){
    fm = new FM_System0();
	fm->SetHeapData(4096, 4096, 65536, 1048576);
    InsideCode_Bake::StaticInit();
    InsideCode_Bake icb;
	icb.init();
    ICB_Extension* ext = Init_exAdd();
    icb.extension.push_back(ext);
	icb.bake_code("code.txt");

	vecarr<ICB_Context *> exectxs;
    ICB_Context* ctx = (ICB_Context*)fm->_New(sizeof(ICB_Context), true);
    ctx->SetICB(&icb, 40960); // 40KB
    
	exectxs.NULLState();
	exectxs.Init(2, false);
    exectxs.push_back(ctx);
	execute(exectxs, 1, code_control, true);
}