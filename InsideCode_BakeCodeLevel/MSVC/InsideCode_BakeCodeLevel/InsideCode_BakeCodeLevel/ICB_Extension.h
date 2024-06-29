#ifndef ICB_EXTENSION_H
#define ICB_EXTENSION_H
#include "Utill_FreeMemory.h"
#include "InsideCodeBake.h"

using namespace freemem;

//basic functions for extension operation

fmlcstr* GetCodeTXT(const char* filename, FM_System0* fm)
{
    char comment_mod = '/';
    FILE* fp;
    fopen_s(&fp, filename, "rt");
    if (fp)
    {
        fmlcstr* codetxt = (fmlcstr*)fm->_New(sizeof(fmlcstr), true);
        codetxt->NULLState();
        codetxt->Init(10, false);
        int max = 0;
        fseek(fp, 0, SEEK_END);
        max = ftell(fp);
        fclose(fp);

        int stack = 0;
        fopen_s(&fp, filename, "rt");
        int k = 0;
        while (k < max)
        {
            char c;
            c = fgetc(fp);
            if (c == '/')
            {
                stack += 1;
            }
            else if (stack == 1 && c == '*') {
                stack += 1;
                comment_mod = '*';
            }
            else
            {
                stack = 0;
            }

            if (stack == 2) // 이거 주석 처리 코드?
            {
                if (comment_mod == '/') {
                    codetxt->pop_back();
                    int mm = 0;
                    while (c != '\n' && k + mm < max)
                    {
                        mm += 1;
                        c = fgetc(fp);
                    }
                    max -= mm + 1;
                    continue;
                }
                else if (comment_mod == '*') {
                    codetxt->pop_back();
                    int mm = 0;
                    char saveC = 0;
                    while ((saveC != '*' || c != '/') && k + mm < max)
                    {
                        mm += 1;
                        saveC = c;
                        c = fgetc(fp);
                    }
                    max -= mm + 1;
                    comment_mod = '/';
                    continue;
                }
            }
            codetxt->push_back(c);
            k++;
        }
        return codetxt;
    }
    else
    {
        //printf("[ERROR] : %s file is not exist.", filename);
        return nullptr;
    }
}

void push_word(char* sptr, fmvecarr<char*>* codesen)
{
    for (int i = 0; i < InsideCode_Bake::wbss.wordlist.size(); ++i)
    {
        char* temp = InsideCode_Bake::wbss.wordlist.at(i);
        if (strcmp(sptr, temp) == 0)
        {
            codesen->push_back(temp);
            return;
        }
    }

    char* cstr = InsideCode_Bake::wbss.addword(sptr);
    codesen->push_back(cstr);
}

void set_word(int index, char* sptr, fmvecarr<char*>* codesen)
{
    for (int i = 0; i < InsideCode_Bake::wbss.wordlist.size(); ++i)
    {
        char* temp = InsideCode_Bake::wbss.wordlist.at(i);
        if (strcmp(sptr, temp) == 0)
        {
            (*codesen)[index] = temp;
            return;
        }
    }

    char* cstr = InsideCode_Bake::wbss.addword(sptr);
    (*codesen)[index] = cstr;

    /*

    const char *sptr = str.c_str();
    int len = strlen(sptr);
    char *cstr = (char *)fm->_New(len + 1, true);
    strcpy(cstr, sptr);
    cstr[len] = 0;
    fm->_Delete((byte8*)(*codesen)[index], strlen((*codesen)[index])+1);
    (*codesen)[index] = cstr;
    */
}

void AddTextBlocks(fmlcstr &codetxt, fmvecarr<char*>* codesen)
{
    fmlcstr insstr;
    insstr.NULLState();
    insstr.Init(2, false);

    cout << "\n\ntext : \n" << codetxt.c_str() << endl;
    for (int i = 0; i < (int)codetxt.size(); i++)
    {
        insstr.push_back(codetxt.at(i));
        if (bCanBeTextblock(insstr))
        {
            if (i == codetxt.size() - 1)
            {
                cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
                push_word(insstr.c_str(), codesen);
                continue;
            }
            insstr.push_back(codetxt.at(i + 1));
            if (bCanBeTextblock(insstr) == false)
            {
                insstr.pop_back();
                cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
                push_word(insstr.c_str(), codesen);
                insstr.clear();
            }
            else
            {
                int inssiz = insstr.size() - 1;
                char c = insstr.at(inssiz);
                bool bnor = ((('0' <= c && c <= '9') || ('a' <= c && c <= 'z')) || ('A' <= c && c <= 'Z') || c == '_');
                if (bnor == false)
                {
                    insstr.pop_back();
                    cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
                    push_word(insstr.c_str(), codesen);
                    insstr.clear();
                    insstr.push_back(c);
                    cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
                    push_word(insstr.c_str(), codesen);
                    insstr.clear();
                    i++;
                }
                else
                    insstr.pop_back();
            }
        }
        else
        {
            if (insstr.at(0) == '\"')
            {
                continue;
            }
            else
            {
                insstr.pop_back();
            }
        }
    }

    for (int i = 0; i < (int)codesen->size(); ++i)
    {
        fmlcstr s;
        s = (*codesen)[i];
        if (s == "-")
        {
            if (i == 0)
                continue;
            fmlcstr t0;
            t0 = (*codesen)[i - 1];
            fmlcstr t1;
            t1 = (*codesen)[i + 1];

            bool bequ = (DecodeTextBlock(t0) == TBT::_operation) && (DecodeTextBlock(t1) != TBT::_operation);
            if (bequ)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i];
                for (int k = 0; k < t1.size(); k++)
                {
                    insstr.push_back(t1[k]);
                }

                set_word(i, insstr.c_str(), codesen);
                (*codesen).erase(i + 1);
            }
        }

        if (s == "=")
        {
            if (i == 0)
                continue;

            char c = (*codesen)[i - 1][0];
            bool bequ = (c == '!' || c == '<') || (c == '=' || c == '>');
            bequ = bequ || ((c == '+' || c == '-') || (c == '*' || c == '/'));
            bequ = bequ || (c == '%');
            if (bequ)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i - 1];
                insstr.push_back('=');
                set_word(i, insstr.c_str(), codesen);
                codesen->erase(i);
            }
        }
        if (s == "|")
        {
            if (i == 0)
                continue;
            char c = (*codesen)[i + 1][0];
            bool bequ = (c == '|');
            if (bequ)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i];
                insstr.push_back('|');
                set_word(i, insstr.c_str(), codesen);
                codesen->erase(i + 1);
            }
        }
        if (s == "&")
        {
            if (i == 0)
                continue;
            char c = (*codesen)[i + 1][0];
            bool bequ = (c == '&');
            if (bequ)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i];
                insstr.push_back('&');
                set_word(i, insstr.c_str(), codesen);
                codesen->erase(i + 1);
            }
        }
        if (s == ".")
        {
            if (i - 1 < 0 && i + 1 > codesen->size() - 1)
                continue;
            fmlcstr front;
            front = (*codesen)[i - 1];
            fmlcstr back;
            back = (*codesen)[i + 1];
            bool bequ = true;
            for (int k = 0; k < front.size(); k++)
            {
                if (('0' <= front.at(k) && front.at(k) <= '9') == false)
                {
                    bequ = false;
                    break;
                }
            }
            for (int k = 0; k < back.size(); k++)
            {
                if (('0' <= back.at(k) && back.at(k) <= '9') == false)
                {
                    bequ = false;
                    break;
                }
            }
            if (bequ)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i - 1];
                insstr.push_back('.');
                for (int k = 0; k < back.size(); k++)
                {
                    insstr.push_back(back[k]);
                }
                set_word(i, insstr.c_str() , codesen);
                codesen->erase(i);
                codesen->erase(i);
            }
        }

        if (s == "\'")
        {
            if (strcmp((*codesen)[i + 2], "\'") == 0)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i];
                fmlcstr cent;
                cent = (*codesen)[i + 1];
                fmlcstr back;
                back = (*codesen)[i + 2];
                for (int k = 0; k < cent.size(); k++)
                {
                    insstr.push_back(cent.at(k));
                }

                for (int k = 0; k < back.size(); k++)
                {
                    insstr.push_back(back.at(k));
                }

                set_word(i, insstr.c_str(), codesen);
                codesen->erase(i + 1);
                codesen->erase(i + 1);
            }
            else if (strcmp((*codesen)[i + 1], "\\") == 0 && strcmp((*codesen)[i + 3], "\'") == 0)
            {
                fmlcstr insstr;
                insstr = (*codesen)[i];
                fmlcstr cent;
                cent = (*codesen)[i + 1];
                fmlcstr back;
                back = (*codesen)[i + 2];
                fmlcstr backback;
                backback = (*codesen)[i + 3];
                for (int k = 0; k < cent.size(); k++)
                {
                    insstr.push_back(cent.at(k));
                }

                for (int k = 0; k < back.size(); k++)
                {
                    insstr.push_back(back.at(k));
                }

                for (int k = 0; k < backback.size(); k++)
                {
                    insstr.push_back(backback.at(k));
                }

                set_word(i, insstr.c_str(), codesen);
                codesen->erase(i + 1);
                codesen->erase(i + 1);
                codesen->erase(i + 1);
            }
        }
    }

    insstr.islocal = true;
}

bool IsTypeString(const char *str, ICB_Extension* ext)
{
    for (int i = 0; i < InsideCode_Bake::basictype_max; ++i)
    {
        if (strcmp(str, InsideCode_Bake::basictype[i].name.c_str()) == 0)
            return true;
    }

    for(int i=0;i<ext->exstructArr.size();++i){
        if (strcmp(str, ext->exstructArr.at(i)->name.c_str()) == 0)
            return true;
    }
    return false;
}

fmvecarr<code_sen *> *AddCodeFromBlockData(fmvecarr<char *> &allcodesen, const char *ScanMod, ICB_Extension* ext)
{
    // allcode_sen-> allcode_sen
    // ic -> this
    // code -> sen_arr

    fmvecarr<code_sen *> *senarr =
        (fmvecarr<code_sen *> *)fm->_New(sizeof(fmvecarr<code_sen *>), true);
    senarr->NULLState();
    senarr->Init(10, false);

    bool readytoStart = true;
    int StartI = 0;
    for (int i = 0; i < (int)allcodesen.size() - 1; i++)
    {
        // codeKind ����
        if (readytoStart)
        {
            if (strcmp(ScanMod, "none") == 0)
            {
                if (strcmp(allcodesen[i], "struct") == 0)
                {
                    int open = 0;
                    int h = 0;
                    while (strcmp(allcodesen[i + h], "}") != 0 || open != 1)
                    {
                        if (strcmp(allcodesen[i + h], "{") == 0)
                            open++;
                        else if (strcmp(allcodesen[i + h], "}") == 0)
                            open--;
                        h++;
                    }

                    i += h;
                    StartI = i + 1;
                }
                if (IsTypeString(allcodesen[i], ext))
                {
                    int typeS = i;
                    int typeE = i;
                    while (true) {
                        if (strcmp(allcodesen[typeE + 1], "*") == 0) {
                            typeE += 1;
                        }
                        else if (strcmp(allcodesen[typeE + 1], "[") == 0) {
                            typeE += 3;
                        }
                        else {
                            break;
                        }
                    }

                    if (strcmp(allcodesen[typeE + 2], "(") == 0)
                    {
                        // addfunction
                        code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
                        *cs = code_sen();
                        cs->ck = codeKind::ck_addFunction;
                        fm->_tempPushLayer();
                        fmvecarr<char*> cbs;
                        cbs.NULLState();
                        cbs.Init(3, false, false);
                        for (int k = typeS; k < typeE + 2; ++k) {
                            cbs.push_back(allcodesen[k]);
                        }

                        int startI = typeE + 2;
                        int count = 0;
                        while (1)
                        {
                            if (strcmp(allcodesen[startI], "(") == 0)
                                count++;
                            else if (strcmp(allcodesen[startI], ")") == 0)
                                count--;

                            cbs.push_back(allcodesen[startI]);

                            startI++;

                            if (count == 0)
                            {
                                break;
                            }
                        }

                        InsideCode_Bake::set_codesen(cs, cbs);
                        fm->_tempPopLayer();

                        senarr->push_back(cs);
                        i = startI - 1;
                        StartI = i + 1;
                    }
                }
                else if (allcodesen.size() > i + 2 && (strcmp(allcodesen[i + 2], "(") == 0 && strcmp(allcodesen[i], "void") == 0))
                {
                    code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
                    *cs = code_sen();
                    cs->ck = codeKind::ck_addFunction;
                    fm->_tempPushLayer();
                    fmvecarr<char*> cbs;
                    cbs.NULLState();
                    cbs.Init(3, false, false);
                    cbs.push_back(allcodesen[i]);
                    cbs.push_back(allcodesen[i + 1]);

                    int startI = i + 2;
                    int count = 0;
                    while (1)
                    {
                        if (strcmp(allcodesen[startI], "(") == 0)
                            count++;
                        else if (strcmp(allcodesen[startI], ")") == 0)
                            count--;

                        cbs.push_back(allcodesen[startI]);

                        startI++;

                        if (count == 0)
                        {
                            break;
                        }
                    }

                    InsideCode_Bake::set_codesen(cs, cbs);
                    fm->_tempPopLayer();
                    senarr->push_back(cs);
                    i = startI - 1;
                    StartI = i + 1;
                }
                else if (strcmp(allcodesen[i], "{") == 0)
                {
                    code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
                    *cs = code_sen();
                    cs->ck = codeKind::ck_blocks;
                    fm->_tempPushLayer();
                    fmvecarr<char*> cbs;
                    cbs.NULLState();
                    cbs.Init(2, false, false);

                    int open = 0;
                    int h = 1;
                    // cbs.push_back(allcodesen[i]);
                    // cout << "block : { ";
                    while (i + h < allcodesen.size() && !(strcmp(allcodesen[i + h], "}") == 0 && open == 0))
                    {
                        if (strcmp(allcodesen[i + h], "{") == 0)
                            open++;
                        else if (strcmp(allcodesen[i + h], "}") == 0)
                            open--;

                        cbs.push_back(allcodesen[i + h]);
                        // cout << allcodesen[i + h] << ' ';

                        ++h;
                    }
                    // cout << "}" << endl;

                    // cbs.pop_back();
                    // cbs.erase(0);

                    fmvecarr<code_sen *> *cbv = AddCodeFromBlockData(cbs, "none", ext);

                    cs->codeblocks = (fmvecarr<int *> *)fm->_New(sizeof(fmvecarr<int *>), true);
                    cs->codeblocks->NULLState();
                    cs->codeblocks->Init(8, false, true);
                    cs->codeblocks->islocal = false;
                    //Init_VPTR<fmvecarr<int *> *>(cs->codeblocks);

                    for (int u = 0; u < (int)cbv->size(); u++)
                    {
                        cs->codeblocks->push_back(reinterpret_cast<int *>((*cbv)[u]));
                    }

                    cbv->release();
                    fm->_Delete((byte8 *)cbv, sizeof(fmvecarr<code_sen*>));

                    senarr->push_back(cs);
                    fm->_tempPopLayer();

                    i += h;
                    StartI = i + 1;
                }
            }

            if (strcmp(ScanMod, "struct") == 0)
            {
                if (strcmp(allcodesen[i], "struct") == 0)
                {
                    code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
                    *cs = code_sen();
                    cs->ck = codeKind::ck_addStruct;
                    fm->_tempPushLayer();
                    fmvecarr<char*> cbs;
                    cbs.NULLState();
                    cbs.Init(3, false, false);

                    int open = 0;
                    int h = 0;
                    cbs.push_back(allcodesen[i]);
                    while (strcmp(allcodesen[i + h], "}") != 0 || open != 1)
                    {
                        if (strcmp(allcodesen[i + h], "{") == 0)
                            open++;
                        else if (strcmp(allcodesen[i + h], "}") == 0)
                            open--;
                        h++;
                        cbs.push_back(allcodesen[i + h]);
                    }

                    fmvecarr<char *> bd;
                    bd.NULLState();
                    bd.Init(10, false, false);
                    for (int i = 0; i < cbs.size(); ++i)
                    {
                        bd.push_back(cbs[i]);
                    }
                    h = 0;
                    while (strcmp(bd[0], "{") != 0)
                    {
                        bd.erase(0);
                        h++;
                    }
                    bd.erase(0);
                    bd.pop_back();

                    fmvecarr<code_sen *> *cbv = AddCodeFromBlockData(bd, "none", ext);

                    cs->codeblocks = (fmvecarr<int *> *)fm->_New(sizeof(fmvecarr<int *>), true);
                    cs->codeblocks->NULLState();
                    cs->codeblocks->Init(8, false, true);

                    for (int u = 0; u < (int)cbv->size(); u++)
                    {
                        cs->codeblocks->push_back(reinterpret_cast<int *>((*cbv)[u]));
                    }

                    cbv->release();
                    fm->_Delete((byte8 *)cbv, sizeof(fmvecarr<code_sen*>));

                    InsideCode_Bake::set_codesen(cs, cbs);
                    fm->_tempPopLayer();
                    senarr->push_back(cs);
                }
            }
        }
    }

    return senarr;
}

type_data *get_type_with_namesen(sen *tname, ICB_Extension* ext)
{
    type_data *td = nullptr;
    type_data *ntd = nullptr;
    char *bt = tname->at(0).data.str;
    for (int i = 0; i < InsideCode_Bake::basictype_max; ++i)
    {
        if (strcmp(bt, InsideCode_Bake::basictype[i].name.c_str()) == 0){
            td = &InsideCode_Bake::basictype[i];
            break;
        }
    }
    if(td == nullptr){
        for(int i=0;i<ext->exstructArr.size();++i){
            if (strcmp(bt, ext->exstructArr.at(i)->name.c_str()) == 0){
                td = ext->exstructArr.at(i);
                break;
            }
        }
    }

    for (int i = 1; i < tname->size(); ++i)
    {
        char c = 0;
        if (tname->at(i).data.str != nullptr)
        {
            c = tname->at(i).data.str[0];
        }
        if (c == '*')
        {
            ntd = InsideCode_Bake::static_get_addpointer_type(td);
            if (i != 1)
            {
                fm->_Delete((byte8 *)td, sizeof(td));
            }
            td = ntd;
        }
        else if (c == '[')
        {
            int siz = atoi(tname->at(i + 1).data.str);
            ntd = InsideCode_Bake::static_get_array_type(td, siz);
            bool isRelease = true;
            if (td->typetype == 's') {
                for (int i = 0; i < ext->exstructArr.size(); ++i) {
                    if (strcmp(td->name.c_str(), ext->exstructArr.at(i)->name.c_str()) == 0) {
                        isRelease = false;
                        break;
                    }
                }
            }
            else if (td->typetype == 'b') {
                isRelease = false;
            }

            if (isRelease)
            {
                EXT_ReleaseTypeData(td, ext);
                td = nullptr;
            }
            td = ntd;
        }
    }
    return td;
}

void interpret_AddStruct(code_sen *cs, ICB_Extension *ext)
{
    sen* code = InsideCode_Bake::get_sen_from_codesen(cs);
    char* cname = code->at(1).data.str;
    int name_siz = strlen(cname) + 1;
    //char *name = (char *)fm->_New(name_siz, true);
    //strcpy(name, cname);
    struct_data* stdata = (struct_data*)fm->_New(sizeof(struct_data), true);
    stdata->name.NULLState();
    stdata->name.Init(8, false);
    stdata->name = cname;
    stdata->member_data.NULLState();
    stdata->member_data.Init(8, false);
    int cpivot = 3;
    int totalSiz = 0;
    while (cpivot < code->size() - 1)
    {
        int loc = InsideCode_Bake::wbss.search_word_first(cpivot, code, ";");
        sen* member_sen = InsideCode_Bake::wbss.sen_cut(code, cpivot, loc - 1);
        //InsideCode_Bake::wbss.dbg_sen(member_sen);
        NamingData nd;
        cname = member_sen->last().data.str;
        nd.name = cname; // (char *)fm->_New(strlen(cname) + 1, true);
        //strcpy(nd.name, cname);
        sen* type_sen = InsideCode_Bake::wbss.sen_cut(member_sen, 0, member_sen->size() - 1);
        type_data* td = get_type_with_namesen(type_sen, ext);
        nd.td = td;
        nd.add_address = totalSiz;
        totalSiz += td->typesiz;
        stdata->member_data.push_back(nd);
        cpivot = loc + 1;

        member_sen->release();
        fm->_Delete((byte8*)member_sen, sizeof(sen));

        type_sen->release();
        fm->_Delete((byte8*)type_sen, sizeof(sen));
    }
    type_data* newtype = (type_data*)fm->_New(sizeof(type_data), true);
    *newtype = InsideCode_Bake::create_type(stdata->name.c_str(), totalSiz, 's', reinterpret_cast<int*>(stdata));

    ext->exstructArr.push_back(newtype);
    code->release();
    fm->_Delete((byte8*)code, sizeof(sen));
}

void compile_addFunction(code_sen *cs, ICB_Extension *ext)
{
    func_data* fd = (func_data*)fm->_New(sizeof(func_data), true);
    fd->name.NULLState();
    fd->name.Init(2, false);

    sen* code = InsideCode_Bake::get_sen_from_codesen(cs);
    int loc = code->size() - 1;
    sen* inner_params = InsideCode_Bake::wbss.oc_search_inv(code, loc, "(", ")");
    int nameloc = loc - inner_params->size();

    fd->name = code->at(nameloc).data.str;
    //cout << fd->name.c_str() << endl;

    sen* typen = InsideCode_Bake::wbss.sen_cut(code, 0, nameloc - 1);
    type_data* td = get_type_with_namesen(typen, ext);
    typen->release();
    fm->_Delete((byte8*)typen, sizeof(sen));
    typen = nullptr;

    sen* params_sen = InsideCode_Bake::wbss.sen_cut(code, nameloc + 2, loc - 1);
    InsideCode_Bake::wbss.dbg_sen(params_sen);
    int coma = InsideCode_Bake::wbss.search_word_first(0, params_sen, ",");
    int savecoma = 0;
    int last = params_sen->size() - 1;

    fd->param_data.NULLState();
    fd->param_data.Init(2, false, true);
    fd->returntype = td;
    fd->start_pc = nullptr;

    int addadd = 0;

    if (last < 0)
    {
        ext->exfuncArr.push_back(fd);
        code->release();
        fm->_Delete((byte8*)code, sizeof(sen));

        params_sen->release();
        fm->_Delete((byte8*)params_sen, sizeof(sen));
        return;
    }

    while (coma != -1)
    {
        sen* param_sen = InsideCode_Bake::wbss.sen_cut(params_sen, savecoma, coma - 1);
        NamingData nd;

        sen* typestr = (sen*)fm->_New(sizeof(sen), true);
        typestr->NULLState();
        typestr->Init(2, false, true);
        for (int i = 0; i < param_sen->size() - 1; ++i)
        {
            typestr->push_back(param_sen->at(i));
        }

        nd.td = get_type_with_namesen(typestr, ext);
        nd.name = param_sen->last().data.str;

        addadd += nd.td->typesiz; //
        nd.add_address = addadd;

        fd->param_data.push_back(nd);
        savecoma = coma + 1;
        coma = InsideCode_Bake::wbss.search_word_first(savecoma, params_sen, ",");

        param_sen->release();
        fm->_Delete((byte8*)param_sen, sizeof(sen));

        typestr->release();
        fm->_Delete((byte8*)typestr, sizeof(sen));
    }

    //fm->dbg_fm1_lifecheck_charprint();

    sen* param_sen = InsideCode_Bake::wbss.sen_cut(params_sen, savecoma, last);
    //InsideCode_Bake::wbss.dbg_sen(param_sen);
    NamingData nd;

    sen* typestr = (sen*)fm->_New(sizeof(sen), true);
    typestr->NULLState();
    typestr->Init(2, false, true);
    for (int i = 0; i < param_sen->size() - 1; ++i)
    {
        typestr->push_back(param_sen->at(i));
    }

    //wbss.dbg_sen(typestr);
    nd.td = get_type_with_namesen(typestr, ext);
    nd.name = param_sen->last().data.str;

    addadd += nd.td->typesiz;
    nd.add_address = addadd;

    fd->param_data.push_back(nd);
    savecoma = coma + 1;

    ext->exfuncArr.push_back(fd);

    code->release();
    fm->_Delete((byte8*)code, sizeof(sen));

    param_sen->release();
    fm->_Delete((byte8*)param_sen, sizeof(sen));

    params_sen->release();
    fm->_Delete((byte8*)params_sen, sizeof(sen));

    inner_params->release();
    fm->_Delete((byte8*)inner_params, sizeof(sen));

    typestr->release();
    fm->_Delete((byte8*)typestr, sizeof(sen));
}

void compile_code(code_sen *cs, ICB_Extension *ext)
{
    switch (cs->ck)
    {
    case codeKind::ck_addFunction:
        compile_addFunction(cs, ext);
        break;
    }
}

void bake_Extension(const char* filename, ICB_Extension* ext){
    ofstream& icl = InsideCode_Bake::icl;
    bool icldetail = InsideCode_Bake::GetICLFlag(ICL_FLAG::Create_New_ICB_Extension_Init__Bake_Extension);
    if (icldetail) icl << "start" << endl;
    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension__GetCodeFromText...";
    fmlcstr *allcodeptr = GetCodeTXT(filename, fm);
    if (icldetail) icl << "finish" << endl;
	fmlcstr &allcode = *allcodeptr;
    fmvecarr<char*> codesen;
    codesen.NULLState();
    codesen.Init(8, false);

    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension__AddTextBlocks...";
	AddTextBlocks(allcode, &codesen);

    allcodeptr->release();
    allcodeptr->NULLState();
    fm->_Delete((byte8*)allcodeptr, sizeof(fmlcstr));

    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension__ScanStructTypes...";
	fmvecarr<code_sen *> *senstptr = AddCodeFromBlockData(codesen, "struct", ext);
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension__AddStructTypes...";
    for (int i = 0; i < senstptr->size(); ++i)
	{
		code_sen *cs = senstptr->at(i);
        if(cs->ck == codeKind::ck_addStruct){
            interpret_AddStruct(cs, ext);
        }
	}
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension__ScanFunctions...";
    fmvecarr<code_sen *> *senptr = AddCodeFromBlockData(codesen, "none", ext);
    if (icldetail) icl << "finish" << endl;

    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension__AddFunctions...";
    for (int i = 0; i < senptr->size(); ++i)
	{
		// fm->dbg_fm1_lifecheck();
		code_sen *cs = senptr->at(i);
        dbg_codesen(cs);
        if (cs->ck == codeKind::ck_addFunction) {
            compile_addFunction(cs, ext);
        }
	}

    codesen.release();
    codesen.NULLState();

    for (int i = 0; i < senstptr->size(); ++i) {
        code_sen* cs = senstptr->at(i);
        InsideCode_Bake::ReleaseCodeSen(cs);
        fm->_Delete((byte8*)cs, sizeof(code_sen));
        senstptr->at(i) = nullptr;
    }
    senstptr->release();
    senstptr->NULLState();
    fm->_Delete((byte8*)senstptr, sizeof(fmvecarr<code_sen*>));

    for (int i = 0; i < senptr->size(); ++i) {
        code_sen* cs = senptr->at(i);
        InsideCode_Bake::ReleaseCodeSen(cs);
        fm->_Delete((byte8*)cs, sizeof(code_sen));
        senptr->at(i) = nullptr;
    }
    senptr->release();
    senptr->NULLState();
    fm->_Delete((byte8*)senptr, sizeof(fmvecarr<code_sen*>));

    if (icldetail) icl << "finish" << endl;
    if (icldetail) icl << "Create_New_ICB_Extension_Init__Bake_Extension ";
}

#endif