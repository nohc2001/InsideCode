#ifndef EXTEND_FUNC_H
#define EXTEND_FUNC_H

#include "InsideCodeBake.h"

void push_word(lcstr &str, vecarr<char*>* codesen, InsideCode_Bake* icb)
{
    const char *sptr = str.c_str();
    int len = strlen(sptr);
    char *cstr = (char *)fm->_New(len + 1, true);

    for (int i = 0; i < len; ++i)
    {
        cstr[i] = sptr[i];
    }
    cstr[len] = 0;

    for (int i = 0; i < icb->wbss.wordlist.size(); ++i)
    {
        if (strcmp(cstr, icb->wbss.wordlist.at(i)->c_str()) == 0)
        {
            fm->_Delete((byte8 *)cstr, len + 1);
            codesen->push_back(icb->wbss.wordlist.at(i)->c_str());
            return;
        }
    }

    icb->wbss.addword(cstr);
    codesen->push_back(cstr);
}

void set_word(int index, lcstr &str, vecarr<char*>* codesen, InsideCode_Bake* icb)
{
    const char *sptr = str.c_str();
    int len = strlen(sptr);
    char *cstr = (char *)fm->_New(len + 1, true);
    strcpy(cstr, sptr);
    cstr[len] = 0;

    for (int i = 0; i < icb->wbss.wordlist.size(); ++i)
    {
        if (strcmp(cstr, icb->wbss.wordlist.at(i)->c_str()) == 0)
        {
            fm->_Delete((byte8 *)cstr, len + 1);
            (*codesen)[index] = icb->wbss.wordlist.at(i)->c_str();
            return;
        }
    }

    icb->wbss.addword(cstr);
    (*codesen)[index] = cstr;
}

void AddTextBlocks(lcstr &codetxt, vecarr<char*>* codesen, InsideCode_Bake* icb)
{
    lcstr insstr;
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
                push_word(insstr, codesen, icb);
                continue;
            }
            insstr.push_back(codetxt.at(i + 1));
            if (bCanBeTextblock(insstr) == false)
            {
                insstr.pop_back();
                cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
                push_word(insstr, codesen, icb);
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
                    push_word(insstr, codesen, icb);
                    insstr.clear();
                    insstr.push_back(c);
                    cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
                    push_word(insstr, codesen, icb);
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
        lcstr s;
        s = (*codesen)[i];
        if (s == "-")
        {
            if (i == 0)
                continue;
            lcstr t0;
            t0 = (*codesen)[i - 1];
            lcstr t1;
            t1 = (*codesen)[i + 1];

            bool bequ = (DecodeTextBlock(t0) == TBT::_operation) && (DecodeTextBlock(t1) != TBT::_operation);
            if (bequ)
            {
                lcstr insstr;
                insstr = (*codesen)[i];
                for (int k = 0; k < t1.size(); k++)
                {
                    insstr.push_back(t1[k]);
                }

                set_word(i, insstr, codesen, icb);
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
                lcstr insstr;
                insstr = (*codesen)[i - 1];
                insstr.push_back('=');
                set_word(i, insstr, codesen, icb);
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
                lcstr insstr;
                insstr = (*codesen)[i];
                insstr.push_back('|');
                set_word(i, insstr, codesen, icb);
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
                lcstr insstr;
                insstr = (*codesen)[i];
                insstr.push_back('&');
                set_word(i, insstr, codesen, icb);
                codesen->erase(i + 1);
            }
        }
        if (s == ".")
        {
            if (i - 1 < 0 && i + 1 > codesen->size() - 1)
                continue;
            lcstr front;
            front = (*codesen)[i - 1];
            lcstr back;
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
                lcstr insstr;
                insstr = (*codesen)[i - 1];
                insstr.push_back('.');
                for (int k = 0; k < back.size(); k++)
                {
                    insstr.push_back(back[k]);
                }
                set_word(i, insstr, codesen, icb);
                codesen->erase(i);
                codesen->erase(i);
            }
        }

        if (s == "\'")
        {
            if (strcmp((*codesen)[i + 2], "\'") == 0)
            {
                lcstr insstr;
                insstr = (*codesen)[i];
                lcstr cent;
                cent = (*codesen)[i + 1];
                lcstr back;
                back = (*codesen)[i + 2];
                for (int k = 0; k < cent.size(); k++)
                {
                    insstr.push_back(cent.at(k));
                }

                for (int k = 0; k < back.size(); k++)
                {
                    insstr.push_back(back.at(k));
                }

                set_word(i, insstr, codesen, icb);
                codesen->erase(i + 1);
                codesen->erase(i + 1);
            }
            else if (strcmp((*codesen)[i + 1], "\\") == 0 && strcmp((*codesen)[i + 3], "\'") == 0)
            {
                lcstr insstr;
                insstr = (*codesen)[i];
                lcstr cent;
                cent = (*codesen)[i + 1];
                lcstr back;
                back = (*codesen)[i + 2];
                lcstr backback;
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

                set_word(i, insstr, codesen, icb);
                codesen->erase(i + 1);
                codesen->erase(i + 1);
                codesen->erase(i + 1);
            }
        }
    }

    insstr.islocal = true;
}

void bake_Extension(const char* filename, InsideCode_Bake* icb, ICB_Extension* ext){
    lcstr *allcodeptr = GetCodeTXT(filename, fm);
	lcstr &allcode = *allcodeptr;
    vecarr<char*> codesen;
    codesen.NULLState();
    codesen.Init(8, false);
	AddTextBlocks(allcode, &codesen, icb);

	vecarr<code_sen *> *senstptr = icb->AddCodeFromBlockData(codesen, "struct");
    for (int i = 0; i < senstptr->size(); ++i)
	{
		code_sen *cs = senstptr->at(i);
        if(cs->ck == codeKind::ck_addStruct){
            icb->interpret_AddStruct(cs);
            ext->exstructArr.push_back((struct_data*)icb->types.last()->structptr);
        }
		//dbg_codesen(cs);
	}

    vecarr<code_sen *> *senptr = icb->AddCodeFromBlockData(codesen, "none");

    for (int i = 0; i < senptr->size(); ++i)
	{
		// fm->dbg_fm1_lifecheck();
		code_sen *cs = senptr->at(i);
        icb->compile_code(cs);
        if(cs->ck == codeKind::ck_addFunction){
            ext->exfuncArr.push_back(icb->functions.last());
        }
	}
}

void exfunc_Execute(int* pcontext){
    ICB_Context* cxt = reinterpret_cast<ICB_Context*>(pcontext);
}

ICB_Extension* Init_Extend(){
    //확장을 입력.
    ICB_Extension* icbext = (ICB_Extension*)fm->_New(sizeof(ICB_Extension), true);
    icbext->exfuncArr.NULLState();
    icbext->exfuncArr.Init(8, false);
    icbext->exstructArr.NULLState();
    icbext->exstructArr.Init(8, false);

    bake_Extension("extension_header.h", )
}

#endif