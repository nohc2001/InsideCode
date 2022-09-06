#include "main.h"
string Int::Type_name = "int";
string Float::Type_name = "float";
string Bool::Type_name = "bool";
string Char::Type_name = "char";
string MemoryValue::Type_name = "memory";

//데이터화된 코드가 들어갈 변수
InsideCode ic;

//준비 함수
ValuePtr::ValuePtr()
{
	if (isClassPtrDebug && DebugObjMap["ValuePtr"]) {
		++classPtrMap["ValuePtr"];
		printf("\n new ValuePtr :\t %p \t| ValuePtr stack : %d", this, classPtrMap["ValuePtr"]);
	}
}

ValuePtr* ValuePtr::AddValue(string Typename, string SubTypename, int arraySiz, Ptr* IC)
{
	if (Typename == "any") {
		ValuePtr* v = new ValuePtr();
		return v;
	}
	else if (Typename == "int") {
		Int* n = new Int();
		return (ValuePtr*)n;
	}
	else if (Typename == "float") {
		Float* n = new Float();
		return (ValuePtr*)n;
	}
	else if (Typename == "bool") {
		Bool* b = new Bool();
		return (ValuePtr*)b;
	}
	else if (Typename == "char") {
		Char* c = new Char();
		return (ValuePtr*)c;
	}
	else if (Typename == "memory") {
		MemoryValue* m = new MemoryValue();
		return (ValuePtr*)m;
	}
	else if (Typename == "pointer") {
		Pointer* p = new Pointer();
		p->preTypeName = SubTypename;
		SubTypename.push_back('*');
		p->TypeName = SubTypename;
		p->address = -1; // 아직 정해지지 않음.
		return (ValuePtr*)p;
	}
	else if (Typename == "array") {
		if (SubTypename != "") {
			InsideCode* ICptr = (InsideCode*)IC;
			CodePlayData* cpd = ((Process*)ICptr->process)->PresentPlayData();
			Array* a = new Array();
			a->siz = arraySiz;
			a->p.preTypeName = SubTypename;
			SubTypename.push_back('*');
			a->p.TypeName = SubTypename;
			a->p.address = AddVariableCore(a->p.preTypeName, "", 0, "", cpd, ICptr);
			for (int i = 1; i < arraySiz; i++) {
				AddVariableCore(a->p.preTypeName, "", 0, "", cpd, ICptr);
			}
			return (ValuePtr*)a;
		}
		else {
			InsideCode* ICptr = (InsideCode*)IC;
			CodePlayData* cpd = ((Process*)ICptr->process)->PresentPlayData();
			Array* a = new Array();
			a->siz = arraySiz;
			a->p.preTypeName = "";
			return (ValuePtr*)a;
		}
	}
	else {
		Struct* s = new Struct();
		bool ExistStruct = false;
		s->structType_Name = Typename;
		InsideCode* ICptr = (InsideCode*)IC;
		for (unsigned int i = 0; i < ICptr->typeList.size(); i++) { // IC를 매개변수로 받아야함
			if (ICptr->typeList.at(i)->type_name == 's') {
				Struct* basic = (Struct*)ICptr->typeList.at(i);
				if (basic->structType_Name == Typename) {
					ValuePtr::CopyValue((ValuePtr*)s, (ValuePtr*)basic, ICptr);
					ExistStruct = true;
				}
			}
		}

		if (ExistStruct == false) {
			CodePlayData* cpd = ((Process*)((InsideCode*)IC)->process)->PresentPlayData();
			PrintCodeLocation(cpd);
			printf("[ERROR] : %s 구조체가 정의되지 않았습니다. \n", Typename.c_str());
		}

		return (ValuePtr*)s;
		//struct의 경우
	}

	return nullptr;
}

ValuePtr::~ValuePtr()
{
	type_name = 0;

	if (isClassPtrDebug && DebugObjMap["ValuePtr"]) {
		--classPtrMap["ValuePtr"];
		printf("\n delete ValuePtr :\t %p \t| ValuePtr stack : %d", this, classPtrMap["ValuePtr"]);
		if (classPtrMap["ValuePtr"] < 0) {
			printf("more delete");
		}
	}
}

string ValuePtr::GetTypeName()
{
	return GetTypeStringFromChar(type_name);
}

bool ValuePtr::CopyValue(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	InsideCode* ICptr = (InsideCode*)IC;
	CodePlayData* cpd = ((Process*)((InsideCode*)IC)->process)->PresentPlayData();
	if ((v1 == nullptr || v2 == nullptr) || (v2->type_name != 'm' && v1->type_name != v2->type_name)) {
		PrintCodeLocation(cpd);
		printf("[ERROR] : 값이 할당되지 않았거나 타입이 같지 않아 복사에 실패했습니다.\n");
		return false;
	}
	else {
		if (v1->type_name == 'i') {
			Int* iv1 = (Int*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				if (ICptr->mem->memory[iv2->value]->type_name == 'i') {
					iv1->value = ((Int*)ICptr->mem->memory[iv2->value])->value;
				}
			}
			else {
				Int* iv2 = (Int*)v2;
				iv1->value = iv2->value;
			}
		}
		else if (v1->type_name == 'b') {
			Bool* iv1 = (Bool*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				if (ICptr->mem->memory[iv2->value]->type_name == 'b') {
					iv1->value = ((Bool*)ICptr->mem->memory[iv2->value])->value;
				}
			}
			else {
				Bool* iv2 = (Bool*)v2;
				iv1->value = iv2->value;
			}
		}
		else if (v1->type_name == 'f') {
			Float* iv1 = (Float*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				if (ICptr->mem->memory[iv2->value]->type_name == 'f') {
					iv1->value = ((Float*)ICptr->mem->memory[iv2->value])->value;
				}
			}
			else {
				Float* iv2 = (Float*)v2;
				iv1->value = iv2->value;
			}
		}
		else if (v1->type_name == 'c') {
			Char* iv1 = (Char*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				if (ICptr->mem->memory[iv2->value]->type_name == 'c') {
					iv1->value = ((Char*)ICptr->mem->memory[iv2->value])->value;
				}
			}
			else {
				Char* iv2 = (Char*)v2;
				iv1->value = iv2->value;
			}
		}
		else if (v1->type_name == 'p') {
			Pointer* iv1 = (Pointer*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				iv1->address = iv2->value;
			}
			else {
				Pointer* iv2 = (Pointer*)v2;
				iv1->address = iv2->address;
			}
		}
		else if (v1->type_name == 's') {
			Struct* iv1 = (Struct*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				if (ICptr->mem->memory[iv2->value]->type_name == 's') {
					Struct* sv2 = (Struct*)ICptr->mem->memory[iv2->value];
					if (iv1->structType_Name == sv2->structType_Name) {
						iv1->memberVnaming.clear();
						for (int i = 0; i < iv1->values.size(); ++i) {
							if (iv1->values[i] != nullptr) {
								delete iv1->values[i];
							}
						}
						if (iv1->values.size() > 0) {
							iv1->values.clear();
						}
						for (int i = 0; i < sv2->memberVnaming.size(); i++) {
							ValuePtr* vp = ValuePtr::AddValue(sv2->memberVnaming.at(i).type_name, "", 0, IC);
							CopyValue(vp, sv2->values.at(i), IC);
							iv1->values.push_back(vp);
							iv1->memberVnaming.push_back(sv2->memberVnaming.at(i));
						}
					}
				}
			}
			else {
				Struct* iv2 = (Struct*)v2;
				if (iv1->structType_Name == iv2->structType_Name) {
					iv1->memberVnaming.clear();
					for (int i = 0; i < iv1->values.size(); ++i) {
						if (iv1->values[i] != nullptr) {
							delete iv1->values[i];
						}
					}
					if (iv1->values.size() > 0) {
						iv1->values.clear();
					}
					for (int i = 0; i < iv2->memberVnaming.size(); i++) {
						ValuePtr* vp = ValuePtr::AddValue(iv2->memberVnaming.at(i).type_name, "", 0, IC);
						CopyValue(vp, iv2->values.at(i), IC);
						iv1->values.push_back(vp);
						iv1->memberVnaming.push_back(iv2->memberVnaming.at(i));
					}
				}
			}
		}
		else if (v1->type_name == 'a') {
			Array* a1 = (Array*)v1;
			if (v2->type_name == 'm') {
				MemoryValue* iv2 = (MemoryValue*)v2;
				if (ICptr->mem->memory[iv2->value]->type_name == 'a') {
					a1->p = ((Array*)ICptr->mem->memory[iv2->value])->p;
					a1->siz = ((Array*)ICptr->mem->memory[iv2->value])->siz;
				}
			}
			else if (v2->type_name == 'a') {
				Array* a2 = (Array*)v2;
				if (a2->p.preTypeName == "") { // 정해진 타입이 없을때
					for (int i = 0; i < a2->values.size(); i++) {
						ValuePtr::CopyValue(ic.mem->memory[a1->p.address + i], a2->values.at(i), IC);
					}
				}
				else {
					for (int i = 0; i < a2->values.size(); i++) {
						ValuePtr::CopyValue(ic.mem->memory[a1->p.address + i], ic.mem->memory[a2->p.address + i], IC);
					}
				}
			}
		}
	}
}

string ValuePtr::GetTypeNameFromValueString(string constant_value)
{
	if (constant_value == "true" || constant_value == "false") {
		return "bool";
	}
	
	int non = 0;
	bool beF = false;
	for (int i = 0; i < constant_value.size(); i++) {
		char c = constant_value.at(i);
		if (c < '0' || '9' < c) {
			if (c == '-' && i == 0) {

			}
			else if (c == '.') {
				beF = true;
			}
			else {
				non++;
			}
		}
	}

	if (non > 0) {
		return "null";
	}
	else {
		if (beF) return "float";
		else return "int";
	}
}

string ValuePtr::GetTypeStringFromChar(char tc)
{
	switch (tc) {
	case 'i':
		return "int";
	case 'f':
		return "float";
	case 'b':
		return "bool";
	case 'c':
		return "char";
	case 'm':
		return "memory";
	case 'p':
		return "pointer";
	case 'a':
		return "array";
	case 's':
		return "struct";
	}
	return "";
}

char ValuePtr::GetTypeCharFromString(string str)
{
	if (str == "int") {
		return 'i';
	}
	else if (str == "float") {
		return 'f';
	}
	else if (str == "bool") {
		return 'b';
	}
	else if (str == "char") {
		return 'c';
	}
	else if (str == "array") {
		return 'a';
	}
	else if (str == "memory") {
		return 'm';
	}
	else if (str == "pointer") {
		return 'p';
	}
	else if (str == "struct") {
		return 's';
	}
	return 0;
}

void ValuePtr::getValueToString(string str)
{
}

bool ValuePtr::isThisType(string str)
{
	return false;
}

textblock* ValuePtr::ValueToString()
{
	return nullptr;
}

void Int::getValueToString(string str)
{
	value = stoi(str);
}

bool Int::isThisType(string str)
{
	for (int i = 0; i < (int)str.size(); i++) {
		char c = str.at(i);
		if (i == 0 && c == '-') {
			continue;
		}

		if (('0' <= c && c <= '9') == false) {
			return false;
		}
	}
	return true;
}

textblock* Int::ValueToString()
{
	textblock* t = new textblock();
	*t = to_string(value);
	return t;
}

void Float::getValueToString(string str)
{
	value = stof(str);
}

bool Float::isThisType(string str)
{
	int dotnum = 0;
	for (int i = 0; i < (int)str.size(); i++) {
		char c = str.at(i);
		if (('0' <= c && c <= '9') == false && c != '.') {
			return false;
		}

		if (c == '.') {
			dotnum++;
		}

		if ((i == 0 && i == str.size() - 1) && c == '.') {
			return false;
		}
	}

	if (dotnum >= 2) {
		return false;
	}

	return true;
}

textblock* Float::ValueToString()
{
	textblock* t = new textblock();
	*t = to_string(value);
	return t;
}

void Bool::getValueToString(string str)
{
	if (str == "true") value = true;
	else value = false;
}

bool Bool::isThisType(string str)
{
	if (str == "true" || str == "false") {
		return true;
	}
	return false;
}

textblock* Bool::ValueToString()
{
	textblock* t;
	if (value) {
		t = new string("true");
	}
	else {
		t = new string("false");
	}
	return t;
}

void Char::getValueToString(string str)
{
	value = str.at(0);//0인것 같은데 1로 되어있었음.. 뭐지
}

bool Char::isThisType(string str)
{
	if (str.size() >= 3) {
		if (str.at(0) == '\'' && str.at(2) == '\'') {
			return true;
		}
		return false;
	}
	else return false;
}

textblock* Char::ValueToString()
{
	textblock* t = new textblock();
	t->push_back(value);
	return t;
}

textblock* Pointer::ValueToString()
{
	textblock* rt = new textblock();
	rt->push_back('{'); rt->push_back('p'); rt->push_back('t'); rt->push_back('r');
	rt->push_back(':');
	// 이 사이에 값을 넣어라
	string a = to_string(address);
	for (int i = 0; i < a.size(); i++) {
		rt->push_back(a.at(i));
	}
	rt->push_back('}');
	return rt;
}

void Pointer::getValueToString(string str)
{
	str.pop_back();
	string a;
	for (int i = 4; i < str.size(); i++) {
		a.push_back(str.at(i));
	}

	address = atoi(a.c_str());
}

textblock* Pointer::PointerToString(int address)
{
	textblock* rt = new textblock();
	rt->push_back('{'); rt->push_back('p'); rt->push_back('t'); rt->push_back('r');
	rt->push_back(':');
	// 이 사이에 값을 넣어라
	string a = to_string(address);
	for (int i = 0; i < a.size(); i++) {
		rt->push_back(a.at(i));
	}
	rt->push_back('}');
	return rt;
}

void Array::getValueToString(string str)
{
	if (str.size() >= 3 && (str.at(0) == '\"' && str.at(str.size() - 1) == '\"')) {
		//문자열일 경우
		siz = str.size() - 2;
		
		for (int i = 1; i < siz + 1; i++) {
			Char* c = new Char();
			c->value = str.at(i);
			values.push_back((ValuePtr*)c);
		}

		Char* c = new Char();
		c->value = '\0';
		values.push_back((ValuePtr*)c);
	}
	else {
		//그냥 배열일 경우
	}
}

void Array::ArrayInMemory(Ptr* ic)
{
	InsideCode* IC = ((InsideCode*)ic);
	int s = values.size();
	for (int i = 0; i < values.size(); ++i) {
		if (IC->mem->memory[p.address + i] != (ValuePtr*)values.at(i)) {
			ValuePtr::CopyValue(IC->mem->memory[p.address + i], (ValuePtr*)values.at(i), IC);
			delete values[i];
		}
	}
	if (s > 0) {
		values.clear();
	}
	
	for (int i = 0; i < s; ++i) {
		values.push_back(IC->mem->memory[p.address + i]);
	}
}

textblock* Array::ValueToString()
{
	textblock* rt = new textblock();
	if (p.preTypeName == "char") {
		for (int i = 0; i < values.size(); i++) {
			if (((Char*)values.at(i))->value == 0) {
				break;
			}
			rt->push_back(((Char*)values.at(i))->value);
		}
		return rt;
	}
	else {
		rt->push_back('{');
		for (int i = 0; i < values.size(); i++) {
			textblock* copyt = values.at(i)->ValueToString();
			for (int k = 0; k < copyt->size(); k++) {
				rt->push_back(copyt->at(k));
			}

			if (i < values.size() - 1) {
				rt->push_back(',');
			}
		}
		rt->push_back('}');
		return rt;
	}
	
}

bool Memory::AddDataToMemory(string mod, ValuePtr* v, int* nptr)
{
	static int stacksearch = 0;
	int limit, start;
	int* flow = nullptr;
	const char* errorMessage = "";
	if (mod == "global") {
		flow = &globalFlow;
		limit = globalLimit;
		start = 0;
		errorMessage = "[ERROR] : 전역변수 메모리 용량이 가득차서 더이상 값을 추가할 수 없습니다. \n";
	}
	else if (mod == "heap") {
		flow = &heapFlow;
		limit = heapLimit;
		start = globalLimit;
		errorMessage = "[ERROR] : 힙 메모리 용량이 가득차서 더이상 값을 추가할 수 없습니다. \n";
	}
	else if (mod == "stack") {
		flow = &stackFlow;
		limit = stackLimit;
		start = heapLimit;
		errorMessage = "[ERROR] : 힙 메모리 용량이 가득차서 더이상 값을 추가할 수 없습니다. \n";
	}


	bool success = false;
	if (memory[*flow] == nullptr) {
		memory[*flow] = v;
		success = true;
		if (nptr != nullptr) {
			*nptr = *flow;
		}
	}
	else {
		*flow = (*flow + 1 >= limit) ? *flow + 1 - limit + start : *flow + 1;
		stacksearch++;
		if (stacksearch > limit) {
			printf(errorMessage);
			stacksearch = 0;
		}
		success = AddDataToMemory(mod, v);
		if (success == false) return false;
	}

	if (success) {
		*flow = (*flow + 1 >= limit) ? *flow + 1 - limit + start : *flow + 1;
		if (v->type_name == 's') {
			Struct* s = (Struct*)v;
			for (int i = 0; i < s->values.size(); i++) {
				AddDataToMemory(mod, s->values.at(i));
			}
		}
	}
}

void Memory::RemoveData(int id)
{
	if (memory[id] == nullptr) return;

	char typestr = memory[id]->type_name;
	if (typestr == 'i') {
		Int* v = (Int*)memory[id];
		v->Type_name.clear();
		memory[id]->type_name = 0;
		delete v;
		memory[id] = nullptr;
	}
	else if (typestr == 'f') {
		Float* v = (Float*)memory[id];
		v->Type_name.clear();
		memory[id]->type_name = 0;
		delete v;
		memory[id] = nullptr;
	}
	else if (typestr == 'b') {
		Bool* v = (Bool*)memory[id];
		v->Type_name.clear();
		memory[id]->type_name = 0;
		delete v;
		memory[id] = nullptr;
	}
	else if (typestr == 's') {
		Struct* v = (Struct*)memory[id];
		v->structType_Name.clear();
		memory[id]->type_name = 0;
		for (int i = 0; i < v->memberVnaming.size(); i++) {
			RemoveData(v->memberVnaming.at(i).dataid);
		}
		v->memberVnaming.clear();
		v->values.clear();
		memory[id] = nullptr;
	}
}

string Memory::MemoryToString(int id)
{
	string rs;
	rs.push_back('{'); rs.push_back('l'); rs.push_back('o'); rs.push_back('c'); rs.push_back(':');
	string loc = to_string(id);
	for (int i = 0; i < loc.size(); i++) {
		rs.push_back(loc.at(i));
	}
	rs.push_back('}');
	loc.clear();
	return rs;
}

int Memory::StringToMemory(string str, Ptr* ICptr)
{
	if (str.at(0) == '{') {
		str.pop_back();
		string a;
		for (int i = 5; i < str.size(); i++) {
			a.push_back(str.at(i));
		}

		return atoi(a.c_str());
	}
	else {
		InsideCode* IC = (InsideCode*)ICptr;
		CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();

		return GetDataLocation(str, cpd, IC);
	}
}

textblock* Struct::ValueToString()
{
	textblock* rt = new textblock();
	rt->push_back('{');
	for (int i = 0; i < memberVnaming.size(); i++) {
		textblock* copyt = values.at(i)->ValueToString();
		for (int k = 0; k < copyt->size(); k++) {
			rt->push_back(copyt->at(k));
		}

		if (i < memberVnaming.size() - 1) {
			rt->push_back(',');
		}
	}
	rt->push_back('}');
	return rt;
}

Operator::Operator()
{
	form = "none";

	if (isClassPtrDebug && DebugObjMap["Operator"]) {
		++classPtrMap["Operator"];
		printf("\n new Operator :\t %p \t| Operator stack : %d", this, classPtrMap["Operator"]);
	}
}

Operator::Operator(string Name, string Form, string ParamType1, string ParamType2, string ReturnType, ValuePtr* (*func)(ValuePtr*, ValuePtr*, Ptr*))
{
	name = Name;
	form = Form;
	paramType[0] = ParamType1;
	paramType[1] = ParamType2;
	returnType = ReturnType;
	Func = func;

	if (isClassPtrDebug && DebugObjMap["Operator"]) {
		++classPtrMap["Operator"];
		printf("\n new Operator :\t %p \t| Operator stack : %d", this, classPtrMap["Operator"]);
	}
}

Operator::~Operator()
{
	if (isClassPtrDebug && DebugObjMap["Operator"]) {
		--classPtrMap["Operator"];
		printf("\n delete Operator :\t %p \t| Operator stack : %d", this, classPtrMap["Operator"]);
	}
}

ValuePtr* IntPercent(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("int", "", 0, IC);
	if (v1->type_name != 'i' || v2->type_name != 'i')
		return returnV;


	Int* A = (Int*)Casting("int", v1, IC);
	Int* B = (Int*)Casting("int", v2, IC);
	((Int*)returnV)->value = A->value % B->value;

	delete A;
	delete B;
	return returnV;
}

ValuePtr* Mul(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = nullptr;
	bool b = (v1->type_name != 'i' && v1->type_name != 'f') && v1->type_name != 'c';
	b = b || (v2->type_name != 'i' && v2->type_name != 'f') && v2->type_name != 'c';
	if (b) {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		return returnV;
	}

	b = v1->type_name == 'f' || v2->type_name == 'f';
	if (b) {
		returnV = ValuePtr::AddValue("float", "", 0, IC);
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		float f[2];
		f[0] = A->value;
		f[1] = B->value;
		((Float*)returnV)->value = f[0] * f[1];
		delete A;
		delete B;
		return returnV;
	}
	else {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Int*)returnV)->value = A->value * B->value;
		delete A;
		delete B;
		return returnV;
	}
}

ValuePtr* Div(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = nullptr;
	bool b = (v1->type_name != 'i' && v1->type_name != 'i') && v1->type_name != 'c';
	b = b || (v2->type_name != 'i' && v2->type_name != 'i') && v2->type_name != 'c';
	if (b) {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		return returnV;
	}

	b = v1->type_name == 'f' || v2->type_name == 'f';
	if (b) {
		returnV = ValuePtr::AddValue("float", "", 0, IC);
		float f[2];
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		f[0] = A->value;
		f[1] = B->value;
		((Float*)returnV)->value = f[0] / f[1];
		delete A;
		delete B;
		return returnV;
	}
	else {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Int*)returnV)->value = A->value / B->value;
		delete A;
		delete B;
		return returnV;
	}
}

ValuePtr* Plus(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = nullptr;
	bool b = (v1->type_name != 'i' && v1->type_name != 'f') && v1->type_name != 'c';
	b = b || (v2->type_name != 'i' && v2->type_name != 'f') && v2->type_name != 'c';
	if (b) {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		return returnV;
	}

	b = v1->type_name == 'f' || v2->type_name == 'f';
	if (b) {
		returnV = ValuePtr::AddValue("float", "", 0, IC);
		float f[2];
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		f[0] = A->value;
		f[1] = B->value;
		((Float*)returnV)->value = f[0] + f[1];
		delete A;
		delete B;
		return returnV;
	}
	else {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Int*)returnV)->value = A->value + B->value;
		delete A;
		delete B;
		return returnV;
	}
}

ValuePtr* Minus(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = nullptr;
	bool b = (v1->type_name != 'i' && v1->type_name != 'f') && v1->type_name != 'c';
	b = b || (v2->type_name != 'i' && v2->type_name != 'f') && v2->type_name != 'c';
	if (b) {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		return returnV;
	}

	b = v1->type_name == 'f' || v2->type_name == 'f';
	if (b) {
		returnV = ValuePtr::AddValue("float", "", 0, IC);
		float f[2];
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		f[0] = A->value;
		f[1] = B->value;
		((Float*)returnV)->value = f[0] - f[1];
		delete A;
		delete B;
		return returnV;
	}
	else {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Int*)returnV)->value = A->value - B->value;
		delete A;
		delete B;
		return returnV;
	}
}

ValuePtr* Same(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name == 'f' || v2->type_name == 'f') {
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		((Bool*)returnV)->value = A->value == B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'i' || v2->type_name == 'i') {
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Bool*)returnV)->value = A->value == B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'b' || v2->type_name == 'b') {
		Bool* A = (Bool*)Casting("bool", v1, IC);
		Bool* B = (Bool*)Casting("bool", v2, IC);
		((Bool*)returnV)->value = A->value == B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'c' || v2->type_name == 'c') {
		Char* A = (Char*)Casting("char", v1, IC);
		Char* B = (Char*)Casting("char", v2, IC);
		((Char*)returnV)->value = A->value == B->value;
		delete A;
		delete B;
	}
	return returnV;
}

ValuePtr* RBig(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name == 'f' || v2->type_name == 'f') {
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		((Bool*)returnV)->value = A->value < B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'i' || v2->type_name == 'i') {
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Bool*)returnV)->value = A->value < B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'b' || v2->type_name == 'b') {
		Bool* A = (Bool*)Casting("bool", v1, IC);
		Bool* B = (Bool*)Casting("bool", v2, IC);
		((Bool*)returnV)->value = A->value < B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'c' || v2->type_name == 'c') {
		Char* A = (Char*)Casting("char", v1, IC);
		Char* B = (Char*)Casting("char", v2, IC);
		((Char*)returnV)->value = A->value < B->value;
		delete A;
		delete B;
	}
	return returnV;
}

ValuePtr* LBig(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name == 'f' || v2->type_name == 'f') {
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		((Bool*)returnV)->value = A->value > B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'i' || v2->type_name == 'i') {
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Bool*)returnV)->value = A->value > B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'b' || v2->type_name == 'b') {
		Bool* A = (Bool*)Casting("bool", v1, IC);
		Bool* B = (Bool*)Casting("bool", v2, IC);
		((Bool*)returnV)->value = A->value > B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'c' || v2->type_name == 'c') {
		Char* A = (Char*)Casting("char", v1, IC);
		Char* B = (Char*)Casting("char", v2, IC);
		((Char*)returnV)->value = A->value > B->value;
		delete A;
		delete B;
	}
	return returnV;
}

ValuePtr* RBigSame(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name == 'f' || v2->type_name == 'f') {
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		((Bool*)returnV)->value = A->value <= B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'i' || v2->type_name == 'i') {
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Bool*)returnV)->value = A->value <= B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'b' || v2->type_name == 'b') {
		Bool* A = (Bool*)Casting("bool", v1, IC);
		Bool* B = (Bool*)Casting("bool", v2, IC);
		((Bool*)returnV)->value = A->value <= B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'c' || v2->type_name == 'c') {
		Char* A = (Char*)Casting("char", v1, IC);
		Char* B = (Char*)Casting("char", v2, IC);
		((Char*)returnV)->value = A->value <= B->value;
		delete A;
		delete B;
	}
	return returnV;
}

ValuePtr* LBigSame(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name == 'f' || v2->type_name == 'f') {
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		((Bool*)returnV)->value = A->value >= B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'i' || v2->type_name == 'i') {
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Bool*)returnV)->value = A->value >= B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name ==  'b' || v2->type_name == 'b') {
		Bool* A = (Bool*)Casting("bool", v1, IC);
		Bool* B = (Bool*)Casting("bool", v2, IC);
		((Bool*)returnV)->value = A->value >= B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'c' || v2->type_name == 'c') {
		Char* A = (Char*)Casting("char", v1, IC);
		Char* B = (Char*)Casting("char", v2, IC);
		((Char*)returnV)->value = A->value >= B->value;
		delete A;
		delete B;
	}
	return returnV;
}

ValuePtr* NotSame(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name == 'f' || v2->type_name == 'f') {
		Float* A = (Float*)Casting("float", v1, IC);
		Float* B = (Float*)Casting("float", v2, IC);
		((Bool*)returnV)->value = A->value != B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'i' || v2->type_name == 'i') {
		Int* A = (Int*)Casting("int", v1, IC);
		Int* B = (Int*)Casting("int", v2, IC);
		((Bool*)returnV)->value = A->value != B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'b' || v2->type_name == 'b') {
		Bool* A = (Bool*)Casting("bool", v1, IC);
		Bool* B = (Bool*)Casting("bool", v2, IC);
		((Bool*)returnV)->value = A->value != B->value;
		delete A;
		delete B;
	}
	else if (v1->type_name == 'c' || v2->type_name == 'c') {
		Char* A = (Char*)Casting("char", v1, IC);
		Char* B = (Char*)Casting("char", v2, IC);
		((Char*)returnV)->value = A->value != B->value;
		delete A;
		delete B;
	}
	return returnV;
}

ValuePtr* And(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name != 'b' || v2->type_name != 'b')
		return returnV;

	Bool* A = (Bool*)Casting("bool", v1, IC);
	Bool* B = (Bool*)Casting("bool", v2, IC);
	((Bool*)returnV)->value = A->value && B->value;
	delete A;
	delete B;
	return returnV;
}

ValuePtr* Or(ValuePtr* v1, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = ValuePtr::AddValue("bool", "", 0, IC);
	if (v1->type_name != 'b' || v2->type_name != 'b')
		return returnV;

	Bool* A = (Bool*)Casting("bool", v1, IC);
	Bool* B = (Bool*)Casting("bool", v2, IC);
	((Bool*)returnV)->value = A->value || B->value;
	delete A;
	delete B;
	return returnV;
}

ValuePtr* Casting(string type, ValuePtr* value, Ptr* IC)
{
	InsideCode* Ic = (InsideCode*)IC;

	if (type == "any") {
		ValuePtr* returnV = ValuePtr::AddValue(ValuePtr::GetTypeStringFromChar(value->type_name), "", 0, IC);
		if (value->type_name == 'i') {
			((Int*)returnV)->value = (int)((Int*)value)->value;
		}
		else if (value->type_name == 'f') {
			((Float*)returnV)->value = (float)((Float*)value)->value;
		}
		else if (value->type_name == 'b') {
			((Bool*)returnV)->value = (bool)((Bool*)value)->value;
		}
		else if (value->type_name == 'c') {
			((Char*)returnV)->value = (char)((Char*)value)->value;
		}
		else if (value->type_name == 'a') {
			((Array*)returnV)->p = ((Array*)value)->p;
			((Array*)returnV)->siz = ((Array*)value)->siz;
			int add = ((Array*)value)->p.address;
			for (int i = 0; i < ((Array*)value)->siz; i++) {
				((Array*)returnV)->values.push_back(Ic->mem->memory[add+i]);
			}
		}
		else if(value->type_name == 's') {
			if (type == ((Struct*)value)->structType_Name) { // 같은 타입의 구조체로 캐스팅할 경우
				Struct* v = (Struct*)value;
				Struct* rv = (Struct*)returnV;
				for (int i = 0; i < rv->memberVnaming.size(); i++) {
					ValuePtr::CopyValue(rv->values[i], v->values[i], IC);
				}
			}
		}

		return returnV;
	}

	ValuePtr* returnV = ValuePtr::AddValue(type, "", 0, IC);

	if (type == "int") {
		if (value->type_name == 'i') {
			((Int*)returnV)->value = (int)((Int*)value)->value;
		}
		if (value->type_name == 'f') {
			((Int*)returnV)->value = (int)((Float*)value)->value;
		}
		if (value->type_name == 'b') {
			((Int*)returnV)->value = (int)((Bool*)value)->value;
		}
		if (value->type_name == 'c') {
			((Int*)returnV)->value = (int)((Char*)value)->value;
		}
	}
	else if (type == "float") {
		if (value->type_name == 'i') {
			((Float*)returnV)->value = (float)((Int*)value)->value;
		}
		if (value->type_name == 'f') {
			((Float*)returnV)->value = (float)((Float*)value)->value;
		}
		if (value->type_name == 'b') {
			((Float*)returnV)->value = (float)((Bool*)value)->value;
		}
		if (value->type_name == 'c') {
			((Float*)returnV)->value = (float)((Char*)value)->value;
		}
	}
	else if (type == "bool") {
		if (value->type_name == 'i') {
			((Bool*)returnV)->value = (bool)((Int*)value)->value;
		}
		if (value->type_name == 'f') {
			((Bool*)returnV)->value = (bool)((Float*)value)->value;
		}
		if (value->type_name == 'b') {
			((Bool*)returnV)->value = (bool)((Bool*)value)->value;
		}
		if (value->type_name == 'c') {
			((Bool*)returnV)->value = (bool)((Char*)value)->value;
		}
	}
	else if (type == "char") {
		if (value->type_name == 'i') {
			((Char*)returnV)->value = (char)((Int*)value)->value;
		}
		if (value->type_name == 'f') {
			((Char*)returnV)->value = (char)((Float*)value)->value;
		}
		if (value->type_name == 'b') {
			((Char*)returnV)->value = (char)((Bool*)value)->value;
		}
		if (value->type_name == 'c') {
			((Char*)returnV)->value = (char)((Char*)value)->value;
		}
	}
	else {
		if (type == ((Struct*)value)->structType_Name) { // 같은 타입의 구조체로 캐스팅할 경우
			Struct* v = (Struct*)value;
			Struct* rv = (Struct*)returnV;
			for (int i = 0; i < rv->memberVnaming.size(); i++) {
				ValuePtr::CopyValue(rv->values[i], v->values[i], IC);
			}
		}
	}
	return returnV;
}

ValuePtr* operTwoValue(ValuePtr* v1, string oper, ValuePtr* v2, Ptr* IC)
{
	ValuePtr* returnV = nullptr;
	bool b = false;
	InsideCode* ICptr = (InsideCode*)IC;
	for (int i = 0; i < ICptr->operatorList.size(); i++) {
		string f = ICptr->operatorList.at(i)->form;
		if (f == oper) {
			b = true;
			returnV = ICptr->operatorList.at(i)->Func(v1, v2, IC);
			break;
		}
	}

	if (b == false) {
		returnV = ValuePtr::AddValue("int", "", 0, IC);
	}
	return returnV;
}

CodePlayData* Process::PresentPlayData()
{
	if (callArray.size() == 0) return nullptr;
	else {
		return callArray.at(callArray.size() - 1);
	}
}

void PrintCodeLocation(CodePlayData* cpd) {
	string FuncName = cpd->func->name;

	int n = 0;
	CodeBlock* codes = (CodeBlock*)cpd->func->codes;
	for (int i = 0; i < cpd->line.size(); i++) {
		n = cpd->line[i];
		codes = (CodeBlock*)codes->CodeBlocks.at(n);

		if (codes->CodeBlocks.size() != 0) {
			continue;
		}
		else {
			break;
		}
	}

	int siz = (codes->blockdata.size() > 20) ? 20 : codes->blockdata.size();
	printf("[Location] : ");
	printf("%s : ", cpd->func->name.c_str());
	for (int i = 0; i < cpd->line.size(); i++) {
		printf("%d - ", cpd->line[i]);
	}
	for (int i = 0; i < siz; i++) {
		printf("%s", codes->blockdata.at(i)->c_str());
	}

	if (codes->blockdata.size() > 20) {
		printf("...");
	}

	printf("\n");
}

void CodeDataization(InsideCode* ic, const char* filename)
{
	//code text를 먼저 가져온다.
	string codetxt = GetCodeTXT(filename);

	//textblock들 구성
	AddTextBlocks(codetxt, ic);

	//메모리 생성
	AllocateICMemory(ic);

	//구조체 구성
	ic->code = AddCodeFromBlockData(ic->ALLblockdata, ic, "struct scan");
	ScanStruct(ic);

	//코드 추출
	ic->code = AddCodeFromBlockData(ic->ALLblockdata, ic, "none");

	//전역변수 스캔
	ScanGlobalVariable(ic);

	//함수스캔
	ScanFunction(ic);
}

string GetCodeTXT(const char* filename)
{
	FILE* fp;
	if (fopen_s(&fp, filename, "rt") == 0) {
		string codetxt;
		int max = 0;
		fseek(fp, 0, SEEK_END);
		max = ftell(fp);
		fclose(fp);

		fopen_s(&fp, filename, "rt");
		int k = 0;
		while (k < max) {
			char c;
			c = fgetc(fp);
			codetxt.push_back(c);
			k++;
		}
		return codetxt;
	}
	else {
		printf("[ERROR] : %s 파일을 불러오는데 오류가 발생했습니다.\n", filename);
	}
}

void AddTextBlocks(string codetxt, InsideCode* IC)
{
	textblock insstr;
	for (int i = 0; i < (int)codetxt.size(); i++) {
		insstr.push_back(codetxt.at(i));
		if (bCanBeTextblock(insstr)) {
			if (i == codetxt.size() - 1) continue;
			insstr.push_back(codetxt.at(i + 1));
			if (bCanBeTextblock(insstr) == false) {
				insstr.pop_back();
				IC->ALLblockdata.push_back(tbClone(&insstr));
				insstr.clear();
			}
			else {
				int inssiz = insstr.size() - 1;
				char c = insstr.at(inssiz);
				bool bnor = (('0' <= c && c <= '9') || ('a' <= c && c <= 'z')) || ('A' <= c && c <= 'Z');
				if (bnor == false) {
					insstr.pop_back();
					IC->ALLblockdata.push_back(tbClone(&insstr));
					insstr.clear();
					insstr.push_back(c);
					IC->ALLblockdata.push_back(tbClone(&insstr));
					insstr.clear();
					i++;
				}
				else insstr.pop_back();
			}
		}
		else {
			if (insstr.at(0) == '\"') {
				continue;
			}
			else {
				insstr.pop_back();
			}
		}
	}

	for (int i = 0; i < (int)IC->ALLblockdata.size(); i++) {
		string s = *IC->ALLblockdata.at(i);
		if (s == "-") {
			if (i == 0) continue;
			textblock t0 = *IC->ALLblockdata.at(i - 1);
			textblock t1 = *IC->ALLblockdata.at(i + 1);
			
			bool bequ = (DecodeTextBlock(t0) == "operator") && (DecodeTextBlock(t1) != "operator");
			if (bequ) {
				for (int k = 0; k < t1.size(); k++) {
					IC->ALLblockdata.at(i)->push_back(t1.at(k));
				}

				delete IC->ALLblockdata[i + 1];
				IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i+1);
			}
		}

		if (s == "=") {
			if (i == 0) continue;
			char c = IC->ALLblockdata.at(i - 1)->at(0);
			bool bequ = (c == '!' || c == '<') || (c == '=' || c == '>');
			if (bequ) {
				IC->ALLblockdata.at(i - 1)->push_back('=');
				delete IC->ALLblockdata[i];
				IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i);
			}
		}
		if (s == "|") {
			if (i == 0) continue;
			char c = IC->ALLblockdata.at(i - 1)->at(0);
			bool bequ = (c == '|');
			if (bequ) {
				IC->ALLblockdata.at(i - 1)->push_back('|');
				delete IC->ALLblockdata[i];
				IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i);
			}
		}
		if (s == "&") {
			if (i == 0) continue;
			char c = IC->ALLblockdata.at(i - 1)->at(0);
			bool bequ = (c == '&');
			if (bequ) {
				IC->ALLblockdata.at(i - 1)->push_back('&');
				delete IC->ALLblockdata[i];
				IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i);
			}
		}
		if (s == ".") {
			if (i - 1 < 0 && i + 1 > IC->ALLblockdata.size() - 1) continue;
			string front = *IC->ALLblockdata.at(i-1);
			string back = *IC->ALLblockdata.at(i + 1);
			bool bequ = true;
			for (int k = 0; k < front.size(); k++) {
				if (('0' <= front.at(k) && front.at(k) <= '9') == false) {
					bequ = false;
					break;
				}
			}
			for (int k = 0; k < back.size(); k++) {
				if (('0' <= back.at(k) && back.at(k) <= '9') == false) {
					bequ = false;
					break;
				}
			}
			if (bequ) {
				IC->ALLblockdata.at(i - 1)->push_back('.');
				for (int k = 0; k < back.size(); k++) {
					IC->ALLblockdata.at(i - 1)->push_back(back.at(k));
				}
				delete IC->ALLblockdata[i];
				IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i);
				delete IC->ALLblockdata[i];
				IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i);
			}
		}

		if (s == "\'" && *IC->ALLblockdata.at(i + 2) == "\'") {
			string cent = *IC->ALLblockdata.at(i + 1);
			string back = *IC->ALLblockdata.at(i + 2);
			for (int k = 0; k < cent.size(); k++) {
				IC->ALLblockdata.at(i)->push_back(cent.at(k));
			}

			for (int k = 0; k < back.size(); k++) {
				IC->ALLblockdata.at(i)->push_back(back.at(k));
			}

			delete IC->ALLblockdata[i + 1];
			IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i + 1);
			delete IC->ALLblockdata[i + 1];
			IC->ALLblockdata.erase(IC->ALLblockdata.begin() + i + 1);
		}
	}
}

vector<CodeBlock*> AddCodeFromBlockData(vector<textblock*> bd, InsideCode* IC, string ScanMod) {
	vector<CodeBlock*> code;
	bool readytoStart = true;
	int StartI = 0;
	for (int i = 0; i < (int)bd.size()-1; i++) {
		// codeKind 결정
		if (readytoStart) {
			if (ScanMod == "none") {
				if (*bd.at(i) == "struct") {
					int open = 0;
					int h = 0;
					while (*bd.at(i + h) != "}" || open != 1) {
						if (*bd.at(i + h) == "{") open++;
						else if (*bd.at(i + h) == "}") open--;
						h++;
					}

					i += h;
					StartI = i + 1;
				}
				if (IsTypeString(*bd.at(i), IC)) {
					if (*bd.at(i + 2) == "(") {
						//함수일 경우
						CodeBlock* cb = new CodeBlock();
						cb->ck = codeKind::ck_addFunction;
						cb->CodeBlocks.clear();
						cb->naming.clear();
						cb->blockdata.push_back(bd.at(i));
						cb->blockdata.push_back(bd.at(i + 1));

						int startI = i + 2;
						int count = 0;
						while (1) {
							if (*bd.at(startI) == "(") count++;
							else if (*bd.at(startI) == ")") count--;

							cb->blockdata.push_back(bd.at(startI));

							startI++;

							if (count == 0) {
								break;
							}
						}

						code.push_back(cb);
						i = startI - 1;
						StartI = i + 1;
					}
					else {
						int k = 1;
						while (true) {
							if (*bd.at(i + k) != ";") {
								k++;
							}
							else {
								break;
							}
						}

						CodeBlock* cb = new CodeBlock();
						cb->ck = codeKind::ck_addVariable;
						cb->CodeBlocks.clear();
						cb->naming.clear();
						for (int j = 0; j < k; j++) {
							cb->blockdata.push_back(bd.at(i+j));
						}
						code.push_back(cb);
						i += k;
						StartI = i + 1;
					}
				}
				else if (bd.size() > i+2 && (*bd.at(i + 2) == "(" && *bd.at(i) == "void")) {
					//아무것도 반환하지 않는 함수일 경우
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_addFunction;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					cb->blockdata.push_back(bd.at(i));
					cb->blockdata.push_back(bd.at(i + 1));

					int startI = i + 2;
					int count = 0;
					while (1) {
						if (*bd.at(startI) == "(") count++;
						else if (*bd.at(startI) == ")") count--;

						cb->blockdata.push_back(bd.at(startI));

						startI++;

						if (count == 0) {
							break;
						}
					}

					code.push_back(cb);
					i = startI - 1;
					StartI = i + 1;
				}
				else if (*bd.at(i) == "{") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_blocks;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					int open = 0;
					int h = 0;
					cb->blockdata.push_back(bd.at(i));
					while (*bd.at(i + h) != "}" || open != 1) {
						if (*bd.at(i + h) == "{") open++;
						else if (*bd.at(i + h) == "}") open--;
						h++;
						cb->blockdata.push_back(bd.at(i + h));
					}
					// {} 지우기
					cb->blockdata.pop_back();
					cb->blockdata.erase(cb->blockdata.begin());
					//{}안의 CodeBlock 처리
					vector<CodeBlock*> cbv = AddCodeFromBlockData(cb->blockdata, IC, "none");
					vector<Ptr*> ptrv;
					for (int u = 0; u < (int)cbv.size(); u++) {
						ptrv.push_back((Ptr*)(cbv[u]));
					}
					cb->CodeBlocks = ptrv;
					code.push_back(cb);
					i += h;
					StartI = i+1;
				}
				else if (*bd.at(i) == "=") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_setVariable;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					int h = 0;
					for (int k = StartI; k < i; k++) {
						cb->blockdata.push_back(bd.at(k));
					}

					while (*bd.at(i + h) != ";") {
						cb->blockdata.push_back(bd.at(i + h));
						h++;
					}
					code.push_back(cb);
					i += h;
					StartI = i + 1;
				}
				else if (*bd.at(i) == "if") {
					//if는 소괄호까지만, codeblock에 담는다. if의 작동방식은 true면 다음 코드블록을 실행, 아니면 건너뛰는 것이다.
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_if;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					int open = 0;
					int h = 0;
					cb->blockdata.push_back(bd.at(i));
					while (*bd.at(i + h) != ")" || open != 1) {
						if (*bd.at(i + h) == "(") open++;
						else if (*bd.at(i + h) == ")") open--;
						h++;
						cb->blockdata.push_back(bd.at(i + h));
					}

					code.push_back(cb);
					i += h;
					StartI = i + 1;
				}
				else if (*bd.at(i) == "else") {
					if (bd.size() > i + 1 && *bd.at(i + 1) == "if") {
						CodeBlock* cb = new CodeBlock();
						cb->ck = codeKind::ck_if;
						cb->CodeBlocks.clear();
						cb->naming.clear();
						int open = 0;
						int h = 1;
						cb->blockdata.push_back(bd.at(i));
						cb->blockdata.push_back(bd.at(i+1));
						while (*bd.at(i + h) != ")" || open != 1) {
							if (*bd.at(i + h) == "(") open++;
							else if (*bd.at(i + h) == ")") open--;
							h++;
							cb->blockdata.push_back(bd.at(i + h));
						}

						code.push_back(cb);
						i += h;
						StartI = i + 1;
						//else if 의 경우
					}
					else {
						CodeBlock* cb = new CodeBlock();
						cb->ck = codeKind::ck_if;
						cb->CodeBlocks.clear();
						cb->naming.clear();
						//그냥 else의 경우
						cb->blockdata.push_back(bd.at(i));
						code.push_back(cb);
					}
				}
				else if (*bd.at(i) == "while") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_while;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					int open = 0;
					int h = 0;
					cb->blockdata.push_back(bd.at(i));
					while (*bd.at(i + h) != ")" || open != 1) {
						if (*bd.at(i + h) == "(") open++;
						else if (*bd.at(i + h) == ")") open--;
						h++;
						cb->blockdata.push_back(bd.at(i + h));
					}
					code.push_back(cb);
					i += h;
					StartI = i + 1;
				}
				else if (*bd.at(i + 1) == "(") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_useFunction;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					int h = 0;
					while (*bd.at(i + h) != ";") {
						cb->blockdata.push_back(bd.at(i + h));
						h++;
					}
					code.push_back(cb);
					i += h;
					StartI = i + 1;
				}
				else if (*bd.at(i) == "return") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_returnInFunction;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					int h = 0;
					while (*bd.at(i + h) != ";") {
						cb->blockdata.push_back(bd.at(i + h));
						h++;
					}
					code.push_back(cb);
					i += h;
					StartI = i + 1;
				}
				else if (*bd.at(i) == "break") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_break;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					cb->blockdata.push_back(bd.at(i));
					code.push_back(cb);
					StartI = i + 1;
				}
				else if (*bd.at(i) == "continue") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_continue;
					cb->CodeBlocks.clear();
					cb->naming.clear();
					cb->blockdata.push_back(bd.at(i));
					code.push_back(cb);
					StartI = i + 1;
				}
			}
			
			if (ScanMod == "struct scan") {
				if (*bd.at(i) == "struct") {
					CodeBlock* cb = new CodeBlock();
					cb->ck = codeKind::ck_addStruct;
					cb->CodeBlocks.clear();

					int open = 0;
					int h = 0;
					cb->blockdata.push_back(bd.at(i));
					while (*bd.at(i + h) != "}" || open != 1) {
						if (*bd.at(i + h) == "{") open++;
						else if (*bd.at(i + h) == "}") open--;
						h++;
						cb->blockdata.push_back(bd.at(i + h));
					}

					vector<textblock*> bd = cb->blockdata;
					h = 0;
					while (*bd.at(0) != "{") {
						bd.erase(bd.begin());
						h++;
					}
					bd.erase(bd.begin()); bd.pop_back();

					vector<CodeBlock*> cbv = AddCodeFromBlockData(bd, IC, "none");
					vector<Ptr*> ptrv;
					for (int u = 0; u < (int)cbv.size(); u++) {
						ptrv.push_back((Ptr*)(cbv[u]));
					}
					cb->CodeBlocks = ptrv;

					code.push_back(cb);
				}
			}
		}
	}

	return code;
}

bool IsTypeString(string str, InsideCode* IC)
{
	int n = IC->typeList.size();
	for (int i = 0; i < n; i++) {
		if (str == ValuePtr::GetTypeStringFromChar(IC->typeList.at(i)->type_name)) {
			return true;
		}
		else if (IC->typeList.at(i)->type_name == 's') {
			Struct* s = (Struct*)IC->typeList.at(i);
			if (str == s->structType_Name) {
				return true;
			}
		}
	}
	return false;
}

void DefineBasicTypes(InsideCode* IC)
{
	Int* basicInt = new Int();
	basicInt->value = 0;
	ValuePtr* vp = (ValuePtr*)basicInt;
	vp->type_name = 'i';
	IC->typeList.push_back(vp);

	Float* basicFloat = new Float();
	basicFloat->value = 0.f;
	basicFloat->Type_name = "float";
	ValuePtr* vpf = (ValuePtr*)basicFloat;
	vpf->type_name = 'f';
	IC->typeList.push_back(vpf);

	Bool* basicBool = new Bool();
	basicBool->value = false;
	basicBool->Type_name = "bool";
	ValuePtr* vpb = (ValuePtr*)basicBool;
	vpb->type_name = 'b';
	IC->typeList.push_back(vpb);

	Char* basicChar = new Char();
	basicInt->value = 0;
	ValuePtr* vpc = (ValuePtr*)basicChar;
	vpc->type_name = 'c';
	IC->typeList.push_back(vpc);

	Pointer* basicPointer = new Pointer();
	basicPointer->address = -1;
	basicPointer->preTypeName = "int";
	basicPointer->TypeName = "int*";
	ValuePtr* vptr = (ValuePtr*)basicPointer;
	vptr->type_name = 'p';
	IC->typeList.push_back(vptr);

	MemoryValue* basicMemoryValue = new MemoryValue();
	basicMemoryValue->Type_name = "memory";
	basicMemoryValue->value = -1;
	ValuePtr* memv = (ValuePtr*)basicMemoryValue;
	memv->type_name = 'm';
	IC->typeList.push_back(memv);

	Array* basicArray = new Array();
	basicArray->p = Pointer(*basicPointer);
	basicArray->siz = 0;
	ValuePtr* arrayv = (ValuePtr*)basicArray;
	arrayv->type_name = 'a';
	IC->typeList.push_back(arrayv);
}

void DefineBasicOperation(InsideCode* IC)
{
	FrontOperator* fopr = nullptr;
	fopr = new FrontOperator("Parentheses Open", "(", INEQ);
	IC->frontOperatorList.push_back(fopr);
	fopr = nullptr;
	fopr = new FrontOperator("Seek Array Index", "[", SeekIndex);
	IC->frontOperatorList.push_back(fopr);
	fopr = nullptr;
	fopr = new FrontOperator("Struct Point", ".", StructDots);
	IC->frontOperatorList.push_back(fopr);
	fopr = nullptr;
	fopr = new FrontOperator("Get Address", "&", GetAddress);
	IC->frontOperatorList.push_back(fopr);
	fopr = nullptr;
	fopr = new FrontOperator("Get Variable From Pointer", "*", GetVariableFromPointer);
	IC->frontOperatorList.push_back(fopr);
	

	Operator* oper = nullptr;
	oper = new Operator("mul", "*", "any", "any", "any", Mul);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("div", "/", "any", "any", "any", Div);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("percent", "%", "int", "int", "int", IntPercent);
	IC->operatorList.push_back(oper);
	oper = nullptr;

	oper = new Operator("int+", "+", "any", "any", "any", Plus);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("int-", "-", "any", "any", "any", Minus);
	IC->operatorList.push_back(oper);
	oper = nullptr;

	oper = new Operator("same", "==", "any", "any", "any", Same);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("rbig", "<", "any", "any", "any", RBig);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("lbig", ">", "any", "any", "any", LBig);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("rbigsame", "<=", "any", "any", "any", RBigSame);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("lbigsame", ">=", "any", "any", "any", LBigSame);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("notsame", "!=", "any", "any", "any", NotSame);
	IC->operatorList.push_back(oper);
	oper = nullptr;

	oper = new Operator("and", "&&", "any", "any", "any", And);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	oper = new Operator("or", "||", "any", "any", "any", Or);
	IC->operatorList.push_back(oper);
	oper = nullptr;
	
}

void AllocateICMemory(InsideCode* IC)
{
	IC->mem = new Memory(256, 64, 128, 256);
}

void ScanGlobalVariable(InsideCode* IC)
 {
	for (int i = 0; i < IC->code.size(); i++) {
		if (IC->code.at(i)->ck == codeKind::ck_addVariable) {
			int siz = IC->code.at(i)->blockdata.size();
			string typeN = *IC->code.at(i)->blockdata.at(siz - 2);
			string name = *IC->code.at(i)->blockdata.at(siz - 1);
			string subType;
			for (int k = 0; k < siz - 2; k++) {
				for (int j = 0; j < IC->code.at(i)->blockdata.at(k)->size(); j++) {
					char c = IC->code.at(i)->blockdata.at(k)->at(j);
					subType.push_back(c);
				}
			}

			AddVariableCore(typeN, subType, 0, name, nullptr, IC);
		}
	}
}

void GetDataAddVariableTexts(vector<textblock*> code, string* typeN, string* Name, string* subType, int* ArraySiz, string beStop, int startI, InsideCode* IC) {
	CodePlayData* cpd;
	if (((Process*)IC->process) == nullptr) {
		cpd = nullptr;
	}
	else {
		cpd = ((Process*)IC->process)->PresentPlayData();
	}

	int siz = 2;
	int k = startI+2;
	if (beStop != "") {
		while (true) {
			if (*code.at(k) == beStop) {
				siz = k;
				break;
			}
		}
	}
	else {
		siz = code.size();
	}
	
	// 이부분에서 배열을 어떻게 하는지..
	bool ar = false;

	*typeN = *code.at(siz - 2);
	int nn = 0;
	if (*typeN == "]") { //배열을 고려
		*typeN = "array";

		for (int k = siz-2; k > 0; k--) {
			if (*code.at(k) == "[") {
				nn = k;
			}
		}

		//siz값 추출
		vector<textblock*> numstr;
		for (int k = nn+1; k <= siz - 3; k++) {
			numstr.push_back(code.at(k));
		}
		Int* sizV = (Int*)GetDataOperation(numstr, IC, "int", cpd);
		*ArraySiz = sizV->value;

		ar = true;

		delete sizV;
	}
	else {
		*ArraySiz = 0;
	}

	*Name = *code.at(siz - 1);

	int n = ar ? nn : siz - 2;
	for (int k = 0; k < n; k++) {
		for (int j = 0; j < code.at(k)->size(); j++) {
			char c = code.at(k)->at(j);
			subType->push_back(c);
		}
	}
}

void ScanFunction(InsideCode* IC)
{
	bool addStart = false;
	IC->process = (Ptr*)(new Process());
	for (int i = 0; i < IC->code.size(); i++) {
		if (IC->code.at(i)->ck == codeKind::ck_addFunction) {
			CodeBlock* cb = IC->code.at(i + 1);
			Function* f = new Function();
			//함수내부 코드 저장
			f->codes = cb;

			string typeN, name, subType;
			int as;

			GetDataAddVariableTexts(IC->code.at(i)->blockdata, &typeN, &name, &subType, &as, "(", 0, IC);

			//함수 이름 지정
			f->name = name;

			//반환형식을 지정
			if (typeN != "void") {
				f->Return = ValuePtr::AddValue(typeN, subType, 0, IC);
			}
			else {
				f->Return = nullptr;
			}
			
			//파라미터를 입력받음
			int startI, endI;
			startI = 3;
			CodeBlock* cb2 = IC->code.at(i);
			for (int i = startI; i < cb2->blockdata.size(); i++) {
				textblock c = *cb2->blockdata.at(i);
				if (c == "," || c == ")") {
					endI = i - 1;
					if (endI - startI >= 1) {
						FuncParam fp;

						string tt, nn, st;
						int ac;
						GetDataAddVariableTexts(cb2->blockdata, &tt, &nn, &st, &ac, c, startI, IC);

						fp.valueType = ValuePtr::AddValue(tt, st, 0, IC);
						fp.name = nn;
						f->Parameter.push_back(fp);
						startI = i + 1;
					}
				}
			}

			//함수 등록
			IC->functions.push_back(f);

			//함수이름이 main일시, 가장 첫번째로 실행되는 함수로 지정.
			if (*IC->code.at(i)->blockdata.at(1) == "main") {
				((Process*)IC->process)->startFunction = IC->functions.size() - 1;
				CodePlayData* cpd = new CodePlayData();
				cpd->func = f;
				cpd->line.push_back(0);
				((Process*)IC->process)->callArray.push_back(cpd);
				addStart = true;
			}
		}
	}

	if (addStart == false) {
		printf("[ERROR] : 첫번째로 실행되는 함수가 지정되지 않았습니다.\n");
	}
}

void ScanStruct(InsideCode* IC)
{
	for (int i = 0; i < IC->code.size(); i++) {
		if (IC->code.at(i)->ck == codeKind::ck_addStruct) {
			Struct* basicStruct = new Struct();
			CodeBlock* cb = IC->code.at(i);
			basicStruct->structType_Name = *cb->blockdata.at(1);
			((ValuePtr*)basicStruct)->type_name = 's';
			for (int k = 0; k < cb->CodeBlocks.size(); k++) {
				CodeBlock* icb = (CodeBlock*)cb->CodeBlocks.at(k);
				if (icb->ck == codeKind::ck_addVariable) {
					string tt, nn, st;
					int ac;
					GetDataAddVariableTexts(icb->blockdata, &tt, &nn, &st, &ac, "", 0, IC);
					basicStruct->values.push_back(ValuePtr::AddValue(tt, st, 0, IC));
					NamingData nd;
					nd.dataid = k;
					nd.type_name = *icb->blockdata.at(0);
					nd.name = *icb->blockdata.at(1);
					basicStruct->memberVnaming.push_back(nd);
				}
			}

			IC->typeList.push_back((ValuePtr*)basicStruct);
		}
	}
}

vector<textblock*> INEQ(vector<textblock*> expression, Ptr* ICptr) {
	InsideCode* IC = (InsideCode*)ICptr;
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	for (int i = 0; i < expression.size(); i++) {
		if (*expression.at(i) == "(") {
			//범위 찾기
			int stack = 0;
			int endI = 0;
			for (int k = i; k < expression.size(); k++) {
				if (*expression.at(k) == "(") stack++;
				if (*expression.at(k) == ")") stack--;
				if (stack == 0) {
					endI = k;
					break;
				}
			}

			vector<textblock*> subExpression;
			for (int k = i + 1; k < endI; k++) {
				subExpression.push_back(expression.at(i+1));
				expression.erase(expression.begin() + i + 1);
			}

			ValuePtr* result = GetDataOperation(subExpression, IC, "none", cpd);
			if (result->type_name == 'm') {
				//변수 처리
				MemoryValue* mv = (MemoryValue*)result;
				textblock* t = new string(IC->mem->MemoryToString(mv->value));
				expression.erase(expression.begin() + i);
				expression.erase(expression.begin() + i);
				expression.insert(expression.begin() + i, t);
			}
			else {
				//값처리
				textblock* t = result->ValueToString();
				expression.erase(expression.begin() + i);
				expression.erase(expression.begin() + i);
				expression.insert(expression.begin() + i, t);
			}
			delete result;//
		}
	}
	return expression;
}

vector<textblock*> SeekIndex(vector<textblock*> expression, Ptr* ICptr)
{
	InsideCode* IC = (InsideCode*)ICptr;
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	for (int i = 0; i < expression.size(); i++) {
		if (*expression.at(i) == "[") {
			//범위 찾기
			int stack = i;
			int endI = 0;
			for (int k = i; k < expression.size(); k++) {
				if (*expression.at(k) == "]") {
					endI = k;
					break;
				}
			}

			//배열 인덱스의 주소 찾기
			vector<textblock*> subExpression;
			for (int k = i + 1; k < endI; k++) {
				subExpression.push_back(expression.at(i + 1));
				expression.erase(expression.begin() + i + 1);
			}
			Int* result = (Int*)GetDataOperation(subExpression, IC, "int", cpd);
			int index = GetDataLocation(*expression.at(i - 1), cpd, IC);
			Array* ThatArray;
			if (index > 0) {
				ThatArray = (Array*)IC->mem->memory[index];
				//ThatArray->p.address = index;

				int IndexAddress = ThatArray->p.address + result->value;

				//메모리에 변수 처리
				MemoryValue* mv = new MemoryValue();
				mv->value = IndexAddress;
				textblock* t = new string(IC->mem->MemoryToString(mv->value));
				expression.erase(expression.begin() + i-1);
				expression.erase(expression.begin() + i-1);
				expression.erase(expression.begin() + i - 1);
				if (expression.size() <= i-1) {
					expression.push_back(t);
				}
				else {
					expression.insert(expression.begin() + i - 1, t);
				}
				delete mv;
			}

			delete result;
		}
	}
	return expression;
}

vector<textblock*> StructDots(vector<textblock*> expression, Ptr* ICptr) {
	InsideCode* IC = (InsideCode*)ICptr;
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	for (int i = 1; i < expression.size(); i++) {
		if (*expression.at(i) == ".") {
			int lo = GetDataLocation(*expression.at(i - 1), cpd, IC);
			if (lo == -1) {
				PrintCodeLocation(cpd);
				printf("[ERROR] : %s변수를 찾을 수 없습니다.", expression.at(i - 1)->c_str());
			}
			ValuePtr* setVP;
			setVP = IC->mem->memory[lo];
			int search = i;
			int memNum = -1;
			while (true) {
				if (expression.size() - 1 <= search) break;
				if (*expression.at(search) == "." && setVP->type_name == 's') {
					Struct* svp = (Struct*)setVP;
					search++;
					memNum = GetNamingLocation(*expression.at(search), svp->memberVnaming);
					setVP = IC->mem->memory[lo + memNum + 1];
					lo = lo + memNum + 1;
					search++;
				}
				else break;
			}

			for (int k = 0; k < search - i + 1; k++) {
				expression.erase(expression.begin() + i - 1);
			}
			if (i - 1 < expression.size()) {
				textblock* t = new string(IC->mem->MemoryToString(lo));
				expression.insert(expression.begin() + i - 1, t);
			}
			else {
				textblock* t = new string(IC->mem->MemoryToString(lo));
				expression.push_back(t);
			}
		}
	}
	return expression;
}

vector<textblock*> GetAddress(vector<textblock*> expression, Ptr* ICptr) {
	InsideCode* IC = (InsideCode*)ICptr;
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	for (int i = 0; i < expression.size(); i++) {
		if (*expression.at(i) == "&") {
			int address = IC->mem->StringToMemory(*expression.at(i + 1), ICptr);
			expression.erase(expression.begin() + i);
			expression.erase(expression.begin() + i);
			expression.insert(expression.begin() + i, Pointer::PointerToString(address));
		}
	}
	return expression;
}

vector<textblock*> GetVariableFromPointer(vector<textblock*> expression, Ptr* ICptr)
{
	InsideCode* IC = (InsideCode*)ICptr;
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	for (int i = 0; i < expression.size(); i++) {
		if (*expression.at(i) == "*" && 
			(i == 0 || DecodeTextBlock(*expression.at(i-1)) == "operator") ) {
			int address = IC->mem->StringToMemory(*expression.at(i + 1), ICptr);
			ValuePtr* vp = IC->mem->memory[address];
			if (vp->type_name == 'p') {
				Pointer* p = (Pointer*)vp;
				address = p->address;
			}
			expression.erase(expression.begin() + i);
			expression.erase(expression.begin() + i);
			textblock* memt = new string(IC->mem->MemoryToString(address));
			expression.insert(expression.begin() + i, memt);
		}
	}
	return expression;
}


//메인
int main() {
	DebugObjMap["ValuePtr"] = false;
	DebugObjMap["Operator"] = false;
	DebugObjMap["FrontOperator"] = false;
	DebugObjMap["Int"] = false;
	DebugObjMap["Float"] = false;
	DebugObjMap["Bool"] = false;
	DebugObjMap["Char"] = false;
	DebugObjMap["MemoryValue"] = false;
	DebugObjMap["Struct"] = false;
	DebugObjMap["Pointer"] = false; // 포인터가 -1이 나옴 이게 왜 그러지
	DebugObjMap["Array"] = false;
	DebugObjMap["CodeBlock"] = false;
	DebugObjMap["FuncParam"] = false;
	DebugObjMap["Function"] = false;
	DebugObjMap["Memory"] = false;
	DebugObjMap["InsideCode"] = false; // 얘도 -1나옴
	DebugObjMap["CodePlayData"] = true; // 얘는 1 나옴
	DebugObjMap["Process"] = true; // 얘도 1 나옴

	//기본자료형 정의
	DefineBasicTypes(&ic);
	//기본 연산자 정의
	DefineBasicOperation(&ic);
	//코드 데이터화
	CodeDataization(&ic, "code.txt");

	//데이터화한 코드 실행
	CodePlayData* cpd = ((Process*)ic.process)->callArray.at(0);
	Float* v = (Float*)RunCode(cpd, &ic);

	//디버그 로그 띄우기
	printf("%g", v->value);

	if (v != nullptr) {
		delete v;
	}
	return 0;
}

void PrintMemory(CodeBlock* code, CodePlayData* cpd, InsideCode* IC) {
	system("cls");
	printf("%s", cpd->func->name.c_str());
	printf("\n");
	for (int i = 0; i < code->blockdata.size(); i++) {
		printf("%s ", code->blockdata.at(i)->c_str());
	}
	printf("\n");
	for (int i = 0; i < IC->mem->dataSiz; i++) {
		if (IC->mem->memory[i] != nullptr) {
			printf("[%d] %s \t", i, IC->mem->memory[i]->ValueToString()->c_str());
		}
		else {
			printf("[%d] %s \t", i, "null");
		}
		if (i % 7 == 0) {
			printf("\n");
		}
	}

	Sleep(1000);
	getchar();
}

ValuePtr* RunCode(CodePlayData* cpd, InsideCode* IC)
{
	ValuePtr* v = nullptr;
	for (int i = 0; i < (int)cpd->func->codes->CodeBlocks.size(); i++) {
		//if (i == 5) {
		//	printf("<Debug Point>");
		//}
		CodeBlock* code = (CodeBlock*)(cpd->func->codes->CodeBlocks.at(i));
		cpd->line[0] = i;
		v = StepCode(code, IC, &i);
		if (v != nullptr) {
			break;
		}

		if (cpd->BreakIsPlaying || cpd->ContinueIsPlaying) {
			return v;
		}

		//메모리 띄우기?
		//PrintMemory(code, cpd, IC);
	}
	return v;
}

ValuePtr* StepCode(CodeBlock* code, InsideCode* IC, int* i)
{
	CodePlayData* cpd = ((Process*)((InsideCode*)IC)->process)->PresentPlayData();
	int lastif = cpd->inIF.size() - 1;
	ValuePtr* v = nullptr;
	switch (code->ck) {
	case codeKind::ck_addVariable:
		AddVariable(code, IC);
		break;
	case codeKind::ck_setVariable:
		SetVariable(code, IC);
		break;
	case codeKind::ck_if:
		if (*code->blockdata.at(0) == "if") {
			//처음 if를 시작할때
			cpd->inIF.push_back(true);
			bool b = Execute_if(code, IC, i);
			cpd->IfIsTrue.push_back(b);
		}
		else if ((*code->blockdata.at(0) == "else" && cpd->inIF.size() != 0) && (cpd->IfIsTrue[lastif] && cpd->inIF[lastif])) {
			(*i)++;
			//CodeBlock* next = (CodeBlock*)(cpd->func->codes->CodeBlocks.at(*i + 1));
			if (code->ck != codeKind::ck_if) {
				cpd->inIF.pop_back();
				cpd->IfIsTrue.pop_back();
			}
		}
		else if ((*code->blockdata.at(0) == "else" && cpd->inIF.size() != 0) && (cpd->IfIsTrue[lastif] == false && cpd->inIF[lastif])) {
			bool b = Execute_if(code, IC, i);
			cpd->IfIsTrue[cpd->inIF.size() - 1] = b;
		}
		break;
	case codeKind::ck_while:
		Execute_While(code, IC, i);
		break;
	case codeKind::ck_useFunction:
		Excute_Function(code, IC);
		break;
	case codeKind::ck_returnInFunction:
		v = ReturnInFunction(code, IC);
		break;
	case codeKind::ck_blocks:
	{
		NamingDataList* NameDL = new NamingDataList();
		cpd->local_ndl.push_back(NameDL);

		for (int k = 0; k < (int)code->CodeBlocks.size(); k++) {
			if (k == 0) {
				cpd->line.push_back(0);
			}
			int n = cpd->line.size() - 1;
			cpd->line[n] = k;

			v = StepCode((CodeBlock*)code->CodeBlocks.at(k), IC, &k);

			if (cpd->BreakIsPlaying || cpd->ContinueIsPlaying) {
				NamingDataList* nameD = cpd->local_ndl.at(cpd->local_ndl.size() - 1);
				for (int k = 0; k < nameD->naming.size(); k++) {
					IC->mem->RemoveData(nameD->naming.at(k).dataid);
				}
				cpd->local_ndl.at(cpd->local_ndl.size() - 1)->naming.clear();
				delete nameD;
				cpd->local_ndl.pop_back();
				cpd->line.pop_back();
				return v;
			}
		}

		NamingDataList* nameD = cpd->local_ndl.at(cpd->local_ndl.size() - 1);
		for (int k = 0; k < nameD->naming.size(); k++) {
			IC->mem->RemoveData(nameD->naming.at(k).dataid);
		}
		cpd->local_ndl.at(cpd->local_ndl.size() - 1)->naming.clear();
		delete nameD;
		cpd->local_ndl.pop_back();
		cpd->line.pop_back();
	}
		break;
	case codeKind::ck_break:
		cpd->BreakIsPlaying = true;
		return v;
	case codeKind::ck_continue:
		cpd->ContinueIsPlaying = true;
		return v;
	default:
		PrintCodeLocation(cpd);
		printf("[ERROR] : codeKind가 정해지지 않았습니다. 비정상적인 코드를 해석하려 했습니다. \n");
		break;
	}

	return v;
}

void AddVariable(CodeBlock* code, InsideCode* IC)
{
	//static int updateID = 0;
	string tt, nn, st;
	int arraySiz = 0;
	GetDataAddVariableTexts(code->blockdata, &tt, &nn, &st, &arraySiz, "", 0, IC);
	AddVariableCore(tt, st, arraySiz, nn,
		((Process*)IC->process)->PresentPlayData(), IC);
}

int AddVariableCore(string type_name, string subTypeName, int ArraySiz, string name, CodePlayData* cpd, InsideCode* IC) {
	int n = 0;
	if (cpd == nullptr) {
		NamingData nd;
		if (type_name == "*") type_name = "pointer";
		nd.type_name = type_name;
		nd.name = name;

		//array일시에, siz를 어떻게 받을 건지 생각해라.
		ValuePtr* v = ValuePtr::AddValue(nd.type_name, subTypeName, ArraySiz, IC);
		
		IC->mem->AddDataToMemory("global", v, &n);
		nd.dataid = n;
		IC->naming.push_back(nd);
	}
	else {
		NamingData nd;
		nd.type_name = type_name;
		nd.name = name;

		ValuePtr* v = ValuePtr::AddValue(nd.type_name, subTypeName, ArraySiz, IC);
		IC->mem->AddDataToMemory("stack", v, &n);
		nd.dataid = n;

		int last = cpd->local_ndl.size() - 1;
		if (last >= 0) {
			cpd->local_ndl.at(last)->naming.push_back(nd);
		}
		else {
			NamingDataList* NameDL = new NamingDataList();
			NameDL->naming.push_back(nd);
			cpd->local_ndl.push_back(NameDL);
		}
	}

	return n; // 추가된 메모리의 주소
}

int GetDataLocationCore(textblock name, vector<NamingData> nd)
{
	for (int i = 0; i < (int)nd.size(); i++) {
		if (name == nd.at(i).name) {
			return nd.at(i).dataid;
		}
	}
	return 0;
}

int GetDataLocationCoreInBlock(textblock name, vector<NamingDataList*> nd)
{
	int last = nd.size() - 1;
	for (int i = last; i >= 0; i--) {
		NamingDataList* nameD = nd.at(i);
		for (int k = 0; k < nameD->naming.size(); k++) {
			if (name == nameD->naming.at(k).name) {
				return  nameD->naming.at(k).dataid;
			}
		}
	}
	return 0;
}

void SetVariable(CodeBlock* code, InsideCode* IC)
{
	vector<textblock*> t;
	vector<textblock*> e;
	bool front = true;
	for (int i = 0; i < code->blockdata.size(); i++) {
		if (*code->blockdata.at(i) == "=") {
			front = false;
			continue;
		}
		else {
			if (front) {
				t.push_back(code->blockdata.at(i));
			}
			else {
				e.push_back(code->blockdata.at(i));
			}
		}
	}
	
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	int search = 1;
	MemoryValue* L_Value = (MemoryValue*)GetDataFrontOperation(t, IC, cpd);
	if (L_Value == nullptr) {
		PrintCodeLocation(cpd);
		printf("[ERROR] : 변수 %s 가 정의되지 않았습니다.\n", t.at(0)->c_str());
		return;
	}

	
	//int lo = GetDataLocation(*t.at(0), cpd, IC);
	//if (lo == -1) {
	//	PrintCodeLocation(cpd);
	//	printf("[ERROR] : 변수 %s 가 정의되지 않았습니다.\n", t.at(0)->c_str());
	//	return;
	//}
	ValuePtr* setVP;
	setVP = IC->mem->memory[L_Value->value];
	//while (true) {
	//	if (t.size() - 1 <= search) break;
	//	if (*t.at(search) == "." && setVP->type_name == "struct") {
	//		Struct* svp = (Struct*)setVP;
	//		search++;
	//		int n = GetNamingLocation(*t.at(search), svp->memberVnaming);
	//		if (n == -1) {
	//			PrintCodeLocation(cpd);
	//			printf("[ERROR] : 구조체 %s 의 맴버변수 %s 가 정의되지 않았습니다.\n", svp->structType_Name.c_str() , t.at(search)->c_str());
	//			return;
	//		}
	//		setVP = svp->values.at(n);
	//		search++;
	//	}
	//}
	
	for (int i = 0; i < IC->typeList.size(); i++) {
		char tname = IC->typeList.at(i)->type_name;
		if (setVP->type_name == tname) { // 포인터를 어떻게 처리할건지 
			//ValuePtr* value = ValuePtr::AddValue(tname, "", IC); ?
			ValuePtr* value = GetDataOperation(e, IC, ValuePtr::GetTypeStringFromChar(tname), cpd);
			ValuePtr::CopyValue(setVP, value, IC);
			delete value;
			break;
		}
	}

	if (L_Value != nullptr) {
		delete L_Value;
	}
}

void SetVariableCore(string type_name, string name, InsideCode* IC, CodePlayData* cpd, ValuePtr* vp) {
	if (cpd == nullptr) {
		int lo = GetDataLocationCore(name, IC->naming);
		if (IC->mem->memory[lo]->type_name == type_name) {
			 ValuePtr::CopyValue(IC->mem->memory[lo], vp, IC);
		}
	}
	else {
		int lo = GetDataLocationCoreInBlock(name, cpd->local_ndl);
		if (IC->mem->memory[lo]->type_name == type_name) {
			ValuePtr::CopyValue(IC->mem->memory[lo], vp, IC);
		}
		else if (IC->mem->memory[lo]->type_name == "struct") {
			//struct의 맴버변수들을 메모리에 먼저 만든다.
			Struct* com = ((Struct*)vp);
			for (int i = 0; i < com->values.size(); i++) {
				ValuePtr* vptr = ValuePtr::AddValue(com->memberVnaming.at(i).type_name.c_str(), "", 0, IC);
				ValuePtr::CopyValue(vptr, com->values.at(i), IC);
				ValuePtr::CopyValue(IC->mem->memory[lo+1+i], vptr, IC);
				delete vptr;
			}

			//그 후 멤버변수를 가지는 구조체표식을 메모리에 만든다.
			if (((Struct*)IC->mem->memory[lo])->structType_Name == type_name) {
				//IC->mem->memory[lo] = vp;
				ValuePtr::CopyValue(IC->mem->memory[lo], vp, IC);
				com = (Struct*)IC->mem->memory[lo];

				for (int i = 0; i < com->values.size(); i++) {
					com->values[i] = IC->mem->memory[lo + 1 + i];
				}
			}
		}
	}
	
}

void StandardOutputFunction(CodeBlock* code, InsideCode* IC)
{
	//함수의 이름과 괄호를 뺀다. - 매개변수만 남게 한다.
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	vector<textblock*> b = code->blockdata;
	b.erase(b.begin());
	b.erase(b.begin());
	b.pop_back();

	string Format;
	int k = 2;
	for (int i = 0; i < b.at(0)->size(); i++) {
		char c = b.at(0)->at(i);
		char nc = 0;
		if (i + 1 < b.at(0)->size()) {
			nc = b.at(0)->at(i + 1);
		}
			
		if (c == '\"') {

		}
		else if (c == '%' && nc == 'v') {
			vector<textblock*> expression;
			int inout = 0;
			while (k < b.size() && !(inout == 0 && *b.at(k) == ",")) {
				if (*b.at(k) == "(") inout++;
				if (*b.at(k) == ")") inout--;

				expression.push_back(b.at(k));
				k++;
			}

			ValuePtr* v = GetDataOperation(expression, IC, "any", cpd); // 3개 생김
			
			textblock* valueBlock = v->ValueToString();
			for (int h = 0; h < valueBlock->size(); h++) {
				Format.push_back(valueBlock->at(h));
			}

			k++;
			i++;

			delete v;
		}
		else if (c == '\\' && nc == 'n') {
			Format.push_back('\n');
			i++;
		}
		else {
			Format.push_back(c);
		}
	}

	printf(Format.c_str());
}

void StandardInputFunction(CodeBlock* code, InsideCode* IC)
{
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	vector<textblock*> b = code->blockdata;
	b.erase(b.begin());
	b.erase(b.begin());
	b.pop_back();

	ValuePtr* m = GetDataFrontOperation(b, IC, cpd);
	if (m != nullptr && m->type_name == "memory") {
		MemoryValue* mv = (MemoryValue*)m;
		int lo = mv->value;
		ValuePtr* variable = IC->mem->memory[lo];
		if (variable->type_name == "int") {
			scanf_s("%d", &(((Int*)variable)->value));
		}
		else if (variable->type_name == "float") {
			scanf_s("%f", &(((Float*)variable)->value));
		}
		else if (variable->type_name == "bool") {
			scanf_s("%d", &(((Bool*)variable)->value) );
		}
		else if (variable->type_name == "char") {
			scanf_s("%c", &(((Char*)variable)->value));
		}
		else if (variable->type_name == "array") {
			if (((Array*)variable)->p.preTypeName == "char") {
				Array* v = (Array*)variable;
				char* str = new char[v->siz];
				scanf_s("%s", str, v->siz);
				int len = strlen(str);
				//string의 경우
				for (int i = 0; i < len; ++i) {
					Char* c = (Char*)ValuePtr::AddValue("char", "", 0, (Ptr*)IC);
					c->value = str[i];
					v->values.push_back((ValuePtr*)c);
				}
				v->ArrayInMemory((Ptr*)IC);
				delete[] str;
			}
		}

		//else {
		//	Struct* sv = (Struct*)variable;
		//	printf("%s struct input...\n", sv->structType_Name.c_str());
		//	for (int i = 0; i < sv->memberVnaming.size(); i++) {
		//		printf("(%s) %s : \t", sv->memberVnaming.at(i).type_name, sv->memberVnaming.at(i).name);
		//		ValuePtr* variable = IC->mem->memory[lo+1+i];
		//		
		//	}
		//}
	}
}

ValuePtr* StandardSizeofFunction(CodeBlock* code, InsideCode* IC)
{
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	vector<textblock*> b = code->blockdata;
	b.erase(b.begin());
	b.erase(b.begin());
	b.pop_back();

	ValuePtr* value = GetDataOperation(b, IC, "none", cpd);
	if (value->type_name == "array") {
		Array* a = (Array*)value;
		Int* sv = (Int*)ValuePtr::AddValue("int", "", 0, IC);
		sv->value = a->siz;
		return (ValuePtr*)sv;
	}
	else if (value->type_name == "struct") {
		Struct* s = (Struct*)value;
		Int* sv = (Int*)ValuePtr::AddValue("int", "", 0, IC);
		sv->value = s->memberVnaming.size();
		return (ValuePtr*)sv;
	}
	else {
		Int* sv = (Int*)ValuePtr::AddValue("int", "", 0, IC);
		sv->value = 1;
		return (ValuePtr*)sv;
	}
}

bool Execute_if(CodeBlock* code, InsideCode* IC, int* i) {
	vector<textblock*> e = code->blockdata;
	if (*e.at(0) == "else") {
		if (e.size() > 1) {
			e.erase(e.begin()); e.erase(e.begin()); e.erase(e.begin());
			e.pop_back();

			CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
			Bool* b = (Bool*)GetDataOperation(e, IC, "bool", cpd);
			bool returnbool = b->value;
			if (returnbool == false) (*i)++;
			delete b;
			return returnbool;
		}
		else {
			return true;
		}
	}
	else {
		e.erase(e.begin()); e.erase(e.begin());
		e.pop_back();

		CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
		Bool* b = (Bool*)GetDataOperation(e, IC, "bool", cpd);
		bool returnbool = b->value;
		if (returnbool == false) (*i)++;
		delete b;
		return returnbool;
	}
}

void Execute_While(CodeBlock* code, InsideCode* IC, int* i)
{
	vector<textblock*> e = code->blockdata;
	e.erase(e.begin()); e.erase(e.begin());
	e.pop_back();
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	while (true) {
		Bool* b = (Bool*)GetDataOperation(e, IC, "bool", cpd);
		bool returnbool = b->value;
		delete b;
		if (returnbool) {
			CodeBlock* code2 = ((CodeBlock*)cpd->func->codes->CodeBlocks.at(*i+1));
			StepCode(code2, IC, i); // 여기에서 i는 아무의미가 없음 어차피 block에서 k로 들어가기 때문.
			if (cpd->BreakIsPlaying) {
				cpd->BreakIsPlaying = false;
				(*i)++;
				break;
			}
			else if (cpd->ContinueIsPlaying) {
				cpd->ContinueIsPlaying = false;
				continue;
			}
		}
		else {
			(*i)++;
			break;
		}
	}
}

ValuePtr* Excute_Function(CodeBlock* code, InsideCode* IC)
{
	ValuePtr* v = nullptr;
	//함수의 이름
	string n = *code->blockdata.at(0);
	bool callFunc = false;
	CodePlayData* cpdp = ((Process*)IC->process)->PresentPlayData();
	for (int i = 0; i < IC->functions.size(); i++) {
		if (IC->functions.at(i)->name == n) {
			callFunc = true;

			CodePlayData* cpd = new CodePlayData();
			cpd->func = IC->functions.at(i);
			cpd->line.push_back(0);

			int paramCount = 0;
			vector<textblock*> expression;
			for (int k = 2; k < code->blockdata.size(); k++) {
				textblock* t = code->blockdata.at(k);
				if (*t != "," && k != code->blockdata.size() - 1) {
					expression.push_back(t);
				}
				else {
					FuncParam fp;
					if (paramCount > cpd->func->Parameter.size() - 1) {
						PrintCodeLocation(cpdp);
						printf("[ERROR] : %s 함수호출에 너무 많은 파라미터가 입력되어 일부가 제외되었습니다. \n", n.c_str());
						break;
					}
					else {
						if (cpd->func->Parameter.size() > 0) {
							fp = cpd->func->Parameter.at(paramCount);
							if (fp.valueType->type_name == "struct") {
								string tstr = ((Struct*)fp.valueType)->structType_Name;
								AddVariableCore(tstr, "", 0, fp.name, cpd, IC);
								ValuePtr* v = GetDataOperation(expression, IC, tstr, cpdp);
								SetVariableCore(tstr, fp.name, IC, cpd, v);
								delete v;
							}
							else {
								AddVariableCore(fp.valueType->type_name, "", 0, fp.name, cpd, IC);
								ValuePtr* v = GetDataOperation(expression, IC, fp.valueType->type_name, cpdp);
								SetVariableCore(fp.valueType->type_name, fp.name, IC, cpd, v);
								delete v;
							}

							paramCount++;
							expression.clear();
						}
					}
				}
			}
			
			((Process*)IC->process)->callArray.push_back(cpd);
			v = RunCode(cpd, IC);
			for (int u = 0; u < cpd->local_ndl.size(); u++) {
				NamingDataList* nameD = cpd->local_ndl.at(0);
				for (int k = 0; k < nameD->naming.size(); k++) {
					IC->mem->RemoveData(nameD->naming.at(k).dataid);
				}
				cpd->local_ndl.erase(cpd->local_ndl.begin() + 0);
			}
			
			((Process*)IC->process)->callArray.pop_back();
			break;
		}
	}

	if (callFunc == false) {
		//표준함수 작동
		if (n == "output") {
			StandardOutputFunction(code, IC);
		}
		else if (n == "input") {
			StandardInputFunction(code, IC);
		}
		else if (n == "sizeof") {
			v = StandardSizeofFunction(code, IC);
		}
		else {
			PrintCodeLocation(cpdp);
			printf("[ERROR] : %s 함수가 정의되지 않았습니다.", n.c_str());
		}
	}
	return v;
}

ValuePtr* ReturnInFunction(CodeBlock* code, InsideCode* IC) {
	ValuePtr* v = nullptr;
	vector<textblock*> e = code->blockdata;
	//return을 지운다.
	e.erase(e.begin());

	vector<CodePlayData*> cpdArray = ((Process*)IC->process)->callArray;
	if (cpdArray.at(cpdArray.size() - 1)->func->Return != nullptr) {
		string rtype = cpdArray.at(cpdArray.size() - 1)->func->Return->type_name; // 현재 작동중인 함수의 리턴타입
		CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();

		for (int i = 0; i < IC->typeList.size(); i++) {
			string tname = IC->typeList.at(i)->type_name;
			if (rtype == tname) {
				//v = ValuePtr::AddValue(tname, "", 0, IC);
				v = GetDataOperation(e, IC, tname, cpd);
				break;
			}
		}
		//((Process*)IC->process)->callArray.pop_back();
		return v;
	}
	else {
		ValuePtr* vp = ValuePtr::AddValue("int", "", 0, IC);
		return vp;
	}
}

//int GetDataLocationCore(textblock name, CodePlayData* cpd) {
//	for (int i = 0; i < (int)cpd->naming.size(); i++) {
//		if (name == cpd->naming.at(i).name) {
//			return i;
//		}
//	}
//	return 0;
//}
//int GetDataLocation(textblock name, InsideCode* IC) {
//	int n = GetDataLocationCore(name, ((Process*)IC->process)->PresentPlayData());
//	if (n == -1) {
//		for (int i = 0; i < (int)IC->naming.size(); i++) {
//			if (name == IC->naming.at(i).name) {
//				return i;
//			}
//		}
//	}
//	else {
//		return n;
//	}
//}

int GetDataLocation(textblock name, CodePlayData* cpd, InsideCode* IC)
{
	if (name.at(0) == '{') {
		return IC->mem->StringToMemory(name, IC);
	}

	int last = cpd->local_ndl.size() - 1;
	for (int i = last; i >= 0; i--) {
		NamingDataList* nameD = cpd->local_ndl.at(i);
		for (int k = 0; k < nameD->naming.size(); k++) {
			if (name == nameD->naming.at(k).name) {
				return nameD->naming.at(k).dataid;
			}
		}
	}

	for (int i = 0; i < IC->naming.size(); i++) {
		if (IC->naming.at(i).name == name) {
			return IC->naming.at(i).dataid;
		}
	}

	return -1;
}

int GetNamingLocation(textblock name, vector<NamingData> nd)
{
	for (int i = 0; i < nd.size(); i++) {
		if (nd.at(i).name == name) {
			return nd.at(i).dataid;
		}
	}
	return -1;
}

string DecodeTextBlock(textblock t) {
	//이 함수는 택스트 블록이 어떤 종류인지를 반환해주는 함수다.
	//그 종류는 다음과 같다.
	//숫자(값), 네임(변수, 함수, 구조체), 연산자.

	//먼저, char하나하나마다 1은 숫자, 2는 문자와 언더바, 3은 특수문자일경우 나타낸다.
	//이 표시하는 숫자들을 배열하고나서 최종적으로 이 택스트블록이 어떤 종류인지를 반환한다.
	if (t == "true" || t == "false") {
		return "num";
	}

	if (t.size() == 3 && (t.at(0) == '\'' && t.at(2) == '\'')) {
		return "num";
	}

	if (t.size() >= 3 && (t.at(0) == '\"' && t.at(t.size() - 1) == '\"')) {
		return "num";
	}

	int* intarray = new int[t.size()];
	bool dot = false;
	bool num = true;
	
	for (int i = 0; i < (int)t.size(); i++) {
		char c = t.at(i);
		
		if ('0' <= c && c <= '9') {
			intarray[i] = 1;
			continue;
		}

		if ((('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) || c == '_') {
			intarray[i] = 2;
			num = false;
			continue;
		}

		if (i == 0 && c == '-') {
			intarray[i] = 1;
			continue;
		}

		if ((num && dot == false) && c == '.') {
			intarray[i] = 1;
			dot = true;
			continue;
		}

		intarray[i] = 3;
		num = false;
	}

	bool bnum = true;
	bool bstr = true;
	bool boper = true;
	bool firstnum = false;
	if (intarray[0] == 1) {
		firstnum = true;
	}
	for (int i = 0; i < (int)t.size(); i++) {
		if (intarray[i] != 1) bnum = false;

		if (firstnum) bstr = false;

		if (intarray[i] != 3) boper = false;

		if (intarray[i] == 3) bstr = false;
	}

	delete[] intarray;

	if (bnum) return "num";
	if (bstr) return "str";
	if (boper) return "operator";
	else {
		if (t.at(0) == '{') {
			return "memory";
		}

		return "null";
	}
	
}

ValuePtr* GetDataOperation(vector<textblock*> expression, InsideCode* IC, string returnType, CodePlayData* cpd) {
	//배열상수처리
	if (expression.size() != 0 && (*expression.at(0) == "{" && *expression.at(expression.size() - 1) == "}")) {
		int arraySiz = 0;

		for (int i = 1; i < expression.size(); i++) {
			if (*expression.at(i) == "," || *expression.at(i) == "}") {
				arraySiz++;
			}
		}

		ValuePtr* ArrayV0 = ValuePtr::AddValue("array", "", arraySiz, IC);
		((Array*)ArrayV0)->siz = arraySiz;
		int startI = 1;
		int endI = 1;
		for (int i = 1; i < expression.size(); i++) {
			bool addV = false;
			if (*expression.at(i) == "," || *expression.at(i) == "}") {
				endI = i;
				addV = true;
			}

			if (addV) {
				vector<textblock*> SubExpression;
				for (int k = startI; k < endI; k++) {
					SubExpression.push_back(expression.at(k));
				}

				ValuePtr* V0 = GetDataOperation(SubExpression, IC, "none", cpd);
				((Array*)ArrayV0)->values.push_back(V0);

				startI = endI+1;
			}
		}

		return ArrayV0;
	}
	
	//함수 처리
	int i = 0;
	while (i < expression.size()-2) {
		if (expression.size() >= 3) {
			//첫번째블록으로 함수가 나올경우..
			if (DecodeTextBlock(*expression.at(i)) == "str" && *expression.at(i+1) == "(") {
				int inout_count = 0;
				for (int k = i+1; k < (int)expression.size(); k++) {
					string s = *expression.at(k);
					if (s == ")") {
						inout_count--;
						if (inout_count == 0) {
							CodeBlock* cb = new CodeBlock();
							vector<textblock*> subExpress;
							for (int l = i; l <= k; l++) {
								subExpress.push_back(expression.at(l));
							}
							cb->blockdata = subExpress;
							//함수를 실행한다.
							ValuePtr* v = Excute_Function(cb, IC);
							delete cb;
							//문자로 치환해 expression에 넣는다.
							for (int u = i; u <= k; u++) {
								expression.erase(expression.begin() + i);
							}
							textblock* t = v->ValueToString();
							expression.insert(expression.begin() + i, t);

							break;
							//return v;
						}
					}

					if (s == "(") {
						inout_count++;
					}
				}
			}
		}
		else {
			i++;
			break;
		}
		i++;
	}
	
	//이건 변수 하나만 expression에 있을때의 대처 같은데 이상함. GetDataFrontOperation이 memory를 반환함.(내가 왜 이걸 썼지...)
	ValuePtr* variableCheck = GetDataFrontOperation(expression, IC, cpd);
	if (variableCheck != nullptr && variableCheck->type_name == returnType) {
		return variableCheck;
	}
	if (variableCheck != nullptr) {
		delete variableCheck;
	}
	

	//Normal Operator Processing // 나중에 속도빠르게 해야 할듯.
	ValuePtr* result = 0;
	for (int u = 0; u < (int)IC->operatorList.size(); u++) {
		string oper = IC->operatorList.at(u)->form;
		for (int i = 0; i < (int)expression.size(); i++) {
			const char* operstr = expression.at(i)->c_str();
			if (strcmp(operstr, oper.c_str()) == 0) {
				if ((1 <= i && i < (int)expression.size() - 1) == false) return 0;
				Operator *o = IC->operatorList.at(u);
				ValuePtr* num1 = nullptr;
				ValuePtr* num2 = nullptr;
				ValuePtr* r = ValuePtr::AddValue(returnType, "", 0, IC);

				int startI, endI;
				string typestr_num1 = DecodeTextBlock(*expression.at(i - 1));
				string typestr_num2 = DecodeTextBlock(*expression.at(i + 1));
				if (typestr_num1 != "operator") {
					num1 = (ValuePtr*)getvaluewithString(*expression.at(i - 1), IC, o->paramType[0]);
					startI = i - 1;
				}
				else {
					//()에 대한 처리
					int inout_count = 0;
					for (int k = i - 1; k >= 0; k--) {
						string s = *expression.at(k);
						if (s == ")") {
							inout_count++;
						}

						if (s == "(") {
							inout_count--;
							if (inout_count == 0) {
								if (DecodeTextBlock(*expression.at(k - 1)) == "str") {
									CodeBlock* cb = new CodeBlock();
									vector<textblock*> subExpress;
									for (int l = k - 1; l < i; l++) {
										subExpress.push_back(expression.at(l));
									}
									cb->blockdata = subExpress;
									num1 = Excute_Function(cb, IC);
									delete cb;
									startI = k;
									break;
								}
								vector<textblock*> subExpress;
								for (int l = k; l < i; l++) {
									subExpress.push_back(expression.at(l));
								}
								subExpress.pop_back(); subExpress.erase(subExpress.begin());
								num1 = GetDataOperation(subExpress, IC, returnType, cpd);
								startI = k;
								break;
							}
						}
					}
				}

				if (typestr_num2 != "operator") {
					if (expression.size() > i + 2 && (typestr_num2 == "str" && *expression.at(i + 2) == "(")) {
						//함수일때 작동
						int inout_count = 0;
						for (int k = i + 1; k < (int)expression.size(); k++) {
							string s = *expression.at(k);
							if (s == ")") {
								inout_count--;
								if (inout_count == 0) {
									CodeBlock* cb = new CodeBlock();
									vector<textblock*> subExpress;
									for (int l = i + 1; l <= k; l++) {
										subExpress.push_back(expression.at(l));
									}
									cb->blockdata = subExpress;
									num2 = Excute_Function(cb, IC);
									delete cb;
									endI = k;
									break;
								}
							}

							if (s == "(") {
								inout_count++;
							}
						}
					}
					else {
						num2 = (ValuePtr*)getvaluewithString(*expression.at(i + 1), IC, o->paramType[1]);//요주
						//num2->getValueToString()
						endI = i + 1;
					}
				}
				else {
					//()에 대한 처리
					int inout_count = 0;
					for (int k = i + 1; k < (int)expression.size(); k++) {
						string s = *expression.at(k);
						if (s == ")") {
							inout_count--;
							if (inout_count == 0) {
								vector<textblock*> subExpress;
								for (int l = i + 1; l <= k; l++) {
									subExpress.push_back(expression.at(l));
								}
								subExpress.pop_back(); subExpress.erase(subExpress.begin());
								num2 = GetDataOperation(subExpress, IC, returnType, cpd);
								endI = k;
								break;
							}
						}

						if (s == "(") {
							inout_count++;

						}
					}
				}

				if (num2 == 0) return 0;
				//ValuePtr* res;
				r = operTwoValue(num1, oper, num2, IC);
				for (int k = startI; k <= endI; k++) {
					expression.erase(expression.begin() + startI);
				}
				textblock* t = r->ValueToString();
				expression.insert(expression.begin() + startI, t);

				delete num1;
				delete num2;
			}
		}
	}

	//return result
	if (expression.size() == 1) {
		string ex = *expression.at(0);
		string de = DecodeTextBlock(ex);
		if (de == "num" || (ex == "true" || ex == "false")) {
			//상수를 입력받으면 타입스트링을 내뱉는 함수 (any일때 사용할 것.)
			if (returnType == "none") {
				returnType = "any";
			}

			if (returnType == "any") {
				string typen = ValuePtr::GetTypeNameFromValueString(ex);
				ValuePtr* v = ValuePtr::AddValue(typen, "", 0, IC);

				v->getValueToString(ex);
				result = v;
				return result;
			}
			else {
				ValuePtr* v = ValuePtr::AddValue(returnType, "", 0, IC);

				v->getValueToString(ex);
				result = v;
				return result;
			}
			
		}
		else if (de == "str" || de == "null") {
			int mem = GetDataLocation(ex, cpd, IC);
			ValuePtr* v = IC->mem->memory[mem];
			if (returnType == "none") {
				ValuePtr* rv = Casting("any", v, IC);
				return rv;
			}
			else {
				ValuePtr* rv = Casting(returnType, v, IC);
				return rv;
			}
		}
		else if (de == "memory") {
			int n = IC->mem->StringToMemory(ex, IC);
			ValuePtr* v = IC->mem->memory[n];
			return v;
		}
	}
	else if (expression.size() >= 3){
		// when only one function remain
		if (DecodeTextBlock(*expression.at(0)) == "str" && *expression.at(1) == "(") {
			int inout_count = 0;
			for (int k = 0; k < (int)expression.size(); k++) {
				string s = *expression.at(k);
				if (s == ")") {
					inout_count--;
					if (inout_count == 0) {
						CodeBlock* cb = new CodeBlock();
						vector<textblock*> subExpress;
						for (int l = 0; l <= k; l++) {
							subExpress.push_back(expression.at(l));
						}
						cb->blockdata = subExpress;
						ValuePtr* v = Excute_Function(cb, IC);
						delete cb;
						return v;
					}
				}

				if (s == "(") {
					inout_count++;
				}
			}
		}
		return nullptr;
	}
}

ValuePtr* GetDataFrontOperation(vector<textblock*>& expression, InsideCode* IC, CodePlayData* cpd) {
	//Front Operator Processing
	
	//수식변환 부분
	for (int i = 0; i < IC->frontOperatorList.size(); i++) {
		expression = IC->frontOperatorList.at(i)->Func(expression, (Ptr*)IC);
	}

	//변수반환부분
	if (expression.size() == 1) {
		if (DecodeTextBlock(*expression.at(0)) != "num") {
			//expression이 name이거나 operator형태일때
			int lo = GetDataLocation(*expression.at(0), cpd, IC);
			if (lo == -1) {
				PrintCodeLocation(cpd);
				printf("[ERROR] : 변수 %s 가 정의되지 않았습니다.\n", expression.at(0)->c_str());
				return nullptr;
			}
			MemoryValue* ins = (MemoryValue*)ValuePtr::AddValue("memory", "", 0, IC);
			ins->value = lo;
			return (ValuePtr*)ins;
		}
		else {
			return nullptr;
		}
	}
	else {
		return nullptr;
	}
}

ValuePtr* getvaluewithString(string str, InsideCode* IC, string returnType)
{
	ValuePtr* result = nullptr;
	CodePlayData* cpd = ((Process*)IC->process)->PresentPlayData();
	string typestr = DecodeTextBlock(str);
	if (returnType != "any") {
		if (typestr == "num") {
			//자료형에 따라 달라야 함.
			//그려려면, 먼저 자료형별 이 텍스트가 이 자료형인지 확인하는 함수가 있어야 함
			ValuePtr* vptr = ValuePtr::AddValue(returnType, "", 0, IC);
			if (vptr->isThisType(str) == true) {
				vptr->getValueToString(str);
				result = vptr;
			}
			//std::string istr = str;
			//size_t siz = istr.size();
			//int value = stoi(istr, &siz, 10);
			//result = (ValuePtr*)(new Int());
			//((Int*)result)->value = value;
		}
		else if (typestr == "operator") {
			return 0;
		}
		else if (typestr == "str") {
			int lo = GetDataLocation(str, cpd, IC);
			string type_string = IC->mem->memory[lo]->type_name;
			ValuePtr* value = 0;
			value = IC->mem->memory[lo];
			result = value;
		}
		else if (typestr == "memory") {
			int lo = MemoryValue::GetLocToString(str);
			ValuePtr* value = 0;
			value = IC->mem->memory[lo];
			result = value;
		}
	}
	else {
		if (typestr == "num") {
			//자료형에 따라 달라야 함.
			//그려려면, 먼저 자료형별 이 텍스트가 이 자료형인지 확인하는 함수가 있어야 함
			
			for (int i = 0; i < IC->typeList.size(); i++) {
				ValuePtr* vptr = ValuePtr::AddValue(IC->typeList.at(i)->type_name, "", 0, IC);
				if (vptr->isThisType(str) == true) {
					vptr->getValueToString(str);
					result = vptr;
					break;
				}
			}
			//std::string istr = str;
			//size_t siz = istr.size();
			//int value = stoi(istr, &siz, 10);
			//result = (ValuePtr*)(new Int());
			//((Int*)result)->value = value;
		}
		else if (typestr == "operator") {
			return 0;
		}
		else if (typestr == "str") {
			int lo = GetDataLocation(str, cpd, IC);
			string type_string = IC->mem->memory[lo]->type_name;
			ValuePtr* value = 0;
			value = IC->mem->memory[lo];
			result = value;
		}
		else if (typestr == "memory") {
			int lo = MemoryValue::GetLocToString(str);
			ValuePtr* value = 0;
			value = IC->mem->memory[lo];
			result = value;
		}
	}
	return result;
}
