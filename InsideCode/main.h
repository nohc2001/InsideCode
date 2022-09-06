#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <Windows.h>
using namespace std;

bool isClassPtrDebug = true;
map<string, int> classPtrMap;
map<string, bool> DebugObjMap;

typedef std::string textblock;

bool bSameStr(string a, char* b) {
	for (int i = 0; i < (int)a.size(); i++) {
		if (a.at(i) != b[i]) {
			return false;
		}
	}

	return true;
}

textblock* tbClone(textblock* tb) {
	textblock* rtb = new textblock();
	for (int i = 0; i < (int)tb->size(); i++) {
		rtb->push_back(tb->at(i));
	}
	return rtb;
}

bool bCanBeName(string a) {
	for (int i = 0; i < (int)a.size(); i++) {
		char c = a.at(i);
		if (!((65 <= c && c <= 90) || (97 <= c && c <= 122))) {
			if (i == 0 && (48 <= c && c <= 57)) {
				return false;
			}
			else if (i != 0 && (48 <= c && c <= 57)) {
				continue;
			}
			else return false;
		}
	}
	return true;
}

bool bCanBeTextblock(string a) {
	/*
	�ڵ尡 �ؽ�Ʈ ����� ������ ����.
	1. �����̽� �� �� ������ ����.
	2. ;�� ������ �����, ;���� ������
	3. =�� ���� �����ڰ� ������ =���� ������
	4. (, )��, {, }�� ������ ���� ������.
	5. "�� ù��°�� ������ ������������ ������ �ʰ�, �� ���Ŀ� "�� ������ ������ ����. ������, ù��°�� "�� ������ �ʾ��� ��� ���Ŀ� ������ "�� ��� �����.
	*/

	bool num = false;
	bool oper = false;
	if (a.at(0) == '\"') {
		if (a.size() > 1 && a.at(a.size() - 1) == '\"') {
			return true;
		}
		else return false;
	}

	for (int i = 0; i < (int)a.size(); i++) {
		char c = a.at(i);
		bool b1 = 33 <= c && c <= 126;
		if (b1 == false) {
			return false;
		}

		bool n1 = (('0' <= c && c <= '9') || ('a' <= c && c <= 'z')) || ('A' <= c && c <= 'Z');
		if (n1) {
			if (oper == true) {
				return false;
			}
			num = true;
		}
		else {
			if (num == true) {
				return false;
			}
			if (oper == true) {
				return false;
			}
			oper = true;
		}
	}
	return true;
}

struct Ptr {
};

//���� �ڷ����� �߻�Ŭ����
class ValuePtr {
public:
	ValuePtr();
	static ValuePtr* AddValue(string Typename, string SubTypename, int arraySiz, Ptr* IC);
	virtual ~ValuePtr();

	char type_name; // �⺻ �ڷ����� ��� �� �̸��� ������, ����ü��� �� ������ "struct"��� ���� ����, ����ü�� �̸��� �ش� Struct ������Ʈ�� structType_Name���� ������ �� �ִ�.
	string GetTypeName();

	static bool CopyValue(ValuePtr* v1, ValuePtr* v2, Ptr* IC);
	static string GetTypeNameFromValueString(string constant_value);
	static string GetTypeStringFromChar(char tc);
	static char GetTypeCharFromString(string str);

	virtual void getValueToString(string str);
	virtual bool isThisType(string str);
	virtual textblock* ValueToString();
};

//������
class Operator : Ptr {
public:
	string form; // �������� ����
	string name; // �������� �̸�
	string paramType[2]; // �ǿ������� Ÿ��
	string returnType; // ��ȯŸ��
	ValuePtr* (*Func)(ValuePtr*, ValuePtr*, Ptr*);
	Operator();
	Operator(string Name, string Form, string ParamType1, string ParamType2, string ReturnType, ValuePtr* (*func)(ValuePtr*, ValuePtr*, Ptr*));
	virtual ~Operator();
};

//���� ������.
ValuePtr* IntPercent(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // %
ValuePtr* Mul(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // *
ValuePtr* Div(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // /
ValuePtr* Plus(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // +
ValuePtr* Minus(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // -

//�� ������
ValuePtr* Same(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // ==
ValuePtr* RBig(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // <
ValuePtr* LBig(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // >
ValuePtr* RBigSame(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // <=
ValuePtr* LBigSame(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // >=
ValuePtr* NotSame(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // !=

//�� ������.
ValuePtr* And(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // &&
ValuePtr* Or(ValuePtr* v1, ValuePtr* v2, Ptr* IC); // ||

//��������
class FrontOperator : Ptr{
public:
	string form; // �������� ����
	string name; // �������� �̸�
	vector<textblock*>(*Func)(vector<textblock*>, Ptr*); // �����Լ�

	FrontOperator() { 
		form = "none"; name = "noname"; Func = nullptr; 

		if (isClassPtrDebug && DebugObjMap["FrontOperator"]) {
			++classPtrMap["FrontOperator"];
			std::printf("\n new FrontOperator :\t %p \t| FrontOperator stack : %d", this, classPtrMap["FrontOperator"]);
		}
	}
	FrontOperator(string Name, string Form, vector<textblock*>(*func)(vector<textblock*>, Ptr*))
	{
		name = Name;
		form = Form;
		Func = func;

		if (isClassPtrDebug && DebugObjMap["FrontOperator"]) {
			++classPtrMap["FrontOperator"];
			std::printf("\n new FrontOperator :\t %p \t| FrontOperator stack : %d", this, classPtrMap["FrontOperator"]);
		}
	}
	virtual ~FrontOperator() {
		if (isClassPtrDebug && DebugObjMap["FrontOperator"]) {
			--classPtrMap["FrontOperator"];
			printf("\n new FrontOperator :\t %s \t| FrontOperator stack : %d", name.c_str(), classPtrMap["FrontOperator"]);
		}

		if (form.size() > 0) {
			form.clear();
		}
		
		if (name.size() > 0) {
			name.clear();
		}
	}
};

vector<textblock*> INEQ(vector<textblock*> expression, Ptr* ICptr); // ()
vector<textblock*> SeekIndex(vector<textblock*> expression, Ptr* ICptr);
vector<textblock*> StructDots(vector<textblock*> expression, Ptr* ICptr); // ptr.x || ptr->x
vector<textblock*> GetAddress(vector<textblock*> expression, Ptr* ICptr); // &ptr
vector<textblock*> GetVariableFromPointer(vector<textblock*> expression, Ptr* ICptr); //*ptr

//ĳ����
ValuePtr* Casting(string type, ValuePtr* value, Ptr* IC);

//�⺻ �ڷ���
class Int : ValuePtr {
public:
	int value;
	static string Type_name;

	Int()
	{
		value = 0;
		type_name = 'i';

		if (isClassPtrDebug && DebugObjMap["Int"]) {
			++classPtrMap["Int"];
			std::printf("\n new Int :\t %p \t| Int stack : %d", this, classPtrMap["Int"]);
		}
	}
	Int(const Int& ref) {
		value = ref.value;
		Type_name = ref.Type_name;
		type_name = ref.type_name;

		if (isClassPtrDebug && DebugObjMap["Int"]) {
			++classPtrMap["Int"];
			printf("\n new Int :\t %p \t| Int stack : %d", this, classPtrMap["Int"]);
		}
	}
	virtual ~Int() {
		if (isClassPtrDebug && DebugObjMap["Int"]) {
			--classPtrMap["Int"];
			printf("\n delete Int :\t %p \t| Int stack : %d", this, classPtrMap["Int"]);
		}
	}
	void getValueToString(string str);
	bool isThisType(string str);
	textblock* ValueToString();
};

class Float : ValuePtr {
public:
	float value;
	static string Type_name;

	Float()
	{
		value = 0.f;
		type_name = 'f';

		if (isClassPtrDebug && DebugObjMap["Float"]) {
			++classPtrMap["Float"];
			printf("\n new Float :\t %p \t| Float stack : %d", this, classPtrMap["Float"]);
		}
	}

	Float(const Float& ref) {
		value = ref.value;
		Type_name = ref.Type_name;
		type_name = ref.type_name;

		if (isClassPtrDebug && DebugObjMap["Float"]) {
			++classPtrMap["Float"];
			printf("\n new Float :\t %p \t| Float stack : %d", this, classPtrMap["Float"]);
		}
	}

	virtual ~Float() {
		if (isClassPtrDebug && DebugObjMap["Float"]) {
			--classPtrMap["Float"];
			printf("\n delete Float :\t %p \t| Float stack : %d", this, classPtrMap["Float"]);
		}
	}
	void getValueToString(string str);
	bool isThisType(string str);
	textblock* ValueToString();
};

class Bool : ValuePtr {
public:
	bool value;
	static string Type_name;

	Bool()
	{
		value = false;
		type_name = 'b';
		if (isClassPtrDebug && DebugObjMap["Bool"]) {
			++classPtrMap["Bool"];
			printf("\n new Bool :\t %p \t| Bool stack : %d", this, classPtrMap["Bool"]);
		}
	}
	Bool(const Bool& ref) {
		value = ref.value;
		Type_name = ref.Type_name;
		type_name = ref.type_name;
		if (isClassPtrDebug && DebugObjMap["Bool"]) {
			++classPtrMap["Bool"];
			printf("\n new Bool :\t %p \t| Bool stack : %d", this, classPtrMap["Bool"]);
		}
	}
	virtual ~Bool() {
		if (isClassPtrDebug && DebugObjMap["Bool"]) {
			--classPtrMap["Bool"];
			printf("\n delete Bool :\t %p \t| Bool stack : %d", this, classPtrMap["Bool"]);
		}
	}
	void getValueToString(string str);
	bool isThisType(string str);
	textblock* ValueToString();
};

class Char : ValuePtr {
public:
	char value;
	static string Type_name;

	Char() {
		value = 0;
		type_name = 'c';
		if (isClassPtrDebug && DebugObjMap["Char"]) {
			++classPtrMap["Char"];
			printf("\n new Char :\t %p \t| Char stack : %d", this, classPtrMap["Char"]);
		}
	}
	Char(const Char& ref) {
		value = ref.value;
		Type_name = ref.Type_name;
		type_name = ref.type_name;
		if (isClassPtrDebug && DebugObjMap["Char"]) {
			++classPtrMap["Char"];
			printf("\n new Char :\t %p \t| Char stack : %d", this, classPtrMap["Char"]);
		}
	}
	virtual ~Char() {
		if (isClassPtrDebug && DebugObjMap["Char"]) {
			--classPtrMap["Char"];
			printf("\n delete Char :\t %p \t| Char stack : %d", this, classPtrMap["Char"]);
		}
	}
	void getValueToString(string str);
	bool isThisType(string str);
	textblock* ValueToString();
};

//�ڵ�
enum class codeKind {
	ck_addVariable, // ��������
	ck_setVariable, // ���� �� ����
	ck_get, // ȣ��, �Լ�ȣ�� ����
	ck_if, // ���ǹ�
	ck_while, // �ݺ���
	ck_blocks, // {} �߰�ȣ
	ck_addFunction, // �Լ�����
	ck_useFunction, // �Լ�ȣ��
	ck_returnInFunction, // �Լ� ������ ���� �ڵ�
	ck_addStruct, // ����ü ����
	ck_break, // break
	ck_continue // continue
};

struct NamingData {
	textblock name;
	int dataid; // InsideCode�� data�� ���° ��������.
	string type_name;
};

class MemoryValue : ValuePtr {
public:
	int value;
	static string Type_name;

	MemoryValue() {
		value = 0;
		type_name = 'm';
		if (isClassPtrDebug && DebugObjMap["MemoryValue"]) {
			++classPtrMap["MemoryValue"];
			printf("\n new MemoryValue :\t %p \t| MemoryValue stack : %d", this, classPtrMap["MemoryValue"]);
		}
	}
	MemoryValue(const MemoryValue& ref) {
		value = ref.value;
		Type_name = ref.Type_name;
		type_name = ref.type_name;
		if (isClassPtrDebug && DebugObjMap["MemoryValue"]) {
			++classPtrMap["MemoryValue"];
			printf("\n new MemoryValue :\t %p \t| MemoryValue stack : %d", this, classPtrMap["MemoryValue"]);
		}
	}
	virtual ~MemoryValue() {
		if (isClassPtrDebug && DebugObjMap["MemoryValue"]) {
			--classPtrMap["MemoryValue"];
			printf("\n delete MemoryValue :\t %p \t| MemoryValue stack : %d", this, classPtrMap["MemoryValue"]);
		}
	}
	void getValueToString(string str) {
		if (str.at(0) == '{') {
			str.pop_back();
			string a;
			for (unsigned int i = 5; i < str.size(); i++) {
				a.push_back(str.at(i));
			}

			value = atoi(a.c_str());
		}
	}

	bool isThisType(string str) {
		if (str.find_first_of("{loc:") && str.at(str.size() - 1) == '}') {
			if (str.size() >= 7)
				return true;
		}

		return false;
	}

	textblock* ValueToString() {
		textblock* tt =  new string("{loc:");
		char num[128];
		_itoa_s(value, num, 10);
		for (unsigned int i = 0; i < strlen(num); i++) {
			tt->push_back(num[i]);
		}
		tt->push_back('}');
		return tt;
	}
	static int GetLocToString(string str) {
		if (str.at(0) == '{') {
			str.pop_back();
			string a;
			for (unsigned int i = 5; i < str.size(); i++) {
				a.push_back(str.at(i));
			}

			int vv = atoi(a.c_str());
			return vv;
		}
		return 0;
	}
};

class Struct : ValuePtr {
public:
	vector<ValuePtr*> values;
	vector<NamingData> memberVnaming;
	string structType_Name;

	Struct() { 
		type_name = 's';
		if (isClassPtrDebug && DebugObjMap["Struct"]) {
			++classPtrMap["Struct"];
			printf("\n new Struct :\t %p \t| Struct stack : %d", this, classPtrMap["Struct"]);
		}
	}
	Struct(const Struct& ref, Ptr* IC) {
		type_name = 's';
		for (unsigned int i = 0; i < ref.memberVnaming.size(); i++) {
			values.push_back(ValuePtr::AddValue(ref.memberVnaming.at(i).type_name, "", 0, IC));
			NamingData nd;
			nd.dataid = i;
			nd.type_name = ref.memberVnaming.at(i).type_name;
			nd.name = ref.memberVnaming.at(i).name;
			memberVnaming.push_back(nd);
			ValuePtr::CopyValue(values.at(i), ref.values.at(i), IC);
		}

		if (isClassPtrDebug && DebugObjMap["Struct"]) {
			++classPtrMap["Struct"];
			printf("\n new Struct :\t %p \t| Struct stack : %d", this, classPtrMap["Struct"]);
		}
	}
	virtual ~Struct() {
		if (isClassPtrDebug && DebugObjMap["Struct"]) {
			--classPtrMap["Struct"];
			printf("\n delete Struct :\t %p \t| Struct stack : %d", this, classPtrMap["Struct"]);
		}

		for (unsigned int i = 0; i < values.size(); ++i) {
			if (values[i] != nullptr) {
				delete values[i];
				values[i] = nullptr;
			}
		}

		if (memberVnaming.size() > 0) {
			memberVnaming.clear();
		}

		if (structType_Name.size() > 0) {
			structType_Name.clear();
		}
	}

	textblock* ValueToString();
};

class Pointer : ValuePtr {
public:
	string preTypeName; // TypeName�� �̰Ͱ� ���� Value�� �� �����Ϳ� �� �� ����.
	string TypeName; // �̰��� � ���� preTypeName�� ������ �� ���� �� �� ������ �ǹ� ��, ���� �� ���� �ڷ����� ����.
	int address; //����Ű�� �ּҰ�..

	Pointer() { 
		type_name = 'p';
		if (isClassPtrDebug && DebugObjMap["Pointer"]) {
			++classPtrMap["Pointer"];
			printf("\n new Pointer :\t %p \t| Pointer stack : %d", this, classPtrMap["Pointer"]);
		}
	}
	Pointer(const Pointer& ref, Ptr* IC) {
		type_name = 'p';
		preTypeName = ref.preTypeName;
		TypeName = ref.TypeName;
		address = ref.address;

		if (isClassPtrDebug && DebugObjMap["Pointer"]) {
			++classPtrMap["Pointer"];
			printf("\n new Pointer :\t %p \t| Pointer stack : %d", this, classPtrMap["Pointer"]);
		}
	}
	virtual ~Pointer() {
		if (isClassPtrDebug && DebugObjMap["Pointer"]) {
			--classPtrMap["Pointer"];
			printf("\n delete Pointer :\t %p \t| Pointer stack : %d", this, classPtrMap["Pointer"]);
		}

		if (preTypeName.size() > 0) {
			preTypeName.clear();
		}

		if (TypeName.size() > 0) {
			TypeName.clear();
		}
	}
	textblock* ValueToString();
	void getValueToString(string str);
	static textblock* PointerToString(int address);
};

class Array : ValuePtr {
public:
	Pointer p;
	int siz;
	vector<ValuePtr*> values;

	Array() { 
		type_name = 'a';
		p = Pointer();
		p.address = -1;
		if (isClassPtrDebug && DebugObjMap["Array"]) {
			++classPtrMap["Array"];
			std::printf("\n new Array :\t %p \t| Array stack : %d", this, classPtrMap["Array"]);
		}
	}
	Array(const Array& ref, Ptr* IC) {
		type_name = 'a';
		p = Pointer(ref.p);
		siz = ref.siz;
		p.address = -1;
		if (isClassPtrDebug && DebugObjMap["Array"]) {
			++classPtrMap["Array"];
			std::printf("\n new Array :\t %p \t| Array stack : %d", this, classPtrMap["Array"]);
		}
	}
	virtual ~Array() {
		//array�� �����ϱ� ���� ArrayInMemory�Լ��� �ݵ�� ȣ���ؾ� �Ѵ�.
		if (isClassPtrDebug && DebugObjMap["Array"]) {
			--classPtrMap["Array"];
			std::printf("\n delete Array :\t %p \t| Array stack : %d", this, classPtrMap["Array"]);
		}

		if (p.address <= 0) {
			for (unsigned int i = 0; i < values.size(); ++i) {
				if (values[i] != nullptr) {
					delete values[i];
					values[i] = nullptr;
				}
			}
		}
		else {
			for (unsigned int i = 0; i < values.size(); ++i) {
				if (values[i] != nullptr) {
					//delete values[i];
					values[i] = nullptr;
				}
			}
		}

		if (values.size() > 0) {
			values.clear();
		}
	}

	void getValueToString(string str);
	void ArrayInMemory(Ptr* ic);
	//static textblock* PointerToString(int address);
	textblock* ValueToString();
};

struct CodeBlock : Ptr {
	codeKind ck;
	vector<textblock*> blockdata; // �׻� heap

	vector<Ptr*> CodeBlocks; // {} �߰�ȣ�Ͻÿ��� �̰��� ��.
	vector<NamingData> naming; //{} �߰�ȣ�Ͻÿ��� �̰��� ��. �������� ������ �׷��� ��.

	CodeBlock() {
		if (isClassPtrDebug && DebugObjMap["CodeBlock"]) {
			++classPtrMap["CodeBlock"];
			printf("\n new CodeBlock :\t %p \t| CodeBlock stack : %d", this, classPtrMap["CodeBlock"]);
		}
	}

	virtual ~CodeBlock() {
		if (isClassPtrDebug && DebugObjMap["CodeBlock"]) {
			--classPtrMap["CodeBlock"];
			printf("\n delete CodeBlock :\t %p \t| CodeBlock stack : %d", this, classPtrMap["CodeBlock"]);
		}

		/*for (int i = 0; i < blockdata.size(); ++i) {
			if (blockdata[i] != nullptr) {
				delete blockdata[i];
				blockdata[i] = nullptr;
			}
		}
		blockdata.clear();*/

		for (unsigned int i = 0; i < CodeBlocks.size(); ++i) {
			CodeBlock* c = (CodeBlock*)CodeBlocks[i];
			if (c != nullptr) {
				delete c;
			}
		}
		CodeBlocks.clear();

		naming.clear();
	}
};

struct FuncParam {
	ValuePtr* valueType = nullptr; // �׻� heap
	string name;

	FuncParam() {
		if (isClassPtrDebug && DebugObjMap["FuncParam"]) {
			++classPtrMap["FuncParam"];
			printf("\n new FuncParam :\t %p \t| FuncParam stack : %d", this, classPtrMap["FuncParam"]);
		}
	}

	virtual ~FuncParam() {
		if (isClassPtrDebug && DebugObjMap["FuncParam"]) {
			--classPtrMap["FuncParam"];
			printf("\n delete FuncParam :\t %p \t| FuncParam stack : %d", this, classPtrMap["FuncParam"]);
		}

		if (valueType != nullptr) {
			delete valueType;
		}

		if (name.size() > 0) {
			name.clear();
		}
	}
};

struct Function {
	string name;
	vector<FuncParam> Parameter;
	CodeBlock* codes;
	ValuePtr* Return;

	Function() {
		if (isClassPtrDebug && DebugObjMap["Function"]) {
			++classPtrMap["Function"];
			printf("\n new Function :\t %p \t| Function stack : %d", this, classPtrMap["Function"]);
		}
	}

	virtual ~Function() {
		if (isClassPtrDebug && DebugObjMap["Function"]) {
			--classPtrMap["Function"];
			printf("\n delete Function :\t %p \t| Function stack : %d", this, classPtrMap["Function"]);
		}

		if (Parameter.size() > 0) {
			Parameter.clear();
		}
		
		//if (codes != nullptr) {
		//	delete codes;
		//}

		if (name.size() > 0) {
			name.clear();
		}

		if (Return != nullptr) {
			delete Return;
		}
	}
};

class Memory {
public:
	int dataSiz = 128;
	ValuePtr** memory;

	int globalLimit = 32; // 0 ~ 32 ���� ���������� ����
	int globalFlow = 0;

	int heapLimit = 64; // 32 ~ 64 ���� heap�� ����
	int heapFlow = 32;

	int stackLimit = 128; // 64~128 ���� �Լ����� ���������� ����
	int stackFlow = 64;

	Memory() {
		memory = new ValuePtr * [dataSiz];
		for (int i = 0; i < dataSiz; i++) {
			memory[i] = nullptr;
		}

		if (isClassPtrDebug && DebugObjMap["Memory"]) {
			++classPtrMap["Memory"];
			printf("\n new Memory :\t %p \t| Memory stack : %d", this, classPtrMap["Memory"]);
		}
	}
	Memory(int siz, int gl, int hl, int sl) {
		dataSiz = siz;
		globalLimit = gl;
		globalFlow = 0;
		heapLimit = hl;
		heapFlow = globalLimit;
		stackLimit = sl;
		stackFlow = heapLimit;

		memory = new ValuePtr * [dataSiz];
		for (int i = 0; i < dataSiz; i++) {
			memory[i] = nullptr;
		}

		if (isClassPtrDebug && DebugObjMap["Memory"]) {
			++classPtrMap["Memory"];
			printf("\n new Memory :\t %p \t| Memory stack : %d", this, classPtrMap["Memory"]);
		}
	}
	virtual ~Memory() {
		if (isClassPtrDebug && DebugObjMap["Memory"]) {
			--classPtrMap["Memory"];
			printf("\n delete Memory :\t %p \t| Memory stack : %d", this, classPtrMap["Memory"]);
		}

		for (int i = 0; i < dataSiz; i++) {
			if (memory[i] != nullptr) {
				delete memory[i];
			}
		}
		delete[] memory;
	}

	bool AddDataToMemory(string mod, ValuePtr* v, int* nptr = nullptr);
	void RemoveData(int id);
	string MemoryToString(int id);
	int StringToMemory(string str, Ptr* ICptr);
};

struct InsideCode : Ptr {
	vector<CodeBlock*> code;
	vector<NamingData> naming; // ������������ �̸�
	Memory* mem;
	vector<ValuePtr*> typeList;
	vector<FrontOperator*> frontOperatorList;
	vector<Operator*> operatorList;
	vector<textblock*> ALLblockdata;
	vector<Function*> functions;

	Ptr* process;

	InsideCode() {
		if (isClassPtrDebug && DebugObjMap["InsideCode"]) {
			++classPtrMap["InsideCode"];
			printf("\n new InsideCode :\t %p \t| InsideCode stack : %d", this, classPtrMap["InsideCode"]);
		}
	}

	virtual ~InsideCode() {
		if (isClassPtrDebug && DebugObjMap["InsideCode"]) {
			--classPtrMap["InsideCode"];
			printf("\n delete InsideCode :\t %p \t| InsideCode stack : %d", this, classPtrMap["InsideCode"]);
		}

		for (unsigned int i = 0; i < code.size(); ++i) {
			if (code[i] != nullptr) {
				delete code[i];
			}
		}
		if (code.size() > 0) {
			code.clear();
		}

		for (unsigned int i = 0; i < typeList.size(); ++i) {
			if (typeList[i] != nullptr) {
				delete typeList[i];
			}
		}
		if (typeList.size() > 0) {
			typeList.clear();
		}

		for (unsigned int i = 0; i < frontOperatorList.size(); ++i) {
			if (frontOperatorList[i] != nullptr) {
				delete frontOperatorList[i];
			}
		}
		if (frontOperatorList.size() > 0) {
			frontOperatorList.clear();
		}

		for (unsigned int i = 0; i < operatorList.size(); ++i) {
			if (operatorList[i] != nullptr) {
				delete operatorList[i];
			}
		}
		if (operatorList.size() > 0) {
			operatorList.clear();
		}

		for (unsigned int i = 0; i < ALLblockdata.size(); ++i) {
			if (ALLblockdata[i] != nullptr) {
				delete ALLblockdata[i];
			}
		}
		if (ALLblockdata.size() > 0) {
			ALLblockdata.clear();
		}

		for (unsigned int i = 0; i < functions.size(); ++i) {
			if (functions[i] != nullptr) {
				delete functions[i];
			}
		}
		if (functions.size() > 0) {
			functions.clear();
		}

		if (mem != nullptr) {
			delete mem;
		}

		if (process != nullptr) {
			delete process;
		}
	}
};
bool IsTypeString(string str, InsideCode* IC);
ValuePtr* operTwoValue(ValuePtr* v1, string oper, ValuePtr* v2, Ptr* IC);

struct NamingDataList {
	vector<NamingData> naming; // ��Ͼȿ� �ִ� ���̹� ������
};

struct CodePlayData {
	Function* func;
	vector<int> line;
	vector<NamingDataList*> local_ndl; // ��������
	vector<bool> inIF; // ���� if���� ���������� ����
	vector<bool> IfIsTrue; // ����� if���� ��� ����
	bool BreakIsPlaying = false;
	bool ContinueIsPlaying = false;

	CodePlayData() {
		func = nullptr;
		if (isClassPtrDebug && DebugObjMap["CodePlayData"]) {
			++classPtrMap["CodePlayData"];
			printf("\n new CodePlayData :\t %p \t| CodePlayData stack : %d", this, classPtrMap["CodePlayData"]);
		}
	}

	virtual ~CodePlayData() {
		if (isClassPtrDebug && DebugObjMap["CodePlayData"]) {
			--classPtrMap["CodePlayData"];
			printf("\n delete CodePlayData :\t %p \t| CodePlayData stack : %d", this, classPtrMap["CodePlayData"]);
		}
	}
};

class Process {
public:
	Process() {
		if (isClassPtrDebug && DebugObjMap["Process"]) {
			++classPtrMap["Process"];
			printf("\n new Process :\t %p \t| Process stack : %d", this, classPtrMap["Process"]);
		}
	}

	virtual ~Process() {
		if (isClassPtrDebug && DebugObjMap["Process"]) {
			--classPtrMap["Process"];
			printf("\n delete Process :\t %p \t| Process stack : %d", this, classPtrMap["Process"]);
		}

		for (int i = 0; i < callArray.size(); ++i) {
			if (callArray[i] != nullptr) {
				delete callArray[i];
			}
		}
		if (callArray.size() > 0) {
			callArray.clear();
		}
	}

	vector<CodePlayData*> callArray; // ȣ�� ����
	int globalLine;
	int startFunction;

	ValuePtr* FindMemoryUseName(string name);
	CodePlayData* PresentPlayData();
};


//�ڵ尡 ����Ǳ� ��, �Ϲ� Ĵ��Ʈ�� �λ��̵��ڵ� �������� ������ȭ�ϴ� ������ �Լ���.
//�������� �ڵ� ������ȭ�ϴ� �Լ�
void CodeDataization(InsideCode* ic, const char* filename);
//txt���Ͽ��� �ڵ幮�ڿ��� �������� �Լ�
string GetCodeTXT(const char* filename);
//�ϳ��� ���ڿ� ������ codetxt�� �ܾ�� �ɰ��� �˾ƺ� �� �ֵ��� �Ѵ�.
void AddTextBlocks(string codetxt, InsideCode* IC); 
// ������ �ý�Ʈ��ϵ��� �����Ͽ� �ڵ带 �����, �� �ڵ带 ������� �����Ͽ� ��ȯ�Ѵ�.
vector<CodeBlock*> AddCodeFromBlockData(vector<textblock*> bd, InsideCode* IC, string ScanMod);

//�⺻�ڷ������� �����Ѵ�.
void DefineBasicTypes(InsideCode* IC);
//�⺻ �����ڵ��� �����Ѵ�.
void DefineBasicOperation(InsideCode* IC);
//����� �޸𸮸� �Ҵ��Ѵ�.
void AllocateICMemory(InsideCode* IC);
//������������ ��ĵ�Ѵ�.
void ScanGlobalVariable(InsideCode* IC);
//�Լ��� ��ĵ�Ѵ�.
void ScanFunction(InsideCode* IC);
//����ü�� ��ĵ�Ѵ�.
void ScanStruct(InsideCode* IC);

//IC�� �����Ų��.
ValuePtr* RunCode(CodePlayData* cpd, InsideCode* IC);

//�ڵ� �ϳ��� �����ϴ� �Լ�
ValuePtr* StepCode(CodeBlock* code, InsideCode* IC, int *i);
// ��������
void AddVariable(CodeBlock* code, InsideCode* IC); 
// ���� �� ����
void SetVariable(CodeBlock* code, InsideCode* IC); 
// if
bool Execute_if(CodeBlock* code, InsideCode* IC, int *i); 
// while
void Execute_While(CodeBlock* code, InsideCode* IC, int* i); 
// add function
ValuePtr* Excute_Function(CodeBlock* code, InsideCode* IC);
// return in function
ValuePtr* ReturnInFunction(CodeBlock* code, InsideCode* IC);

//�ھ� �Լ� - �������� ������ ���� �Լ�
int AddVariableCore(string type_name, string subTypeName, int ArraySiz, string name, CodePlayData* cpd, InsideCode* IC);
int GetDataLocationCore(textblock name, vector<NamingData> nd);
void SetVariableCore(string type_name, string name, InsideCode* IC, CodePlayData* cpd, ValuePtr* vp);

//ǥ���Լ�
void StandardOutputFunction(CodeBlock* code, InsideCode* IC); //output()
void StandardInputFunction(CodeBlock* code, InsideCode* IC); // input()
ValuePtr* StandardSizeofFunction(CodeBlock* code, InsideCode* IC); // sizeof()

// �ڵ带 �����ϴ� �Լ��ȿ��� ����ó�� �ۿ��ϴ� �Լ���
// ������ �̸��� �Է¹޾� �� ���� �޸��� �ּ�(�迭�� ��ġ)�� ��ȯ
int GetDataLocation(textblock name, CodePlayData* cpd, InsideCode* IC);

// �ڵ� ���� �ܾ�(�ؽ�Ʈ �ڽ�)�� � ������ ���� ���ϴ��� �˷��ش�. ex> ����, �̸�, ������ ���.
string DecodeTextBlock(textblock t); 

// ���� �Է��ϸ�, �� ���� ��ȯ��. ex> 5+1 => 6��ȯ
ValuePtr* GetDataFrontOperation(vector<textblock*>& expression, InsideCode* IC, CodePlayData* cpd);
ValuePtr* GetDataOperation(vector<textblock*> expression, InsideCode* IC, string returnType, CodePlayData* cpd);

ValuePtr* getvaluewithString(string str, InsideCode* IC, string returnType);

int GetNamingLocation(textblock name, vector<NamingData> nd);

void PrintCodeLocation(CodePlayData* cpd);