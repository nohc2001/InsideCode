#include <iostream>
#include <math.h>
#include <string>
#include "infArray_fm_v0.h"
#include "sen_tr.h"
using namespace std;
using namespace freemem;
int FM_Model1::updateid = 0;

typedef unsigned char byte8;
typedef unsigned short ushort;
typedef unsigned int uint;

bool cd_eqoper(char *str)
{
	if (str[1] == '=')
		return true;
	else
		return false;
}

bool bCanBeTextblock(lcstr &a)
{
	bool num = false;
	bool oper = false;
	if (a.at(0) == '\"')
	{
		if (a.size() > 1 && a.at(a.size() - 1) == '\"')
		{
			return true;
		}
		else
			return false;
	}

	if (a.size() == 2 && (a[0] == '&' && a[1] == '&'))
	{
		return true;
	}

	if (a.size() == 2 && (a[0] == '&' && a[1] == '&'))
	{
		return true;
	}

	bool isdot = false;
	bool isn = false;
	if (a.at(0) == '-')
	{
		for (int i = 1; i < (int)a.size(); ++i)
		{
			char c = a.at(i);
			bool n1 = ('0' <= c && c <= '9');
			if (!n1)
			{
				if ((isn && isdot == false) && c == '.')
				{
					isdot = true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				isn = true;
			}
		}
		return true;
	}

	for (int i = 0; i < (int)a.size(); i++)
	{
		char c = a.at(i);
		bool b1 = 33 <= c && c <= 126;
		if (b1 == false)
		{
			return false;
		}

		bool n1 = (('0' <= c && c <= '9') || ('a' <= c && c <= 'z')) || (('A' <= c && c <= 'Z') || c == '_');
		if (n1)
		{
			if (oper == true)
			{
				return false;
			}
			num = true;
		}
		else
		{
			if (num == true)
			{
				return false;
			}
			if (oper == true)
			{
				return false;
			}
			oper = true;
		}
	}
	return true;
}

typedef enum class TBT
{
	_value_bool = 0,
	_value_integer = 1,
	_value_float = 2,
	_value_char = 3,
	_value_str = 4,
	_str = 5,
	_operation = 6,
	_memory = 7,
	_null = 8
} TBT;

TBT DecodeTextBlock(lcstr &t)
{
	if (t == "true" || t == "false")
	{
		return TBT::_value_bool;
	}

	if (t.size() == 3 && (t.at(0) == '\'' && t.at(2) == '\''))
	{
		return TBT::_value_char;
	}
	else if (t.size() == 4 && ((t.at(0) == '\'' && t.at(3) == '\'') && t.at(1) == '\\'))
	{
		return TBT::_value_char;
	}

	if (t.size() >= 3 && (t.at(0) == '\"' && t.at(t.size() - 1) == '\"'))
	{
		return TBT::_value_str;
	}

	int *intarray = new int[t.size()];
	bool dot = false;
	bool num = true;

	for (int i = 0; i < (int)t.size(); i++)
	{
		char c = t.at(i);

		if ('0' <= c && c <= '9')
		{
			intarray[i] = 1;
			continue;
		}

		if ((('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) || c == '_')
		{
			intarray[i] = 2;
			num = false;
			continue;
		}

		if (i == 0 && c == '-')
		{
			intarray[i] = 1;
			continue;
		}

		if ((num && dot == false) && c == '.')
		{
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
	if (intarray[0] == 1)
	{
		firstnum = true;
	}
	for (int i = 0; i < (int)t.size(); i++)
	{
		if (intarray[i] != 1)
			bnum = false;

		if (firstnum)
			bstr = false;

		if (intarray[i] != 3)
			boper = false;

		if (intarray[i] == 3)
			bstr = false;
	}

	delete[] intarray;

	if (bnum)
	{
		if (dot)
		{
			return TBT::_value_float;
		}
		else
		{
			return TBT::_value_integer;
		}
	}
	if (bstr)
		return TBT::_str;
	if (boper)
		return TBT::_operation;
	else
	{
		if (t.at(0) == '{')
		{
			return TBT::_memory;
		}
		return TBT::_null;
	}
}

lcstr *GetCodeTXT(const char *filename, FM_System0 *fm)
{
	FILE *fp = fopen(filename, "rt");
	if (fp)
	{
		lcstr *codetxt = (lcstr *)fm->_New(sizeof(lcstr), true);
		codetxt->Init(10, false);
		int max = 0;
		fseek(fp, 0, SEEK_END);
		max = ftell(fp);
		fclose(fp);

		int stack = 0;
		fp = fopen(filename, "rt");
		int k = 0;
		while (k < max)
		{
			char c;
			c = fgetc(fp);
			if (c == '/')
			{
				stack += 1;
			}
			else
			{
				stack = 0;
			}

			if (stack == 2)
			{
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
			codetxt->push_back(c);
			k++;
		}
		return codetxt;
	}
	else
	{
		printf("[ERROR] : %s file is nor exist.", filename);
		return nullptr;
	}
}

enum class codeKind
{
	ck_addVariable,
	ck_setVariable,
	ck_get,
	ck_if,
	ck_while,
	ck_blocks,
	ck_addFunction,
	ck_useFunction,
	ck_returnInFunction,
	ck_addStruct,
	ck_break,
	ck_continue,
	ck_addsetVariable
};

struct code_sen
{
	char **sen;
	int maxlen;
	codeKind ck;
	vecarr<int *> *codeblocks = nullptr;
	FM_System0 *fm;

	int start_line = 0;
	int end_line = 0;
};

void dbg_codesen(code_sen *cs)
{
	switch (cs->ck)
	{
	case codeKind::ck_addVariable:
		cout << "add var : ";
		break;
	case codeKind::ck_setVariable:
		cout << "set var : ";
		break;
	case codeKind::ck_if:
		cout << "if_ sen : ";
		break;
	case codeKind::ck_while:
		cout << "while__ : ";
		break;
	case codeKind::ck_blocks:
		cout << "block__ : ";
		break;
	case codeKind::ck_addFunction:
		cout << "addfunc : ";
		break;
	case codeKind::ck_useFunction:
		cout << "usefunc : ";
		break;
	case codeKind::ck_returnInFunction:
		cout << "return_ : ";
		break;
	case codeKind::ck_addStruct:
		cout << "struct__ : ";
		break;
	case codeKind::ck_break:
		cout << "break__ : ";
		break;
	case codeKind::ck_continue:
		cout << "continue: ";
		break;
	case codeKind::ck_addsetVariable:
		cout << "adsetvar: ";
		break;
	default:
		break;
	}

	if (cs->ck != codeKind::ck_blocks)
	{
		for (int i = 0; i < cs->maxlen; ++i)
		{
			cout << cs->sen[i] << " ";
		}
		cout << endl;
	}
	else
	{
		cout << "{" << endl;
		for (int i = 0; i < cs->codeblocks->size(); ++i)
		{
			dbg_codesen(reinterpret_cast<code_sen *>(cs->codeblocks->at(i)));
		}
		cout << "closed_ : }" << endl;
	}
}

struct type_data
{
	lcstr name;

	int typesiz;

	char typetype = 'b';
	// b - basictype / s - struct type
	// p - pointer / a - array

	int *structptr = nullptr;
	// a or p - it is subtype
	// s - struct_data*
};

struct NamingData
{
	char *name;
	type_data *td;
	int add_address;
};

struct operator_data
{
	const char *symbol;
	char mod = 'f'; // f : front / o - asmoper
	int startop = 0;
	int endop = 0;
};

operator_data create_oper(const char *symbo, char mo, int starto, int endo)
{
	operator_data op;
	op.symbol = symbo;
	op.mod = mo;
	op.startop = starto;
	op.endop = endo;
	return op;
}

enum class blockstate
{
	bs_if,
	bs_while,
	bs_function,
	bs_struct,
	bs_else
};

struct block_data
{
	byte8 *start_pc;
	vecarr<NamingData> variable_data;
	int add_address_up = 0;
	blockstate bs;
	uint64_t parameter[5] = {};
	vecarr<int> *breakpoints;
	vecarr<int> *continuepoints;

	bool ifin = false;
	int lastcsindex = 0;
	int presentcsindex = 0;
};

struct func_data
{
	lcstr name;
	byte8 *start_pc;
	vecarr<NamingData> param_data; // save sizeoftype
	type_data *returntype;
};

struct struct_data
{
	lcstr name;
	vecarr<NamingData> member_data;
};

struct temp_mem
{
	vecarr<byte8> mem;
	int memsiz = 0;
	int valuetype = 0;
	type_data *valuetype_detail;
};

enum class casting_type
{
	byte_to_short = 0,
	byte_to_ushort = 1,
	byte_to_int = 2,
	byte_to_uint = 3,
	byte_to_float = 4,
	ubyte_to_float = 5,
	short_to_byte = 6,
	short_to_int = 7,
	short_to_float = 8,
	ushort_to_float = 9,
	int_to_byte = 10,
	int_to_short = 11,
	int_to_float = 12,
	uint_to_float = 13,
	float_to_byte = 14,
	float_to_ubyte = 15,
	float_to_short = 16,
	float_to_ushort = 17,
	float_to_int = 18,
	float_to_uint = 19,
	nocasting = 20
};

casting_type get_cast_type(int type0, int type1)
{
	if (type0 == 0)
	{
		switch (type1)
		{
		case 2:
			return casting_type::byte_to_short;
		case 3:
			return casting_type::byte_to_ushort;
		case 4:
			return casting_type::byte_to_int;
		case 5:
			return casting_type::byte_to_uint;
		case 6:
			return casting_type::byte_to_float;
		default:
			return casting_type::nocasting;
		}
	}
	else if (type0 == 1)
	{
		switch (type1)
		{
		case 6:
			return casting_type::ubyte_to_float;
		default:
			return casting_type::nocasting;
		}
	}
	else if (type0 == 2)
	{
		switch (type1)
		{
		case 0:
			return casting_type::short_to_byte;
		case 4:
			return casting_type::short_to_int;
		case 6:
			return casting_type::short_to_float;
		default:
			return casting_type::nocasting;
		}
	}
	else if (type0 == 3)
	{
		switch (type1)
		{
		case 6:
			return casting_type::ushort_to_float;
		default:
			return casting_type::nocasting;
		}
	}
	else if (type0 == 4)
	{
		switch (type1)
		{
		case 0:
			return casting_type::int_to_byte;
		case 2:
			return casting_type::int_to_short;
		case 6:
			return casting_type::int_to_float;
		default:
			return casting_type::nocasting;
		}
	}
	else if (type0 == 5)
	{
		switch (type1)
		{
		case 6:
			return casting_type::uint_to_float;
		default:
			return casting_type::nocasting;
		}
	}
	else if (type0 == 6)
	{
		switch (type1)
		{
		case 0:
			return casting_type::float_to_byte;
		case 1:
			return casting_type::float_to_ubyte;
		case 2:
			return casting_type::float_to_short;
		case 3:
			return casting_type::float_to_ushort;
		case 4:
			return casting_type::float_to_int;
		case 5:
			return casting_type::float_to_uint;
		default:
			return casting_type::nocasting;
		}
	}
	return casting_type::nocasting;
}

struct instruct_data
{
	char name[64] = {};
	int param_num = 0;
	int param_typesiz[4] = {};
};

class InsideCode_Bake
{
private:
	bool current_if_is_multiple = false;

public:
	FM_System0 *fm;

	// execute var
	uint32_t max_mem_byte = 40960; // 40KB
	byte8 *mem = nullptr;
	vecarr<byte8> datamem;
	int dataptr = max_mem_byte;

	static constexpr uint32_t max_instruction = 256;
	void *inst[max_instruction] = {};

	static constexpr int max_casting_type = 20;
	void *cast[max_casting_type] = {};

	static constexpr int max_dbgtype = 9;
	void *dbgt[max_dbgtype] = {};
	void *inpt[max_dbgtype] = {};

	circularArray<uint64_t> _as;
	circularArray<uint64_t> _bs;

	byte8 *pc = 0; // program counter
	byte8 *sp = 0; // stack pointer

	byte8 **pcb = nullptr;
	ushort **pcs = nullptr;
	uint **pci = nullptr;

	byte8 **spb = nullptr;
	ushort **sps = nullptr;
	uint **spi = nullptr;

	vecarr<byte8 *> fsp;
	vecarr<byte8 *> call_stack;

	byte8 *rfsp = 0; // function stack pos
	byte8 *lfsp = 0; // last function stack pos

	byte8 **rfspb = nullptr;
	ushort **rfsps = nullptr;
	uint **rfspi = nullptr;

	byte8 **lfspb = nullptr;
	ushort **lfsps = nullptr;
	uint **lfspi = nullptr;

	uint64_t _a = 0;
	uint64_t _b = 0;
	uint64_t _x = 0;
	uint64_t _y = 0;
	uint64_t _la = 0; // left address

	// compile var
	vecarr<char *> allcode_sen;
	word_base_sen_sys wbss;
	vecarr<code_sen *> *csarr;
	// infArray < code_sen > sen_arr;

	vecarr<func_data *> functions;
	vecarr<type_data *> types;
	vecarr<block_data *> blockstack;
	vecarr<NamingData *> globalVariables;

	block_data nextbd;
	struct_data *nextsd;

	instruct_data inst_meta[256];

	int writeup = 0;

	static constexpr int basictype_max = 8;
	type_data *basictype[basictype_max];

	static constexpr int basicoper_max = 19;
	operator_data basicoper[basicoper_max];

	void release_tempmem(temp_mem *ptr)
	{
		ptr->mem.release();
		fm->_Delete((byte8 *)ptr, sizeof(temp_mem));
		// type_data memeory management reqired.. but how?
	}

	type_data *create_type(char *nam, int tsiz, char typ, int *strptr)
	{
		type_data *td = (type_data *)fm->_New(sizeof(type_data), true);
		td->name.NULLState();
		td->name.Init(2, false);
		td->name = nam;
		td->structptr = strptr;
		td->typesiz = tsiz;
		td->typetype = typ;
		return td;
	}

	func_data *get_func_with_name(char *name)
	{
		for (int i = 0; i < functions.size(); ++i)
		{
			if (strcmp(functions[i]->name.c_str(), name) == 0)
			{
				return functions[i];
			}
		}
		return nullptr;
	}

	type_data *get_sub_type(type_data *t)
	{
		return reinterpret_cast<type_data *>(t->structptr);
	}

	type_data *get_addpointer_type(type_data *td)
	{
		type_data *rtd = (type_data *)fm->_New(sizeof(type_data), true);
		rtd->name.NULLState();
		rtd->name.Init(td->name.size() + 1, false);

		int len = strlen(td->name.c_str());
		rtd->name.operator=(td->name.c_str());
		rtd->name.push_back('*');
		rtd->structptr = reinterpret_cast<int *>(td);
		rtd->typesiz = 4;
		rtd->typetype = 'p';
		return rtd;
	}

	type_data *get_array_type(type_data *td, int size)
	{
		type_data *rtd = (type_data *)fm->_New(sizeof(type_data), true);
		rtd->name.NULLState();
		rtd->name.Init(2, false);
		int len = strlen(td->name.c_str());
		rtd->name = td->name.c_str();
		rtd->name.push_back('[');

		string str = to_string(size);
		char *strnum = (char *)fm->_New(str.size() + 2, true);
		strcpy(strnum, str.c_str());
		for (int i = 0; i < strlen(strnum); ++i)
		{
			rtd->name.push_back(strnum[i]);
		}
		rtd->name.push_back(']');

		rtd->structptr = reinterpret_cast<int *>(td);
		rtd->typesiz = td->typesiz * size;
		rtd->typetype = 'a';
		return rtd;
	}

	type_data *get_basic_type_with_int(int n)
	{
		switch (n)
		{
		case 0:
			return basictype[1];
		case 1:
			return basictype[7];
		case 2:
			return basictype[2];
		case 3:
			return basictype[6];
		case 4:
			return basictype[0];
		case 5:
			return basictype[5];
		case 6:
			return basictype[3];
		case 7:
			return basictype[4];
		}
		return nullptr;
	}

	int get_int_with_basictype(type_data *td)
	{
		if (td == basictype[1])
			return 0;
		if (td == basictype[7])
			return 1;
		if (td == basictype[2])
			return 2;
		if (td == basictype[6])
			return 3;
		if (td == basictype[0])
			return 4;
		if (td == basictype[5])
			return 5;
		if (td == basictype[3])
			return 6;
		if (td == basictype[4])
			return 7;
		return 8;
	}

	InsideCode_Bake()
	{
	}

	virtual ~InsideCode_Bake()
	{
	}

	bool IsTypeString(const char *str)
	{
		for (int i = 0; i < basictype_max; ++i)
		{
			if (strcmp(str, basictype[i]->name.c_str()) == 0)
				return true;
		}
		return false;
	}

	sen *get_sen_from_codesen(code_sen *cs)
	{
		sen *rs = (sen *)fm->_New(sizeof(sen), true);
		rs->NULLState();
		rs->Init(2, false);

		for (int i = 0; i < cs->maxlen; ++i)
		{
			segment seg;
			seg.type = 'w';
			seg.data.str = cs->sen[i];
			rs->push_back(seg);
		}

		return rs;
	}

	void setfm(FM_System0 *fms)
	{
		fm = fms;
		allcode_sen.NULLState();
		allcode_sen.Init(2, false);
		allcode_sen.islocal = false;
		csarr = nullptr;
		// word_arr.Init(10, fm);
		// sen_arr.Init(10, fm);
		wbss.fm = fm;
	}

	void read_inst_table()
	{
		FILE *file = fopen("instruction_table.txt", "r"); // Open the file
		// in read
		// mode
		if (file == nullptr)
		{
			std::cout << "Failed to open the file." << std::endl;
			return;
		}

		int c;
		instruct_data id;
		int idi = 0;
		int pi = 0;
		int metai = 0;
		while ((c = fgetc(file)) != EOF)
		{
			if ((c == ':' || c == ' ') || (c == '\n'))
			{
				continue;
			}
			if (c == '+')
			{
				id.param_num += 1;
				id.name[idi] = 0;
				c = fgetc(file);
				switch (c)
				{
				case '1':
					id.param_typesiz[pi++] = 1;
					break;
				case '2':
					id.param_typesiz[pi++] = 2;
					break;
				case '4':
					id.param_typesiz[pi++] = 4;
					break;
				}
			}
			else if (c == ';')
			{
				id.name[idi] = 0;
				inst_meta[metai++] = id;
				id.param_num = 0;
				idi = 0;
				pi = 0;
			}
			else
			{
				id.name[idi++] = c;
			}
		}

		for (int k = metai; k < 256; ++k)
		{
			inst_meta[k].param_num = -1;
		}

		fclose(file); // Close the file

		for (int i = 0; i < 256; ++i)
		{
			cout << i << " : " << inst_meta[i].name << endl;
		}
	}

	void dbg_registers()
	{
		cout << "a : " << (uint)_as[0] << "(";
		for (int i = 1; i < _as.maxsiz; ++i)
		{
			cout << "|" << _as[i];
		}
		cout << endl;
		cout << "b : " << (uint)_bs[0] << "(";
		for (int i = 1; i < _bs.maxsiz; ++i)
		{
			cout << "|" << _bs[i];
		}
		cout << endl;
		cout << "x : " << (uint)_x << endl;
		cout << "y : " << (uint)_y << endl;
	}

	void dbg_stack()
	{
		cout << "stack mem" << endl;
		for (byte8 *ptr = sp; ptr != &mem[max_mem_byte - 1]; ++ptr)
		{
			cout << (int)*ptr << ' ';
		}
		cout << endl;
	}

	void dbg_data(){
		cout << "data mem" << endl;
		for (int i=0;i<datamem.size();++i)
		{
			cout << (int)datamem[i] << ' ';
		}
		cout << endl;
	}

	code_sen *find_codesen_with_linenum(vecarr<code_sen *> *csa, int line)
	{
		for (int i = 0; i < csa->size(); ++i)
		{
			code_sen *cs = csa->at(i);
			if (cs->start_line <= line && line <= cs->end_line)
			{
				if (cs->ck == codeKind::ck_blocks)
				{
					return find_codesen_with_linenum(reinterpret_cast<vecarr<
														 code_sen *> *>(cs->codeblocks),
													 line);
				}
				else
				{
					return cs;
				}
			}
		}

		return nullptr;
	}

	void dbg_pc()
	{
		if (inst_meta[*pc].param_num < 0)
		{
			return;
		}
		code_sen *cs = find_codesen_with_linenum(csarr, (int)(pc - mem));
		if (cs != nullptr)
		{
			dbg_codesen(cs);
		}
		cout << (int)(pc - mem) << " : " << inst_meta[*pc].name << "(" << (uint)*pc << ")";
		instruct_data id = inst_meta[*pc];
		int n;
		for (int k = 0; k < id.param_num; ++k)
		{
			switch (id.param_typesiz[k])
			{
			case 1:
				n = (uint) * (pc + 1);
				cout << " > " << n;
				break;
			case 2:
				n = *reinterpret_cast<ushort *>(pc + 1);
				cout << " > " << n << "(" << (uint) * (pc + 1) << ", " << (uint) * (pc + 2) << ")";
				break;
			case 4:
				n = *reinterpret_cast<uint *>(pc + 1);
				cout << " > " << n << "(" << (uint) * (pc + 1) << ", " << (uint) * (pc + 2) << ", " << (uint) * (pc + 3) << ", " << (uint) * (pc + 4) << ")";
				break;
			}
		}
		cout << endl;
	}

	void print_asm(int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			if (inst_meta[mem[i]].param_num < 0)
			{
				return;
			}
			if (mem[i] == 214 || mem[i] == 215)
			{
				cout << i << "\t:" << inst_meta[mem[i]].name << "(" << (uint)mem[i] << ")";
				uint strmax = 0;
				instruct_data id = inst_meta[mem[i]];
				int n;
				for (int k = 0; k < id.param_num; ++k)
				{
					switch (id.param_typesiz[k])
					{
					case 1:
						++i;
						n = (uint)mem[i];
						cout << " > " << n;
						break;
					case 2:
						++i;
						n = *reinterpret_cast<ushort *>(&mem[i]);
						cout << " > " << n << "(" << (uint)mem[i] << ", " << (uint)mem[i + 1] << ")";
						i += 1;
						break;
					case 4:
						++i;
						n = *reinterpret_cast<uint *>(&mem[i]);
						strmax = n;
						cout << " > " << n << "(" << (uint)mem[i] << ", " << (uint)mem[i + 1] << ", "
							 << (uint)mem[i + 2] << ", " << (uint)mem[i + 3] << ")";
						i += 3;
						break;
					}
				}
				cout << "\"";
				for (int k = 1; k < strmax; ++k)
				{
					cout << mem[i + k];
				}
				cout << "\"";
				cout << endl;
				i += strmax + 1;
			}
			cout << i << "\t:" << inst_meta[mem[i]].name << "(" << (uint)mem[i] << ")";
			instruct_data id = inst_meta[mem[i]];
			int n;
			for (int k = 0; k < id.param_num; ++k)
			{
				switch (id.param_typesiz[k])
				{
				case 1:
					++i;
					n = (uint)mem[i];
					cout << " > " << n;
					break;
				case 2:
					++i;
					n = *reinterpret_cast<ushort *>(&mem[i]);
					cout << " > " << n << "(" << (uint)mem[i] << ", " << (uint)mem[i + 1] << ")";
					i += 1;
					break;
				case 4:
					++i;
					n = *reinterpret_cast<uint *>(&mem[i]);
					cout << " > " << n << "(" << (uint)mem[i] << ", " << (uint)mem[i + 1] << ", " << (uint)mem[i + 2] << ", " << (uint)mem[i + 3] << ")";
					i += 3;
					break;
				}
			}
			cout << endl;
		}
	}

	void dbg_bakecode(vecarr<code_sen *> *csa, int sav)
	{
		// cout << "all asm :" << endl;
		int save = sav;
		for (int i = 0; i < csa->size(); ++i)
		{
			code_sen *cs = csa->at(i);
			if (cs->start_line > save)
			{
				cout << "\033[0;36m";
				cout << "<nocode>" << endl;
				cout << "\033[0;37m";
				print_asm(save, cs->start_line - 1);
				save = cs->start_line;
				cout << endl;
			}
			if (cs->ck == codeKind::ck_blocks)
			{
				vecarr<code_sen *> *css =
					reinterpret_cast<vecarr<code_sen *> *>(cs->codeblocks);
				dbg_bakecode(css, save);
				save = css->last()->end_line + 1;
			}
			else
			{
				cout << "\033[0;36m";
				dbg_codesen(cs);
				cout << "\033[0;37m";
				print_asm(cs->start_line, cs->end_line);
				save = cs->end_line + 1;
				cout << endl;
			}
		}
	}

	void init()
	{
		wbss.Init();

		fsp.NULLState();
		fsp.Init(2, false);

		call_stack.NULLState();
		call_stack.Init(2, false);

		char *name[8] = {};
		name[0] = (char *)fm->_New(4, true);
		strcpy(name[0], "int");
		basictype[0] = create_type(name[0], 4, 'b', nullptr);

		name[1] = (char *)fm->_New(5, true);
		strcpy(name[1], "char");
		basictype[1] = create_type(name[1], 1, 'b', nullptr);

		name[2] = (char *)fm->_New(6, true);
		strcpy(name[2], "short");
		basictype[2] = create_type(name[2], 2, 'b', nullptr);

		name[3] = (char *)fm->_New(6, true);
		strcpy(name[3], "float");
		basictype[3] = create_type(name[3], 4, 'b', nullptr);

		name[4] = (char *)fm->_New(5, true);
		strcpy(name[4], "bool");
		basictype[4] = create_type(name[4], 4, 'b', nullptr);

		name[5] = (char *)fm->_New(5, true);
		strcpy(name[5], "uint");
		basictype[5] = create_type(name[5], 4, 'b', nullptr);

		name[6] = (char *)fm->_New(7, true);
		strcpy(name[6], "ushort");
		basictype[6] = create_type(name[6], 2, 'b', nullptr);

		name[7] = (char *)fm->_New(6, true);
		strcpy(name[7], "uchar");
		basictype[7] = create_type(name[7], 1, 'b', nullptr);

		for (int i = 0; i < basictype_max; ++i)
		{
			types.push_back(basictype[i]);
		}

		block_data *bd = (block_data *)fm->_New(sizeof(block_data), true);
		bd->start_pc = &mem[0];
		bd->add_address_up = 0;
		bd->variable_data.NULLState();
		bd->variable_data.Init(10, false);

		basicoper[0] = create_oper("[", 'f', 0, 0);
		basicoper[1] = create_oper(".", 'f', 0, 0);
		basicoper[2] = create_oper("->", 'f', 0, 0);
		basicoper[3] = create_oper("&", 'f', 0, 0);
		basicoper[4] = create_oper("*", 'f', 0, 0);
		basicoper[5] = create_oper("*", 'o', 81, 94);
		basicoper[6] = create_oper("/", 'o', 95, 108);
		basicoper[7] = create_oper("%", 'o', 109, 120);
		basicoper[8] = create_oper("+", 'o', 53, 66);
		basicoper[9] = create_oper("-", 'o', 67, 80);
		basicoper[10] = create_oper("==", 'o', 127, 128);
		basicoper[11] = create_oper("<", 'o', 159, 172);
		basicoper[12] = create_oper(">", 'o', 131, 144);
		basicoper[13] = create_oper("<=", 'o', 173, 186);
		basicoper[14] = create_oper(">=", 'o', 145, 158);
		basicoper[15] = create_oper("!=", 'o', 129, 130);
		basicoper[16] = create_oper("!", 'o', 125, 126);
		basicoper[17] = create_oper("&&", 'o', 121, 122);
		basicoper[18] = create_oper("||", 'o', 123, 124);

		mem = new byte8[max_mem_byte];
		for (int i = 0; i < max_mem_byte; ++i)
		{
			mem[i] = 255;
		}

		cout << "instruction table" << endl;
		read_inst_table();

		blockstack.NULLState();
		blockstack.Init(2, false);
		blockstack.islocal = false;

		pc = &mem[0];
		sp = &mem[max_mem_byte - 1];
		lfsp = sp;
		rfsp = sp;

		nextbd.breakpoints = (vecarr<int> *)fm->_New(sizeof(vecarr<int>), true);
		nextbd.continuepoints = (vecarr<int> *)fm->_New(sizeof(vecarr<int>), true);

		functions.NULLState();
		functions.Init(2, false);

		globalVariables.NULLState();
		globalVariables.Init(2, false);

		datamem.NULLState();
		datamem.Init(2, false);

		_as.Init(32, fm);
		_bs.Init(32, fm);
	}

	void push_word(lcstr &str)
	{
		const char *sptr = str.c_str();
		int len = strlen(sptr);
		char *cstr = (char *)fm->_New(len + 1, true);

		for (int i = 0; i < len; ++i)
		{
			cstr[i] = sptr[i];
		}
		cstr[len] = 0;

		for (int i = 0; i < wbss.wordlist.size(); ++i)
		{
			if (strcmp(cstr, wbss.wordlist.at(i)->c_str()) == 0)
			{
				fm->_Delete((byte8 *)cstr, len + 1);
				allcode_sen.push_back(wbss.wordlist.at(i)->c_str());
				return;
			}
		}

		wbss.addword(cstr);
		allcode_sen.push_back(cstr);
	}

	void set_word(int index, lcstr &str)
	{
		const char *sptr = str.c_str();
		int len = strlen(sptr);
		char *cstr = (char *)fm->_New(len + 1, true);
		strcpy(cstr, sptr);
		cstr[len] = 0;

		for (int i = 0; i < wbss.wordlist.size(); ++i)
		{
			if (strcmp(cstr, wbss.wordlist.at(i)->c_str()) == 0)
			{
				fm->_Delete((byte8 *)cstr, len + 1);
				allcode_sen[index] = wbss.wordlist.at(i)->c_str();
				return;
			}
		}

		wbss.addword(cstr);
		allcode_sen[index] = cstr;
	}

	void AddTextBlocks(lcstr &codetxt)
	{
		lcstr insstr;
		insstr.NULLState();
		insstr.Init(2, false);

		cout << "\n\ntext : \n"
			 << codetxt.c_str() << endl;
		for (int i = 0; i < (int)codetxt.size(); i++)
		{
			insstr.push_back(codetxt.at(i));
			if (bCanBeTextblock(insstr))
			{
				if (i == codetxt.size() - 1)
				{
					cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
					push_word(insstr);
					continue;
				}
				insstr.push_back(codetxt.at(i + 1));
				if (bCanBeTextblock(insstr) == false)
				{
					insstr.pop_back();
					cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
					push_word(insstr);
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
						push_word(insstr);
						insstr.clear();
						insstr.push_back(c);
						cout << "siz : " << i << "-" << codetxt.size() << " str : " << insstr.c_str() << endl;
						push_word(insstr);
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

		for (int i = 0; i < (int)allcode_sen.size(); ++i)
		{
			lcstr s;
			s = allcode_sen[i];
			if (s == "-")
			{
				if (i == 0)
					continue;
				lcstr t0;
				t0 = allcode_sen[i - 1];
				lcstr t1;
				t1 = allcode_sen[i + 1];

				bool bequ = (DecodeTextBlock(t0) == TBT::_operation) && (DecodeTextBlock(t1) != TBT::_operation);
				if (bequ)
				{
					lcstr insstr;
					insstr = allcode_sen[i];
					for (int k = 0; k < t1.size(); k++)
					{
						insstr.push_back(t1[k]);
					}

					set_word(i, insstr);
					allcode_sen.erase(i + 1);
				}
			}

			if (s == "=")
			{
				if (i == 0)
					continue;

				char c = allcode_sen[i - 1][0];
				bool bequ = (c == '!' || c == '<') || (c == '=' || c == '>');
				bequ = bequ || ((c == '+' || c == '-') || (c == '*' || c == '/'));
				bequ = bequ || (c == '%');
				if (bequ)
				{
					lcstr insstr;
					insstr = allcode_sen[i - 1];
					insstr.push_back('=');
					set_word(i - 1, insstr);
					allcode_sen.erase(i);
				}
			}
			if (s == "|")
			{
				if (i == 0)
					continue;
				char c = allcode_sen[i + 1][0];
				bool bequ = (c == '|');
				if (bequ)
				{
					lcstr insstr;
					insstr = allcode_sen[i];
					insstr.push_back('|');
					set_word(i, insstr);
					allcode_sen.erase(i + 1);
				}
			}
			if (s == "&")
			{
				if (i == 0)
					continue;
				char c = allcode_sen[i + 1][0];
				bool bequ = (c == '&');
				if (bequ)
				{
					lcstr insstr;
					insstr = allcode_sen[i];
					insstr.push_back('&');
					set_word(i, insstr);
					allcode_sen.erase(i + 1);
				}
			}
			if (s == ".")
			{
				if (i - 1 < 0 && i + 1 > allcode_sen.size() - 1)
					continue;
				lcstr front;
				front = allcode_sen[i - 1];
				lcstr back;
				back = allcode_sen[i + 1];
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
					insstr = allcode_sen[i - 1];
					insstr.push_back('.');
					for (int k = 0; k < back.size(); k++)
					{
						insstr.push_back(back[k]);
					}
					set_word(i - 1, insstr);
					allcode_sen.erase(i);
					allcode_sen.erase(i);
				}
			}

			if (s == "\'")
			{
				if (strcmp(allcode_sen[i + 2], "\'") == 0)
				{
					lcstr insstr;
					insstr = allcode_sen[i];
					lcstr cent;
					cent = allcode_sen[i + 1];
					lcstr back;
					back = allcode_sen[i + 2];
					for (int k = 0; k < cent.size(); k++)
					{
						insstr.push_back(cent.at(k));
					}

					for (int k = 0; k < back.size(); k++)
					{
						insstr.push_back(back.at(k));
					}

					set_word(i, insstr);
					allcode_sen.erase(i + 1);
					allcode_sen.erase(i + 1);
				}
				else if (strcmp(allcode_sen[i + 1], "\\") == 0 && strcmp(allcode_sen[i + 3], "\'") == 0)
				{
					lcstr insstr;
					insstr = allcode_sen[i];
					lcstr cent;
					cent = allcode_sen[i + 1];
					lcstr back;
					back = allcode_sen[i + 2];
					lcstr backback;
					backback = allcode_sen[i + 3];
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

					set_word(i, insstr);
					allcode_sen.erase(i + 1);
					allcode_sen.erase(i + 1);
					allcode_sen.erase(i + 1);
				}
			}
		}

		insstr.islocal = true;
	}

	void set_codesen(code_sen *sen, vecarr<char *> &arr, FM_System0 *fm)
	{
		sen->maxlen = arr.size();
		sen->sen = (char **)fm->_New(sizeof(char *) * sen->maxlen, true);
		for (int i = 0; i < sen->maxlen; ++i)
		{
			sen->sen[i] = arr[i];
			// cout << arr[i] << ' '; // dbg
		}
		// cout << endl; // dbg
	}

	vecarr<code_sen *> *AddCodeFromBlockData(vecarr<char *> &allcodesen, const char *ScanMod)
	{
		// allcode_sen-> allcode_sen
		// ic -> this
		// code -> sen_arr

		vecarr<code_sen *> *senarr =
			(vecarr<code_sen *> *)fm->_New(sizeof(vecarr<code_sen *>), true);
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
					if (IsTypeString(allcodesen[i]))
					{
						if (strcmp(allcodesen[i + 2], "(") == 0)
						{
							// addfunction
							code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
							cs->ck = codeKind::ck_addFunction;
							vecarr<char *> cbs;
							cbs.NULLState();
							cbs.Init(3, true);
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

							set_codesen(cs, cbs, fm);

							senarr->push_back(cs);
							i = startI - 1;
							StartI = i + 1;
						}
						else
						{
							int k = 1;
							int v = k;
							bool addset = false;
							while (true)
							{
								if (addset)
								{
									if (strcmp(allcodesen[i + v], ";") != 0)
									{
										v++;
									}
									else
									{
										break;
									}
								}
								else
								{
									if (strcmp(allcodesen[i + k], "=") == 0)
									{
										addset = true;
										v = k + 1;
										continue;
									}

									if (strcmp(allcodesen[i + k], ";") != 0)
									{
										k++;
									}
									else
									{
										break;
									}
								}
							}

							if (addset == false)
							{
								code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
								cs->ck = codeKind::ck_addVariable;
								vecarr<char *> cbs;
								cbs.NULLState();
								cbs.Init(3, true);
								for (int j = 0; j < k; j++)
								{
									cbs.push_back(allcodesen[i + j]);
								}

								set_codesen(cs, cbs, fm);
								senarr->push_back(cs);
								i += k;
								StartI = i + 1;
							}
							else
							{
								code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
								cs->ck = codeKind::ck_addsetVariable;
								vecarr<char *> cbs;
								cbs.NULLState();
								cbs.Init(3, true);
								for (int j = 0; j < v; j++)
								{
									cbs.push_back(allcodesen[i + j]);
								}

								set_codesen(cs, cbs, fm);
								senarr->push_back(cs);
								i += v;
								StartI = i + 1;
							}
						}
					}
					else if (allcodesen.size() > i + 2 && (strcmp(allcodesen[i + 2], "(") == 0 && strcmp(allcodesen[i], "void") == 0))
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_addFunction;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
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

						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						i = startI - 1;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "{") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_blocks;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(2, true);

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

						vecarr<code_sen *> *cbv = AddCodeFromBlockData(cbs, "none");

						cs->codeblocks = (vecarr<int *> *)fm->_New(sizeof(vecarr<int *>), true);
						cs->fm = fm;
						cs->codeblocks->islocal = false;
						Init_VPTR<vecarr<int *> *>(cs->codeblocks);

						for (int u = 0; u < (int)cbv->size(); u++)
						{
							cs->codeblocks->push_back(reinterpret_cast<int *>((*cbv)[u]));
						}

						cbv->release();
						fm->_Delete((byte8 *)cbv, sizeof(cbv));

						senarr->push_back(cs);

						i += h;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "=") == 0 || (strlen(allcodesen[i]) == 2 && allcodesen[i][1] == '='))
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_setVariable;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);

						int h = 0;
						for (int k = StartI; k < i; k++)
						{
							cbs.push_back(allcodesen[k]);
						}

						while (allcodesen[i + h][0] != ';')
						{
							cbs.push_back(allcodesen[i + h]);
							h++;
						}

						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);

						i += h;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "if") == 0)
					{
						// if�� �Ұ�ȣ������,
						// codeblock�� ��´�. if��
						// �۵������ true�� ����
						// �ڵ������ ����,
						// �ƴϸ�
						// �ǳʶٴ� ���̴�.
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_if;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
						int open = 0;
						int h = 0;
						cbs.push_back(allcodesen[i]);
						while (strcmp(allcodesen[i + h], ")") != 0 || open != 1)
						{
							if (strcmp(allcodesen[i + h], "(") == 0)
								open++;
							else if (strcmp(allcodesen[i + h], ")") == 0)
								open--;
							h++;
							cbs.push_back(allcodesen[i + h]);
						}

						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						i += h;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "else") == 0)
					{
						if (allcodesen.size() > i + 1 && strcmp(allcodesen[i + 1], "if") == 0)
						{
							code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
							cs->ck = codeKind::ck_if;
							vecarr<char *> cbs;
							cbs.NULLState();
							cbs.Init(3, true);

							int open = 0;
							int h = 1;
							cbs.push_back(allcodesen[i]);
							cbs.push_back(allcodesen[i + 1]);
							while (strcmp(allcodesen[i + h], ")") != 0 || open != 1)
							{
								if (strcmp(allcodesen[i + h], "(") == 0)
									open++;
								else if (strcmp(allcodesen[i + h], ")") == 0)
									open--;
								h++;
								cbs.push_back(allcodesen[i + h]);
							}

							set_codesen(cs, cbs, fm);
							senarr->push_back(cs);
							i += h;
							StartI = i + 1;
							// else if �� ���
						}
						else
						{
							code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
							cs->ck = codeKind::ck_if;
							vecarr<char *> cbs;
							cbs.NULLState();
							cbs.Init(3, true);
							// �׳� else�� ���
							cbs.push_back(allcodesen[i]);
							set_codesen(cs, cbs, fm);
							senarr->push_back(cs);
						}
					}
					else if (strcmp(allcodesen[i], "while") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_while;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
						int open = 0;
						int h = 0;
						cbs.push_back(allcodesen[i]);
						while (strcmp(allcodesen[i + h], ")") != 0 || open != 1)
						{
							if (strcmp(allcodesen[i + h], "(") == 0)
								open++;
							else if (strcmp(allcodesen[i + h], ")") == 0)
								open--;
							h++;
							cbs.push_back(allcodesen[i + h]);
						}
						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						i += h;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i + 1], "(") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_useFunction;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
						int h = 0;
						while (i + h < allcodesen.size() && strcmp(allcodesen[i + h], ";") != 0)
						{
							cbs.push_back(allcodesen[i + h]);
							h++;
						}
						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						i += h;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "return") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_returnInFunction;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
						int h = 0;
						while (strcmp(allcodesen[i + h], ";") != 0)
						{
							cbs.push_back(allcodesen[i + h]);
							h++;
						}
						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						i += h;
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "break") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_break;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
						cbs.push_back(allcodesen[i]);
						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						StartI = i + 1;
					}
					else if (strcmp(allcodesen[i], "continue") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_continue;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);
						cbs.push_back(allcodesen[i]);
						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
						StartI = i + 1;
					}
				}

				if (strcmp(ScanMod, "struct scan") == 0)
				{
					if (strcmp(allcodesen[i], "struct") == 0)
					{
						code_sen *cs = (code_sen *)fm->_New(sizeof(code_sen), true);
						cs->ck = codeKind::ck_addStruct;
						vecarr<char *> cbs;
						cbs.NULLState();
						cbs.Init(3, true);

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

						vecarr<char *> bd;
						bd.NULLState();
						bd.Init(10, false);
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

						vecarr<code_sen *> *cbv = AddCodeFromBlockData(bd, "none");

						cs->codeblocks = (vecarr<int *> *)fm->_New(sizeof(vecarr<int *>), true);
						cs->fm = fm;
						Init_VPTR<vecarr<int *> *>(cs->codeblocks);

						for (int u = 0; u < (int)cbv->size(); u++)
						{
							cs->codeblocks->push_back(reinterpret_cast<int *>((*cbv)[u]));
						}

						cbv->release();
						fm->_Delete((byte8 *)cbv, sizeof(cbv));

						set_codesen(cs, cbs, fm);
						senarr->push_back(cs);
					}
				}
			}
		}

		return senarr;
	}

	int get_global_address_with_globalv_name(char *name)
	{
		for (int i = 0; i < globalVariables.size(); ++i)
		{
			NamingData nd;
			nd = *globalVariables.at(i);
			if (strcmp(nd.name, name) == 0)
			{
				return nd.add_address;
			}
		}
		return -1;
	}

	int get_address_with_name(char *name)
	{
		bool found = false;
		int si = 0;
		int sk = 0;
		for (int i = blockstack.size() - 1; i >= 0; --i)
		{
			for (int k = 0; k < blockstack[i]->variable_data.size(); ++k)
			{
				if (strcmp(blockstack[i]->variable_data[k].name, name) == 0)
				{
					found = true;
					si = i;
					sk = k;
					break;
				}
			}

			if (found)
			{
				break;
			}
		}

		if (found)
		{
			int addadd_z = 0;
			for (int i = 0; i <= si - 1; ++i)
			{
				addadd_z += blockstack[i]->add_address_up;
			}

			return addadd_z + blockstack[si]->variable_data[sk].add_address;
		}

		return -1;
	}

	type_data *get_type_with_namesen(sen *tname)
	{
		type_data *td = nullptr;
		type_data *ntd = nullptr;
		char *bt = tname->at(0).data.str;
		for (int i = 0; i < types.size(); ++i)
		{
			if (strcmp(bt, types[i]->name.c_str()) == 0)
			{
				td = types[i];
			}
		}
		for (int i = 1; i < tname->size(); ++i)
		{
			char c = 0;
			if (tname->at(i).data.str != nullptr)
			{
				cout << (int *)tname->at(i).data.str << endl;
				c = tname->at(i).data.str[0];
			}
			if (c == '*')
			{
				ntd = get_addpointer_type(td);
				if (i != 1)
				{
					fm->_Delete((byte8 *)td, sizeof(td));
				}
				td = ntd;
			}
			else if (c == '[')
			{
				int siz = atoi(tname->at(i + 1).data.str);
				// get array type with siz
				ntd = get_array_type(td, siz);
				td = ntd;
			}
		}
		return td;
	}

	type_data *get_type_with_global_vname(char *name)
	{
		for (int i = 0; i < globalVariables.size(); ++i)
		{
			NamingData nd;
			nd = *globalVariables.at(i);
			if (strcmp(nd.name, name) == 0)
			{
				return nd.td;
			}
		}
		return nullptr;
	}

	type_data *get_type_with_vname(char *name)
	{
		bool found = false;
		int si = 0;
		int sk = 0;
		for (int i = blockstack.size() - 1; i >= 0; --i)
		{
			for (int k = 0; k < blockstack[i]->variable_data.size(); ++k)
			{
				if (strcmp(blockstack[i]->variable_data[k].name, name) == 0)
				{
					found = true;
					si = i;
					sk = k;
					break;
				}
			}

			if (found)
			{
				break;
			}
		}

		if (found)
		{
			return blockstack[si]->variable_data[sk].td;
		}

		return nullptr;
	}

	int combine_opertype(int type0, int type1)
	{
		int max = (type0 >= type1) ? type0 : type1;
		if ((type0 + type1) % 2 == 1 && max % 2 == 1)
		{
			return max - 1;
		}
		else
		{
			return max;
		}
	}

	temp_mem *get_asm_from_sen(sen *ten, bool is_a, bool isvalue)
	{
		// fm->dbg_fm1_lifecheck();
		temp_mem *tm = (temp_mem *)fm->_New(sizeof(temp_mem), true);
		// fm->dbg_fm1_lifecheck();
		tm->mem.NULLState();
		tm->mem.Init(2, false);
		tm->valuetype_detail = nullptr;
		if (ten->at(0).type == 'a')
		{
			tm = reinterpret_cast<temp_mem *>(ten->at(0).data.str);
			return tm;
		}

		if (ten->size() > 1 && strcmp(ten->at(1).data.str, "(") == 0)
		{
			// function
			// use compile_useFunction()
			func_data *fd = nullptr;

			sen *code = ten;
			int loc = code->size() - 1;
			sen *inner_params = wbss.oc_search_inv(code, loc, "(", ")");
			// wbss.dbg_sen(code);
			// wbss.dbg_sen(inner_params);
			int nameloc = loc - inner_params->size();
			char *funcname = code->at(nameloc).data.str;
			if (strcmp(funcname, "dbg") == 0)
			{
				inner_params->pop_back();
				inner_params->erase(0);
				temp_mem *rtm = get_asm_from_sen(inner_params, true, true);
				for (int k = 0; k < rtm->memsiz; ++k)
				{
					tm->mem.push_back(rtm->mem[k]);
				}
				tm->mem.push_back(209);
				tm->mem.push_back((byte8)rtm->valuetype);

				inner_params->release();
				fm->_Delete((byte8 *)inner_params, sizeof(sen));
				release_tempmem(rtm);
				return tm;
			}
			else if (strcmp(funcname, "inp") == 0)
			{
				inner_params->pop_back();
				inner_params->erase(0);
				temp_mem *rtm = get_asm_from_sen(inner_params, true, true);
				for (int k = 0; k < rtm->memsiz; ++k)
				{
					tm->mem.push_back(rtm->mem[k]);
				}
				tm->mem.push_back(210);
				tm->mem.push_back((byte8)rtm->valuetype);

				inner_params->release();
				fm->_Delete((byte8 *)inner_params, sizeof(sen));
				release_tempmem(rtm);
				return tm;
			}
			else if (strcmp(funcname, "sizeof") == 0)
			{
				inner_params->pop_back();
				inner_params->erase(0);
				// type_data* td = get_type_with_namesen(inner_params);

				// release_tempmem(rtm);
			}
			else if (strcmp(funcname, "codeptr") == 0)
			{
				// release_tempmem(rtm);
			}
			else if (strcmp(funcname, "goto") == 0)
			{
				// release_tempmem(rtm);
			}

			fd = get_func_with_name(code->at(nameloc).data.str);
			// mem[writeup++] = fd->start_pc; // func

			sen *params_sen = wbss.sen_cut(code, nameloc + 2, loc - 1);
			if (params_sen->size() == 0)
			{
				tm->mem.push_back(189); // FUNC
				tm->mem.push_back(200); // jmp
				byte8 bb[4] = {};
				*reinterpret_cast<uint *>(bb) = (uint)(fd->start_pc - &mem[0]);
				for (int u = 0; u < 4; ++u)
				{
					tm->mem.push_back(bb[u]);
				}

				inner_params->release();
				fm->_Delete((byte8 *)inner_params, sizeof(sen));

				params_sen->release();
				fm->_Delete((byte8 *)params_sen, sizeof(sen));
				tm->memsiz = tm->mem.size();
				return tm;
			}

			wbss.dbg_sen(params_sen);
			int coma = wbss.search_word_first(0, params_sen, ",");
			int savecoma = 0;
			int last = params_sen->size() - 1;

			int addadd = 0;
			int paramid = 0;

			tm->mem.push_back(189);

			while (coma != -1)
			{
				sen *param_sen = wbss.sen_cut(params_sen, savecoma, coma - 1);
				temp_mem *rtm = get_asm_from_sen(param_sen, true, true);
				for (int i = 0; i < rtm->memsiz; ++i)
				{
					tm->mem.push_back(rtm->mem[i]);
				}

				switch (fd->param_data[paramid].td->typesiz)
				{
				case 1:
					tm->mem.push_back(190); // param
					break;
				case 2:
					tm->mem.push_back(191); // param
					break;
				case 4:
					tm->mem.push_back(192); // param
					break;
				}

				++paramid;

				param_sen->release();
				fm->_Delete((byte8 *)param_sen, sizeof(sen));

				release_tempmem(rtm);
			}

			wbss.dbg_sen(params_sen);
			sen *param_sen = wbss.sen_cut(params_sen, savecoma, last);
			wbss.dbg_sen(param_sen);
			temp_mem *rtm = get_asm_from_sen(param_sen, true, true);
			for (int i = 0; i < rtm->memsiz; ++i)
			{
				tm->mem.push_back(rtm->mem[i]);
			}

			switch (fd->param_data[paramid].td->typesiz)
			{
			case 1:
				tm->mem.push_back(190); // param
				break;
			case 2:
				tm->mem.push_back(191); // param
				break;
			case 4:
				tm->mem.push_back(192); // param
				break;
			}

			tm->mem.push_back(200); // jmp
			int ll = tm->mem.size();
			for (int k = 0; k < 4; ++k)
			{
				tm->mem.push_back(0);
			}
			*reinterpret_cast<uint *>(&tm->mem[ll]) = (uint)(fd->start_pc - &mem[0]);

			tm->memsiz = tm->mem.size();
			tm->valuetype = get_int_with_basictype(fd->returntype);
			tm->valuetype_detail = fd->returntype;

			inner_params->release();
			fm->_Delete((byte8 *)inner_params, sizeof(sen));

			params_sen->release();
			fm->_Delete((byte8 *)params_sen, sizeof(sen));

			param_sen->release();
			fm->_Delete((byte8 *)param_sen, sizeof(sen));

			release_tempmem(rtm);

			return tm;
		}

		if (ten->size() == 1)
		{
			// if variable name or const
			int gvarid = get_global_address_with_globalv_name(ten->at(0).data.str);
			if (gvarid >= 0)
			{
				type_data *td = get_type_with_global_vname(ten->at(0).data.str);
				if (is_a)
				{
					tm->mem.push_back(18);
					byte8 add[4];
					*reinterpret_cast<uint *>(&add[0]) = dataptr + gvarid;
					for (int i = 0; i < 4; ++i)
					{
						tm->mem.push_back(add[i]);
					}
				}
				else
				{
					tm->mem.push_back(27);
					byte8 add[4];
					*reinterpret_cast<uint *>(&add[0]) = dataptr + gvarid;
					for (int i = 0; i < 4; ++i)
					{
						tm->mem.push_back(add[i]);
					}
				}

				if (isvalue)
				{
					if (is_a)
					{
						tm->mem.push_back(205);
					}
					else
					{
						tm->mem.push_back(206);
					}
					tm->memsiz = 6;
					tm->valuetype = get_int_with_basictype(td);
					tm->valuetype_detail = td;
					return tm;
				}
				else
				{
					tm->memsiz = 5;
					tm->valuetype = 8;
					tm->valuetype_detail = get_addpointer_type(td);
					return tm;
				}
			}

			wbss.dbg_sen(ten);
			int varid = get_address_with_name(ten->at(0).data.str);
			if (varid >= 0)
			{
				// variable
				type_data *td = get_type_with_vname(ten->at(0).data.str);
				if (is_a)
				{
					tm->mem.push_back(195);
					byte8 add[4];
					*reinterpret_cast<uint *>(&add[0]) = varid;
					for (int i = 0; i < 4; ++i)
					{
						tm->mem.push_back(add[i]);
					}
				}
				else
				{
					tm->mem.push_back(196);
					byte8 add[4];
					*reinterpret_cast<uint *>(&add[0]) = varid;
					for (int i = 0; i < 4; ++i)
					{
						tm->mem.push_back(add[i]);
					}
				}

				if (isvalue)
				{
					if (is_a)
					{
						tm->mem.push_back(205);
					}
					else
					{
						tm->mem.push_back(206);
					}
					tm->memsiz = 6;
					tm->valuetype = get_int_with_basictype(td);
					tm->valuetype_detail = td;
					return tm;
				}
				else
				{
					tm->memsiz = 5;
					tm->valuetype = 8;
					tm->valuetype_detail = get_addpointer_type(td);
					return tm;
				}
			}
			else
			{
				// const
				lcstr str;
				str.NULLState();
				str.Init(2, false);
				str = ten->at(0).data.str;
				TBT t = DecodeTextBlock(str);
				switch (t)
				{
				case TBT::_value_bool:
				{
					bool b = true;
					if (is_a)
					{
						tm->mem.push_back(18);
					}
					else
					{
						tm->mem.push_back(27);
					}
					if (strcmp(str.c_str(), "true") == 0)
						b = true;
					else
						b = false;
					for (int k = 0; k < 4; ++k)
					{
						tm->mem.push_back(0);
					}
					*reinterpret_cast<bool *>(&tm->mem[1]) = b;
					tm->memsiz = 5;
					tm->valuetype = 7;
				}
				break;
				case TBT::_value_integer:
				{
					int a = 0;
					if (is_a)
					{
						tm->mem.push_back(18);
					}
					else
					{
						tm->mem.push_back(27);
					}
					a = atoi(str.c_str());
					for (int k = 0; k < 4; ++k)
					{
						tm->mem.push_back(0);
					}
					*reinterpret_cast<int *>(&tm->mem[1]) = a;
					tm->memsiz = 5;
					tm->valuetype = 4;
				}
				break;
				case TBT::_value_float:
				{
					float a = 0;
					if (is_a)
					{
						tm->mem.push_back(18);
					}
					else
					{
						tm->mem.push_back(27);
					}
					a = stof(str.c_str());
					for (int k = 0; k < 4; ++k)
					{
						tm->mem.push_back(0);
					}
					*reinterpret_cast<float *>(&tm->mem[1]) = a;
					tm->memsiz = 5;
					tm->valuetype = 6;
				}
				break;
				case TBT::_value_char:
				{
					if (is_a)
					{
						tm->mem.push_back(16);
					}
					else
					{
						tm->mem.push_back(25);
					}
					if (str[1] != '\\')
					{
						tm->mem.push_back(str[1]);
					}
					else
					{
						switch (str[2])
						{
						case 'n':
							tm->mem.push_back('\n');
							break;
						case '0':
							tm->mem.push_back(0);
							break;
						case 't':
							tm->mem.push_back('\t');
							break;
						case '\\':
							tm->mem.push_back('\\');
							break;
						case '\'':
							tm->mem.push_back('\'');
							break;
						case '\"':
							tm->mem.push_back('\"');
							break;
						default:
							tm->mem.push_back(0);
							break;
						}
					}

					tm->memsiz = 2;
					tm->valuetype = 0;
				}
				break;
				case TBT::_value_str:
				{
					if (is_a)
					{
						tm->mem.push_back(214);
					}
					else
					{
						tm->mem.push_back(215);
					}
					int max = strlen(str.c_str()) - 1;
					char cc[4] = {};
					*reinterpret_cast<uint *>(cc) = max;
					for (int k = 0; k < 4; ++k)
					{
						tm->mem.push_back(cc[k]);
					}
					for (int k = 1; k < max; ++k)
					{
						tm->mem.push_back(str[k]);
					}
					tm->mem.push_back(0);
					tm->memsiz = tm->mem.size();
					tm->valuetype = 8;
				}
				break;
				}
				tm->valuetype_detail = get_basic_type_with_int(tm->valuetype);
				str.islocal = true;
				return tm;
			}
		}

		// seperate term and operator of expr
		vecarr<sen *> segs; // term
		segs.NULLState();
		segs.Init(2, false);
		sen *vtemp = (sen *)fm->_New(sizeof(sen), true);
		vtemp->NULLState();
		vtemp->Init(2, false);

		sen *temp = nullptr;
		for (int i = 0; i < ten->size(); ++i)
		{
			char c = ten->at(i).data.str[0];
			if (c == '(')
			{
				temp = wbss.oc_search(ten, i, "(", ")");
				i += temp->size();
				segs.push_back(temp);
			}
			else if (c == '[')
			{
				if (vtemp->size() != 0)
				{
					segs.push_back(vtemp);

					vtemp = (sen *)fm->_New(sizeof(sen), true);
					vtemp->NULLState();
					vtemp->Init(2, true);
				}

				vtemp->push_back(ten->at(i));
				segs.push_back(vtemp);

				vtemp = (sen *)fm->_New(sizeof(sen), true);
				vtemp->NULLState();
				vtemp->Init(2, true);

				temp = wbss.oc_search(ten, i, "[", "]");
				temp->pop_back();
				temp->erase(0);

				segs.push_back(temp);

				i += temp->size();
			}
			else if ((('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) || (c == '_' || ('0' <= c && c <= '9')))
			{
				vtemp->push_back(ten->at(i));
			}
			else
			{
				if (vtemp->size() != 0)
				{
					segs.push_back(vtemp);

					vtemp = (sen *)fm->_New(sizeof(sen), true);
					vtemp->NULLState();
					vtemp->Init(2, true);
				}

				vtemp->push_back(ten->at(i));
				vtemp->at(0).type = 'o';
				segs.push_back(vtemp);

				vtemp = (sen *)fm->_New(sizeof(sen), true);
				vtemp->NULLState();
				vtemp->Init(2, false);
			}
		}

		if (vtemp->size() > 0)
		{
			segs.push_back(vtemp);
		}
		else
		{
			vtemp->release();
			fm->_Delete((byte8 *)vtemp, sizeof(sen));
		}

		/*
		   for(int k=0;k<segs.size();++k){ wbss.dbg_sen(segs.at(k)); } */

		for (int k = 0; k < basicoper_max; ++k)
		{
			for (int i = 0; i < segs.size(); ++i)
			{
				sen *seg = segs.at(i);
				if (seg->at(0).type == 'a')
					continue;
				if (seg->size() == 0)
					continue;
				lcstr str;
				str.NULLState();
				str.Init(2, true);
				str = seg->at(0).data.str;
				TBT tbt = DecodeTextBlock(str);
				if (tbt == TBT::_operation)
				{
					if (strcmp(str.Arr, basicoper[k].symbol) == 0)
					{
						if (basicoper[k].mod == 'o')
						{
							if (basicoper[k].endop - basicoper[k].startop >= 11)
							{
								// input num oper
								temp_mem *result_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);
								temp_mem *left_ten = nullptr;
								temp_mem *right_ten = nullptr;
								left_ten = get_asm_from_sen(segs.at(i - 1), true, true);
								right_ten = get_asm_from_sen(segs.at(i + 1), false, true);
								/*
								wbss.dbg_sen(segs.at(i-1));
								cout << basicoper[k].symbol << endl;
								wbss.dbg_sen(segs.at(i+1));
								*/
								int opertype = combine_opertype(left_ten->valuetype,
																right_ten->valuetype);
								int add = 1;
								int leftcast = (int)get_cast_type(left_ten->valuetype, opertype);
								int rightcast = (int)get_cast_type(right_ten->valuetype, opertype);

								if (leftcast != (int)casting_type::nocasting)
									++add;

								if (rightcast != (int)casting_type::nocasting)
									++add;

								result_ten->mem.NULLState();
								result_ten->mem.Init(result_ten->memsiz + 1, false);
								for (int u = 0; u < left_ten->memsiz; ++u)
								{
									result_ten->mem.push_back(left_ten->mem[u]);
								}
								// a casting
								if (leftcast != (int)casting_type::nocasting)
								{
									result_ten->mem.push_back((byte8)201);
									result_ten->mem.push_back((byte8)leftcast);
								}
								for (int u = 0; u < right_ten->memsiz; ++u)
								{
									result_ten->mem.push_back(right_ten->mem[u]);
								}
								// b casting
								if (rightcast != (int)casting_type::nocasting)
								{
									result_ten->mem.push_back((byte8)202);
									result_ten->mem.push_back((byte8)rightcast);
								}

								result_ten->mem.push_back(52);
								result_ten->mem.push_back(218); // POP

								if (is_a)
								{
									int opp = basicoper[k].startop + 2 * opertype;
									result_ten->mem.push_back((byte8)opp);
								}
								else
								{
									int opp = basicoper[k].startop + 2 * opertype + 1;
									result_ten->mem.push_back((byte8)opp);
								}
								result_ten->memsiz = result_ten->mem.size();

								if (k <= 10)
								{
									result_ten->valuetype = opertype;
								}
								else
								{
									result_ten->valuetype = 7; // bool
								}
								result_ten->valuetype_detail =
									get_basic_type_with_int(result_ten->valuetype);

								segs.erase(i + 1);
								segs[i]->at(0).type = 'a'; // asm
								segs[i]->at(0).data.str = reinterpret_cast<char *>(result_ten);
								segs.erase(i - 1);
								--i;
							}
							else if (basicoper[k].endop - basicoper[k].startop == 1)
							{
								if(basicoper[k].startop == 127 || basicoper[k].startop == 129){
									// input num oper
									temp_mem *result_ten =
										(temp_mem *)fm->_New(sizeof(temp_mem), true);
									temp_mem *left_ten = nullptr;
									temp_mem *right_ten = nullptr;
									left_ten = get_asm_from_sen(segs.at(i - 1), true, true);
									right_ten = get_asm_from_sen(segs.at(i + 1), false, true);
									/*
									wbss.dbg_sen(segs.at(i-1));
									cout << basicoper[k].symbol << endl;
									wbss.dbg_sen(segs.at(i+1));
									*/
									int opertype = combine_opertype(left_ten->valuetype,
																	right_ten->valuetype);
									int add = 1;
									int leftcast = (int)get_cast_type(left_ten->valuetype, opertype);
									int rightcast = (int)get_cast_type(right_ten->valuetype, opertype);

									if (leftcast != (int)casting_type::nocasting)
										++add;

									if (rightcast != (int)casting_type::nocasting)
										++add;

									result_ten->mem.NULLState();
									result_ten->mem.Init(result_ten->memsiz + 1, false);
									for (int u = 0; u < left_ten->memsiz; ++u)
									{
										result_ten->mem.push_back(left_ten->mem[u]);
									}
									// a casting
									if (leftcast != (int)casting_type::nocasting)
									{
										result_ten->mem.push_back((byte8)201);
										result_ten->mem.push_back((byte8)leftcast);
									}
									for (int u = 0; u < right_ten->memsiz; ++u)
									{
										result_ten->mem.push_back(right_ten->mem[u]);
									}
									// b casting
									if (rightcast != (int)casting_type::nocasting)
									{
										result_ten->mem.push_back((byte8)202);
										result_ten->mem.push_back((byte8)rightcast);
									}

									result_ten->mem.push_back(52);
									result_ten->mem.push_back(218); // POP

									if (is_a)
									{
										int opp = basicoper[k].startop;
										result_ten->mem.push_back((byte8)opp);
									}
									else
									{
										int opp = basicoper[k].startop;
										result_ten->mem.push_back((byte8)opp);
									}
									result_ten->memsiz = result_ten->mem.size();

									if (k <= 10)
									{
										result_ten->valuetype = opertype;
									}
									else
									{
										result_ten->valuetype = 7; // bool
									}
									result_ten->valuetype_detail =
										get_basic_type_with_int(result_ten->valuetype);

									segs.erase(i + 1);
									segs[i]->at(0).type = 'a'; // asm
									segs[i]->at(0).data.str = reinterpret_cast<char *>(result_ten);
									segs.erase(i - 1);
									--i;
								}
								else if (k == 16)
								{
									
									// ! not
									temp_mem *result_ten =
										(temp_mem *)fm->_New(sizeof(temp_mem), true);
									temp_mem *right_ten = nullptr;
									right_ten = get_asm_from_sen(segs.at(i + 1), true, true);
									int add = 1;
									result_ten->memsiz = right_ten->memsiz + add + 1;
									result_ten->mem.NULLState();
									result_ten->mem.Init(result_ten->memsiz + 1, false);
									for (int u = 0; u < right_ten->memsiz; ++u)
									{
										result_ten->mem.push_back(right_ten->mem[u]);
									}

									result_ten->mem.push_back(52); // AXBY
									result_ten->mem.push_back(216); // POP A

									if (is_a)
									{
										int opp = basicoper[k].startop; // A = !X
										result_ten->mem.push_back((byte8)opp);
									}
									else
									{
										int opp = basicoper[k].startop + 1; // B = !X
										result_ten->mem.push_back((byte8)opp);
									}
									result_ten->memsiz = result_ten->mem.size();

									result_ten->valuetype = 7; // bool
									result_ten->valuetype_detail =
										get_basic_type_with_int(result_ten->valuetype);

									segs.erase(i + 1);
									segs[i]->at(0).type = 'a'; // asm
									segs[i]->at(0).data.str =
										reinterpret_cast<char *>(result_ten);
								}
								else
								{
									// && and ||
									temp_mem *result_ten =
										(temp_mem *)fm->_New(sizeof(temp_mem), true);
									temp_mem *left_ten = nullptr;
									temp_mem *right_ten = nullptr;
									left_ten = get_asm_from_sen(segs.at(i - 1), true, true);
									right_ten = get_asm_from_sen(segs.at(i + 1), false, true);
									result_ten->memsiz = left_ten->memsiz + right_ten->memsiz + 2;
									result_ten->mem.NULLState();
									result_ten->mem.Init(result_ten->memsiz + 1, false);
									for (int u = 0; u < left_ten->memsiz; ++u)
									{
										result_ten->mem.push_back(left_ten->mem[u]);
									}
									for (int u = 0; u < right_ten->memsiz; ++u)
									{
										result_ten->mem.push_back(right_ten->mem[u]);
									}

									result_ten->mem.push_back(52);
									result_ten->mem.push_back(218); // POP

									if (is_a)
									{
										int opp = basicoper[k].startop;
										result_ten->mem.push_back((byte8)opp);
									}
									else
									{
										int opp = basicoper[k].startop + 1;
										result_ten->mem.push_back((byte8)opp);
									}

									result_ten->memsiz = result_ten->mem.size();

									result_ten->valuetype = 7; // bool
									result_ten->valuetype_detail =
										get_basic_type_with_int(result_ten->valuetype);

									segs.erase(i + 1);

									segs[i]->at(0).type = 'a'; // asm
									segs[i]->at(0).data.str =
										reinterpret_cast<char *>(result_ten);
									segs.erase(i - 1);
									--i;
								}
							}
						}
						else
						{
							// ptr oper
							switch (basicoper[k].symbol[0])
							{
							case '[':
							{
								//there is two arr[i] operation. 
								//case 1. type is array : it return value of (self's address + i) address.
								//case 2. type is pointer : it return value of (self's value + i) address.

								temp_mem *result_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);
								temp_mem *left_ten = nullptr;
								temp_mem *right_ten = nullptr;

								bool is_array_type = false;
								if(segs[i-1]->at(0).type == 'a'){
									//if asm
									type_data* imtd = reinterpret_cast<type_data*>(reinterpret_cast<temp_mem*>(segs[i-1]->at(0).data.str)->valuetype_detail->structptr);
									is_array_type = imtd->typetype == 'a';
								}
								type_data* ltd = get_type_with_vname(segs[i-1]->at(0).data.str);
								if(ltd == nullptr){
									ltd = get_type_with_global_vname(segs[i-1]->at(0).data.str);
								}

								if(ltd == nullptr){
									is_array_type |= false;
								}
								else{
									is_array_type |= segs[i-1]->size() == 1 && ltd->typetype == 'a';
								}
								
								if(is_array_type){
									//case 1 : array type
									left_ten = get_asm_from_sen(segs.at(i - 1), true, false);
								}
								else{
									//case 2 : pointer type
									left_ten = get_asm_from_sen(segs.at(i - 1), true, true);
								}
								right_ten = get_asm_from_sen(segs.at(i + 1), true, true);
								
								result_ten->memsiz = left_ten->memsiz + right_ten->memsiz + 9;
								result_ten->mem.NULLState();
								result_ten->mem.Init(result_ten->memsiz + 1, false);

								for (int u = 0; u < right_ten->memsiz; ++u)
								{
									result_ten->mem.push_back(right_ten->mem[u]);
								}

								type_data *td = get_sub_type(left_ten->valuetype_detail);
								type_data *std = nullptr;
								if(is_array_type){
									std = get_sub_type(td);
								}
								else{
									std = nullptr;
								}
								//type_data *std = get_sub_type(td);
								result_ten->mem.push_back(27); // b=const4
								char cc[4] = {};
								if(is_array_type){
									*reinterpret_cast<uint *>(cc) = std->typesiz;
								}
								else{
									*reinterpret_cast<uint *>(cc) = td->typesiz;
								}
								for (int u = 0; u < 4; ++u)
									result_ten->mem.push_back(cc[u]);

								result_ten->mem.push_back(52);	// axby
								result_ten->mem.push_back(218); // POP
								// b=x*y uint
								result_ten->mem.push_back(92);

								for (int u = 0; u < left_ten->memsiz; ++u)
								{
									result_ten->mem.push_back(left_ten->mem[u]);
								}

								result_ten->mem.push_back(52);	// axby
								result_ten->mem.push_back(218); // POP

								if (is_a)
								{
									int opp = 61;
									result_ten->mem.push_back((byte8)opp);
									
									if (isvalue)
									{
										if(is_array_type){
											result_ten->valuetype = get_int_with_basictype(std);
										}
										else{
											result_ten->valuetype = get_int_with_basictype(td);
										}
										
										if (result_ten->valuetype == 8)
										{
											if(is_array_type){
												result_ten->valuetype_detail = get_addpointer_type(std);
											}
											else{
												result_ten->valuetype_detail = get_addpointer_type(td);
											}
										}
										else
										{
											if(is_array_type){
												result_ten->valuetype_detail = std;
											}
											else{
												result_ten->valuetype_detail = td;
											}
											result_ten->mem.push_back(205);
										}
									}
									else
									{
										result_ten->valuetype = 8;
										if(is_array_type){
											result_ten->valuetype_detail =
											get_addpointer_type(std);
										}
										else{
											result_ten->valuetype_detail =
											get_addpointer_type(td);
										}
									}

									result_ten->memsiz = result_ten->mem.size();
								}
								else
								{
									int opp = 62;
									result_ten->mem.push_back((byte8)opp);
									
									if (isvalue)
									{
										result_ten->valuetype = get_int_with_basictype(td);
										if (result_ten->valuetype == 8)
										{
											if(is_array_type){
												result_ten->valuetype_detail =
												get_addpointer_type(std);
											}
											else{
												result_ten->valuetype_detail =
												get_addpointer_type(td);
											}
										}
										else
										{
											if(is_array_type){
												result_ten->valuetype_detail =
												get_addpointer_type(std);
											}
											else{
												result_ten->valuetype_detail =
												get_addpointer_type(td);
											}
											result_ten->mem.push_back(206);
										}
									}
									else
									{
										result_ten->valuetype = 8;
										if(is_array_type){
											result_ten->valuetype_detail =
											get_addpointer_type(std);
										}
										else{
											result_ten->valuetype_detail =
											get_addpointer_type(td);
										}
									}

									result_ten->memsiz = result_ten->mem.size();
								}

								segs.erase(i + 1);
								segs.erase(i + 1);

								segs[i]->at(0).type = 'a'; // asm
								segs[i]->at(0).data.str =
									reinterpret_cast<char *>(result_ten);
								segs.erase(i - 1);
								--i;
								result_ten->memsiz = result_ten->mem.size();
							}
							break;
							case '.':
							{
								temp_mem *result_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);

								temp_mem *left_ten = nullptr;
								left_ten = get_asm_from_sen(segs.at(i - 1), true, false);
								type_data *td;
								if (segs.at(i + 1)->size() == 1 && segs.at(i + 1)->at(0).type == 'w')
								{
									td = get_type_with_vname(segs.at(i + 1)->at(0).data.str);
									// a.b
									uint bid = 0;
									bool perfect = false;
									type_data *returntype = nullptr;

									if (td == nullptr)
										break;
									if (td->typetype != 's')
										break;
									struct_data *sptr =
										reinterpret_cast<struct_data *>(td->structptr);
									int su = 0;
									for (int u = 0; u < sptr->member_data.size(); ++u)
									{
										if (sptr->member_data[u].name ==
											segs.at(i + 1)->at(0).data.str)
										{
											bid = sptr->member_data[u].add_address;
											su = u;
											returntype = sptr->member_data[u].td;
											perfect = true;
											break;
										}
									}

									if (perfect)
									{
										result_ten->memsiz = left_ten->memsiz + 7;
										result_ten->mem.NULLState();
										result_ten->mem.Init(2, false);
										for (int u = 0; u < left_ten->memsiz; ++u)
										{
											result_ten->mem.push_back(left_ten->mem[u]);
										}
										// axby
										result_ten->mem.push_back(52);
										result_ten->mem.push_back(218); // POP
										// set y const4
										result_ten->mem.push_back(45);

										char cc[4] = {};
										*reinterpret_cast<uint *>(cc) = bid;
										for (int u = 0; u < 4; ++u)
											result_ten->mem.push_back(cc[u]);

										if (is_a)
										{
											// a=x+y uint
											result_ten->mem.push_back(61);
										}
										else
										{
											// b=x+y uint
											result_ten->mem.push_back(62);
										}

										// ptr
										result_ten->valuetype = 8;
										result_ten->valuetype_detail =
											sptr->member_data[su].td;

										segs.erase(i + 1);
										segs[i]->at(0).type = 'a'; // asm
										segs[i]->at(0).data.str =
											reinterpret_cast<char *>(result_ten);
										segs.erase(i - 1);
										--i;
									}
								}
							}
							break;
							case '-':
							{
								temp_mem *result_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);
								temp_mem *left_ten = nullptr;
								left_ten = get_asm_from_sen(segs.at(i - 1), true, false);
								type_data *td;
								if (segs.at(i + 1)->size() == 1 && segs.at(i + 1)->at(0).type == 'w')
								{
									td = get_type_with_vname(segs.at(i + 1)->at(0).data.str);
									// a.b
									uint bid = 0;
									bool perfect = false;
									type_data *returntype = nullptr;

									if (td == nullptr)
										break;
									if (td->typetype != 's')
										break;
									struct_data *sptr =
										reinterpret_cast<struct_data *>(td->structptr);
									int su = 0;
									for (int u = 0; u < sptr->member_data.size(); ++u)
									{
										if (sptr->member_data[u].name ==
											segs.at(i + 1)->at(0).data.str)
										{
											bid = sptr->member_data[u].add_address;
											su = u;
											returntype = sptr->member_data[u].td;
											perfect = true;
											break;
										}
									}

									if (perfect)
									{
										result_ten->memsiz = left_ten->memsiz + 8;
										result_ten->mem.NULLState();
										result_ten->mem.Init(2, false);
										for (int u = 0; u < left_ten->memsiz; ++u)
										{
											result_ten->mem.push_back(left_ten->mem[u]);
										}
										// a=value(a)
										result_ten->mem.push_back(205);
										// axby
										result_ten->mem.push_back(52);
										result_ten->mem.push_back(218); // POP
										// set y const4
										result_ten->mem.push_back(45);

										char cc[4] = {};
										*reinterpret_cast<uint *>(cc) = bid;
										for (int u = 0; u < 4; ++u)
											result_ten->mem.push_back(cc[u]);

										if (is_a)
										{
											// a=x+y uint
											result_ten->mem.push_back(61);
										}
										else
										{
											// b=x+y uint
											result_ten->mem.push_back(62);
										}

										// ptr
										result_ten->valuetype = 8;
										result_ten->valuetype_detail =
											sptr->member_data[su].td;

										segs.erase(i + 1);

										segs[i]->at(0).type = 'a'; // asm
										segs[i]->at(0).data.str =
											reinterpret_cast<char *>(result_ten);
										segs.erase(i - 1);

										--i;
									}
								}
							}
							break;
							case '&':
							{
								temp_mem *result_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);
								temp_mem *right_ten = nullptr;
								if (is_a)
								{
									right_ten = get_asm_from_sen(segs.at(i + 1), true, false);
								}
								else
								{
									right_ten = get_asm_from_sen(segs.at(i + 1), false, false);
								}

								result_ten->mem.NULLState();
								result_ten->mem.Init(2, false);
								result_ten->memsiz = right_ten->memsiz;
								for (int u = 0; u < right_ten->memsiz; ++u)
								{
									result_ten->mem.push_back(right_ten->mem[u]);
								}

								// ptr
								result_ten->valuetype = 8;
								result_ten->valuetype_detail = right_ten->valuetype_detail;
								segs.erase(i + 1);
								segs[i]->at(0).type = 'a'; // asm
								segs[i]->at(0).data.str =
									reinterpret_cast<char *>(result_ten);
							}
							break;
							case '*':
							{
								if (segs.at(i - 1)->at(0).type != 'o')
									break;
								temp_mem *result_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);
								temp_mem *right_ten =
									(temp_mem *)fm->_New(sizeof(temp_mem), true);
								type_data *td = get_sub_type(right_ten->valuetype_detail);

								if (is_a)
								{
									right_ten = get_asm_from_sen(segs.at(i + 1), true, true);
								}
								else
								{
									right_ten = get_asm_from_sen(segs.at(i + 1), false, true);
								}
								result_ten->memsiz = right_ten->memsiz + 1;
								result_ten->mem.NULLState();
								result_ten->mem.Init(2, false);
								for (int u = 0; u < right_ten->memsiz; ++u)
								{
									result_ten->mem.push_back(right_ten->mem[u]);
								}
								if (is_a)
								{
									result_ten->mem.push_back(205);
								}
								else
								{
									result_ten->mem.push_back(206);
								}

								// ptr
								result_ten->valuetype = 8;
								result_ten->valuetype_detail = td;
								segs.erase(i + 1);
								segs[i]->at(0).type = 'a'; // asm
								segs[i]->at(0).data.str =
									reinterpret_cast<char *>(result_ten);
							}
							break;
							}
						}
					}
				}
			}
		}

		if (segs.size() == 1)
		{
			tm = reinterpret_cast<temp_mem *>(segs[0]->at(0).data.str);
			return tm;
		}

		return nullptr;
	}

	void compile_addVariable(code_sen *cs)
	{
		sen *code = get_sen_from_codesen(cs);
		wbss.dbg_sen(code);
		int loc = code->up - 1;
		char *variable_name = code->at(loc).data.str;
		sen *type_name = wbss.sen_cut(code, 0, loc - 1);
		// cout << variable_name << endl;
		// wbss.dbg_sen(type_name);
		if (blockstack.size() == 0)
		{
			// global variable
			NamingData *nd = (NamingData *)fm->_New(sizeof(NamingData), true);
			nd->name = (char *)fm->_New(strlen(variable_name) + 1, true);
			strcpy(nd->name, variable_name);
			nd->td = get_type_with_namesen(type_name);
			if (globalVariables.size() == 0)
			{
				nd->add_address = 0;
			}
			else
			{
				nd->add_address =
					globalVariables.last()->add_address + globalVariables.last()->td->typesiz;
			}
			globalVariables.push_back(nd);
		}
		else
		{
			bool add = false;
			if (loc - 1 == 0)
			{
				// basic or struct type
				char *typen = type_name->at(0).data.str;
				for (int id = 0; id < basictype_max; ++id)
				{
					if (strcmp(typen, basictype[id]->name.c_str()) == 0)
					{
						mem[writeup] = 0;
						++writeup;

						int spup = 0;
						int kst = 0;
						for (int k = blockstack.size() - 1; k >= 0; --k)
						{
							if (blockstack.at(k)->bs == blockstate::bs_function)
							{
								kst = k;
							}
						}
						for (int k = kst; k < blockstack.size(); ++k)
						{
							spup += blockstack.at(k)->add_address_up;
						}
						spup += basictype[id]->typesiz;
						*reinterpret_cast<int *>(&mem[writeup]) = spup;
						writeup += 4;

						NamingData nd;
						nd.name = (char *)fm->_New(strlen(variable_name) + 1, true);
						strcpy(nd.name, variable_name);
						nd.td = basictype[id];
						nd.add_address =
							blockstack.last()->add_address_up + basictype[id]->typesiz;
						blockstack.last()->add_address_up += basictype[id]->typesiz;

						blockstack.last()->variable_data.push_back(nd);
						add = true;
						break;
					}
				}

				if (add == false)
				{
					// struct
					for (int i = basictype_max; i < types.size(); ++i)
					{
						if (strcmp(typen, types.at(i)->name.c_str()) == 0)
						{
							mem[writeup] = 0;
							++writeup;
							*reinterpret_cast<int *>(&mem[writeup]) = types.at(i)->typesiz;
							writeup += 4;

							NamingData nd;
							nd.name = (char *)fm->_New(strlen(variable_name) + 1, true);
							strcpy(nd.name, variable_name);
							nd.td = types.at(i);
							nd.add_address = blockstack.last()->add_address_up;
							blockstack.last()->add_address_up += types.at(i)->typesiz;
							blockstack.last()->variable_data.push_back(nd);
							add = true;
							break;
						}
					}
				}
			}
			else
			{
				// combine type
			}
		}

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		type_name->release();
		fm->_Delete((byte8 *)type_name, sizeof(sen));
	}

	void compile_setVariable(code_sen *cs)
	{
		// dbg_codesen(cs);
		sen *code = get_sen_from_codesen(cs);
		int loc = wbss.search_word_first(0, code, "=");

		if (loc < 0)
		{
			// += oper
			loc = wbss.search_word_first_cd(0, code, cd_eqoper);
			char operc = code->at(loc).data.str[0];

			sen *left_expr = wbss.sen_cut(code, 0, loc - 1);
			sen *right_expr = wbss.sen_cut(code, loc + 1, code->size() - 1);

			temp_mem *left_tm_ptr = get_asm_from_sen(left_expr, true, false);
			temp_mem *left_tm_v = get_asm_from_sen(left_expr, true, true);
			temp_mem *right_tm = get_asm_from_sen(right_expr, false, true);

			bool need_casting = false;
			casting_type castt;
			type_data *lstd = get_sub_type(left_tm_ptr->valuetype_detail);
			int ltype = get_int_with_basictype(lstd);
			if (ltype != right_tm->valuetype)
			{
				castt = get_cast_type(right_tm->valuetype, ltype);
				if (castt != casting_type::nocasting)
				{
					need_casting = true;
				}
			}

			for (int i = 0; i < right_tm->memsiz; ++i)
			{
				mem[writeup++] = right_tm->mem[i];
			}
			if (need_casting)
			{
				mem[writeup++] = (byte8)201;
				mem[writeup++] = (byte8)castt;
			}
			for (int i = 0; i < left_tm_v->memsiz; ++i)
			{
				mem[writeup++] = left_tm_v->mem[i];
			}
			mem[writeup++] = 52;

			for (int k = 0; k < basicoper_max; ++k)
			{
				if (basicoper[k].symbol[0] == operc)
				{
					mem[writeup++] = basicoper[k].startop + ltype * 2;
					break;
				}
			}

			for (int i = 0; i < left_tm_ptr->memsiz; ++i)
			{
				mem[writeup++] = left_tm_ptr->mem[i];
			}
			mem[writeup++] = (byte8)199;

			switch (lstd->typesiz)
			{
			case 1:
				mem[writeup++] = (byte8)211;
				break;
			case 2:
				mem[writeup++] = (byte8)212;
				break;
			case 4:
				mem[writeup++] = (byte8)213;
				break;
			default:
				cout << "typesiz is more than 4." << endl;
				break;
			}

			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));

			left_expr->release();
			fm->_Delete((byte8 *)left_expr, sizeof(sen));

			right_expr->release();
			fm->_Delete((byte8 *)right_expr, sizeof(sen));

			release_tempmem(left_tm_ptr);
			release_tempmem(left_tm_v);
			release_tempmem(right_tm);
		}
		else
		{
			sen *left_expr = wbss.sen_cut(code, 0, loc - 1);
			sen *right_expr = wbss.sen_cut(code, loc + 1, code->size() - 1);

			wbss.dbg_sen(left_expr);
			// wbss.dbg_sen(right_expr);

			temp_mem *left_tm = get_asm_from_sen(left_expr, true, false);
			temp_mem *right_tm = get_asm_from_sen(right_expr, true, true);

			bool need_casting = false;
			casting_type castt;
			type_data *lstd = get_sub_type(left_tm->valuetype_detail);
			int ltype = get_int_with_basictype(lstd);
			if (ltype != right_tm->valuetype)
			{
				castt = get_cast_type(right_tm->valuetype, ltype);
				if (castt != casting_type::nocasting)
				{
					need_casting = true;
				}
			}
			
			for (int i = 0; i < right_tm->memsiz; ++i)
			{
				mem[writeup++] = right_tm->mem[i];
			}
			if (need_casting)
			{
				mem[writeup++] = (byte8)201;
				mem[writeup++] = (byte8)castt;
			}

			for (int i = 0; i < left_tm->memsiz; ++i)
			{
				mem[writeup++] = left_tm->mem[i];
			}
			mem[writeup++] = (byte8)199;

			switch (lstd->typesiz)
			{
			case 1:
				mem[writeup++] = (byte8)211;
				break;
			case 2:
				mem[writeup++] = (byte8)212;
				break;
			case 4:
				mem[writeup++] = (byte8)213;
				break;
			default:
				cout << "typesiz is more than 4." << endl;
				break;
			}

			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));

			left_expr->release();
			fm->_Delete((byte8 *)left_expr, sizeof(sen));

			right_expr->release();
			fm->_Delete((byte8 *)right_expr, sizeof(sen));

			release_tempmem(left_tm);
			release_tempmem(right_tm);
		}
	}

	void compile_if(code_sen *cs)
	{
		sen *code = get_sen_from_codesen(cs);
		wbss.dbg_sen(code);
		int loc = wbss.search_word_first(0, code, "if");
		if (loc == -1){
			//else
			nextbd.bs = blockstate::bs_else;
			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));
			return;
		}
			
		sen *inner_expr = wbss.oc_search(code, loc, "(", ")");
		// wbss.dbg_sen(inner_expr);
		inner_expr->pop_back();
		inner_expr->erase(0);
		temp_mem *inner_tm = get_asm_from_sen(inner_expr, true, true);
		for (int i = 0; i < inner_tm->memsiz; ++i)
		{
			mem[writeup++] = inner_tm->mem[i];
		}
		mem[writeup++] = 187;
		nextbd.bs = blockstate::bs_if;
		nextbd.parameter[0] = writeup;
		writeup += 4;

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		inner_expr->release();
		fm->_Delete((byte8 *)inner_expr, sizeof(sen));

		release_tempmem(inner_tm);
	}

	void compile_while(code_sen *cs)
	{
		// fm->dbg_fm1_lifecheck();
		sen *code = get_sen_from_codesen(cs);
		// fm->dbg_fm1_lifecheck();
		wbss.dbg_sen(code);
		int loc = wbss.search_word_first(0, code, "while");
		sen *inner_expr = wbss.oc_search(code, loc, "(", ")");

		inner_expr->pop_back();
		inner_expr->erase(0);
		wbss.dbg_sen(inner_expr);
		int save = writeup;
		temp_mem *inner_tm = get_asm_from_sen(inner_expr, true, true);
		for (int i = 0; i < inner_tm->memsiz; ++i)
		{
			mem[writeup++] = inner_tm->mem[i];
		}
		mem[writeup++] = 187;
		nextbd.bs = blockstate::bs_while;
		nextbd.parameter[0] = writeup;
		nextbd.parameter[1] = save;
		nextbd.breakpoints = (vecarr<int> *)fm->_New(sizeof(vecarr<int>), true);
		nextbd.breakpoints->NULLState();
		nextbd.breakpoints->Init(2, false);

		nextbd.continuepoints = (vecarr<int> *)fm->_New(sizeof(vecarr<int>), true);
		nextbd.continuepoints->NULLState();
		nextbd.continuepoints->Init(2, false);

		writeup += 4;
		if ((int *)code == (int *)nextbd.breakpoints)
		{
			cout << "error" << endl;
		}

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		inner_expr->release();
		fm->_Delete((byte8 *)inner_expr, sizeof(sen));

		release_tempmem(inner_tm);
	}

	void compile_block(code_sen *cs)
	{
		if (nextbd.bs == blockstate::bs_struct)
		{
			nextsd->member_data.NULLState();
			nextsd->member_data.Init(2, false);
			int addadd = 0;
			for (int i = 0; i < cs->codeblocks->size(); ++i)
			{
				code_sen *css = reinterpret_cast<code_sen *>(cs->codeblocks->at(i));
				if (css->ck == codeKind::ck_addVariable)
				{
					NamingData nd;
					nd.name = css->sen[css->maxlen - 1];
					sen *typesen = (sen *)fm->_New(sizeof(sen), true);
					typesen->NULLState();
					typesen->Init(2, false);
					for (int k = 0; k < css->maxlen - 1; ++k)
					{
						segment seg;
						seg.data.str = css->sen[k];
						seg.type = 'w';
						typesen->push_back(seg);
					}
					nd.td = get_type_with_namesen(typesen);
					nd.add_address = addadd;
					addadd += nd.td->typesiz;
					nextsd->member_data.push_back(nd);

					typesen->release();
					fm->_Delete((byte8 *)typesen, sizeof(sen));
				}
			}

			type_data *td = (type_data *)fm->_New(sizeof(type_data), true);
			td->structptr = reinterpret_cast<int *>(nextsd);
			td->name = nextsd->name.c_str();
			td->typetype = 's';
			td->typesiz = addadd;
			types.push_back(td);
		}
		else
		{
			block_data *bd = (block_data *)fm->_New(sizeof(block_data), true);
			bd->add_address_up = 0;
			bd->start_pc = &mem[writeup];
			bd->variable_data.NULLState();
			bd->variable_data.Init(2, false);
			bd->bs = nextbd.bs;
			if (bd->bs == blockstate::bs_while)
			{
				bd->breakpoints = nextbd.breakpoints;
				bd->continuepoints = nextbd.continuepoints;
			}
			else
			{
				bd->breakpoints = nullptr;
				bd->continuepoints = nullptr;
			}

			for (int i = 0; i < 5; ++i)
			{
				bd->parameter[i] = nextbd.parameter[i];
			}
			blockstack.push_back(bd);

			if (blockstack.last()->bs == blockstate::bs_function)
			{
				func_data *fd = reinterpret_cast<func_data *>(nextbd.parameter[0]);
				fd->start_pc = bd->start_pc;

				for (int k = 0; k < fd->param_data.size(); ++k)
				{
					bd->variable_data.push_back(fd->param_data[k]);
				}

				if (strcmp(fd->name.c_str(), "main") == 0)
				{
					*reinterpret_cast<uint *>(&mem[2]) = (uint)(fd->start_pc - mem);
				}

				if (fd->param_data.size() != 0)
				{
					bd->add_address_up =
						fd->param_data.last().add_address + fd->param_data.last().td->typesiz;
				}
				else
				{
					bd->add_address_up = 0;
				}

				functions.push_back(fd);
			}

			for (int i = 0; i < cs->codeblocks->size(); ++i)
			{
				code_sen *css = reinterpret_cast<code_sen *>(cs->codeblocks->at(i));
				dbg_codesen(css);
				if (css->ck == codeKind::ck_if)
				{
					vecarr<int> ifptr_arr;
					ifptr_arr.NULLState();
					ifptr_arr.Init(2, true);
					int ifi = i + 2;
					int stack = 0;
					if (cs->codeblocks->size() <= i + 2)
					{
						current_if_is_multiple = false;
						compile_code(css);
						continue;
					}
					code_sen *css2 = reinterpret_cast<code_sen *>(cs->codeblocks->at(i + 2));
					while (css2 != nullptr && css2->ck == codeKind::ck_if)
					{
						stack += 1;
						css2 = reinterpret_cast<code_sen *>(cs->codeblocks->at(ifi + 2));
						ifi += 2;
						if (css2 == nullptr)
						{
							break;
						}
					}
					if (ifi != i + 2)
					{
						ifi -= 1;
					}

					//compile if
					if (stack == 0)
					{
						current_if_is_multiple = false;
						compile_code(css);
						continue;
					}
					else
					{
						current_if_is_multiple = true;
						// else if if
						compile_code(css);
						css2 = reinterpret_cast<code_sen *>(cs->codeblocks->at(i + 1));
						uint address = nextbd.parameter[0];
						compile_code(css2);
						mem[writeup++] = 188;
						ifptr_arr.push_back(writeup);
						writeup += 4;

						*reinterpret_cast<uint *>(&mem[address]) = (uint)writeup;

						int ifk = i + 2;
						while (ifk + 1 <= ifi)
						{
							css2 = reinterpret_cast<code_sen *>(cs->codeblocks->at(ifk));
							if (css2->maxlen == 1 && strcmp(css2->sen[0], "else") == 0)
							{
								// else
								compile_code(css2);
								css2 = reinterpret_cast<code_sen *>(cs->codeblocks->at(ifk + 1));
								compile_code(css2);
								ifk += 2;
								continue;
							}
							else
							{
								compile_code(css2);
								address = nextbd.parameter[0];
							}

							css2 = reinterpret_cast<code_sen *>(cs->codeblocks->at(ifk + 1));
							compile_code(css2);
							mem[writeup++] = 188;
							ifptr_arr.push_back(writeup);
							writeup += 4;
							*reinterpret_cast<uint *>(&mem[address]) = (uint)writeup;
							ifk += 2;
						}

						for (int k = 0; k < ifptr_arr.size(); ++k)
						{
							*reinterpret_cast<uint *>(&mem[ifptr_arr[k]]) = (uint)writeup;
						}

						i = ifi;
					}
				}
				else
				{
					compile_code(css);
				}
			}

			switch (blockstack.last()->bs)
			{
			case blockstate::bs_if:
				// case 1 : if only - writeup;
				// case 2 : if / else or more - writeup+5;
				if(current_if_is_multiple){
					*reinterpret_cast<uint *>(&mem[blockstack.last()->parameter[0]]) =
					(uint)writeup + 5;
				}
				else{
					*reinterpret_cast<uint *>(&mem[blockstack.last()->parameter[0]]) =
					(uint)writeup;
				}
				break;
			case blockstate::bs_while:
			{
				*reinterpret_cast<uint *>(&mem[blockstack.last()->parameter[0]]) =
					(uint)writeup + 5;
				for (int i = 0; i < blockstack.last()->breakpoints->size(); ++i)
				{
					*reinterpret_cast<uint *>(&mem[blockstack.last()->breakpoints->at(i)]) =
						(uint)writeup + 5;
				}

				for (int i = 0; i < blockstack.last()->continuepoints->size(); ++i)
				{
					*reinterpret_cast<
						uint *>(&mem[blockstack.last()->continuepoints->at(i)]) =
						(uint)blockstack.last()->parameter[1];
				}

				mem[writeup++] = 188; // jmp
				*reinterpret_cast<uint *>(&mem[writeup]) =
					(uint)blockstack.last()->parameter[1];
				writeup += 4;
			}
			break;
			case blockstate::bs_function:
			{
				mem[writeup++] = 193; // return
			}
			break;
			}

			if (bd->breakpoints != nullptr)
			{
				bd->breakpoints->release();
				fm->_Delete((byte8 *)bd->breakpoints, sizeof(vecarr<int>));
			}
			bd->variable_data.release();
			if (bd->continuepoints != nullptr)
			{
				bd->continuepoints->release();
				fm->_Delete((byte8 *)bd->continuepoints, sizeof(vecarr<int>));
			}
			fm->_Delete((byte8 *)bd, sizeof(block_data));
			blockstack.pop_back();
		}
	}

	void compile_addFunction(code_sen *cs)
	{
		func_data *fd = (func_data *)fm->_New(sizeof(func_data), true);
		fd->name.NULLState();
		fd->name.Init(2, false);

		sen *code = get_sen_from_codesen(cs);
		int loc = code->size() - 1;
		sen *inner_params = wbss.oc_search_inv(code, loc, "(", ")");
		int nameloc = loc - inner_params->size();

		fd->name = code->at(nameloc).data.str;

		sen *typen = wbss.sen_cut(code, 0, nameloc - 1);
		sen *params_sen = wbss.sen_cut(code, nameloc + 2, loc - 1);
		wbss.dbg_sen(params_sen);
		int coma = wbss.search_word_first(0, params_sen, ",");
		int savecoma = 0;
		int last = params_sen->size() - 1;

		fd->param_data.NULLState();
		fd->param_data.Init(2, false);

		int addadd = 0;

		if (last < 0)
		{
			nextbd.bs = blockstate::bs_function;
			nextbd.parameter[0] = reinterpret_cast<uint64_t>(fd);

			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));

			params_sen->release();
			fm->_Delete((byte8 *)params_sen, sizeof(sen));
			return;
		}

		while (coma != -1)
		{
			sen *param_sen = wbss.sen_cut(params_sen, savecoma, coma - 1);
			NamingData nd;

			sen *typestr = (sen *)fm->_New(sizeof(sen), true);
			typestr->NULLState();
			typestr->Init(2, false);
			for (int i = 0; i < param_sen->size() - 1; ++i)
			{
				typestr->push_back(param_sen->at(i));
			}

			nd.td = get_type_with_namesen(typestr);
			nd.name = param_sen->last().data.str;

			addadd += nd.td->typesiz; //
			nd.add_address = addadd;

			fd->param_data.push_back(nd);
			savecoma = coma + 1;
			coma = wbss.search_word_first(savecoma, params_sen, ",");

			param_sen->release();
			fm->_Delete((byte8 *)param_sen, sizeof(sen));

			typestr->release();
			fm->_Delete((byte8 *)typestr, sizeof(sen));
		}

		sen *param_sen = wbss.sen_cut(params_sen, savecoma, last);
		wbss.dbg_sen(param_sen);
		NamingData nd;

		sen *typestr = (sen *)fm->_New(sizeof(sen), true);
		typestr->NULLState();
		typestr->Init(2, false);
		for (int i = 0; i < param_sen->size() - 1; ++i)
		{
			typestr->push_back(param_sen->at(i));
		}

		wbss.dbg_sen(typestr);
		nd.td = get_type_with_namesen(typestr);
		nd.name = param_sen->last().data.str;

		addadd += nd.td->typesiz;
		nd.add_address = addadd;

		fd->param_data.push_back(nd);
		savecoma = coma + 1;

		nextbd.bs = blockstate::bs_function;
		nextbd.parameter[0] = reinterpret_cast<uint64_t>(fd);

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		param_sen->release();
		fm->_Delete((byte8 *)param_sen, sizeof(sen));

		typestr->release();
		fm->_Delete((byte8 *)typestr, sizeof(sen));
	}

	void compile_useFunction(code_sen *cs)
	{
		func_data *fd = nullptr;

		sen *code = get_sen_from_codesen(cs);
		int loc = code->size() - 1;
		sen *inner_params = wbss.oc_search_inv(code, loc, "(", ")");
		int nameloc = loc - inner_params->size();
		char *funcname = code->at(nameloc).data.str;

		if (strcmp(funcname, "dbg") == 0)
		{
			inner_params->pop_back();
			inner_params->erase(0);
			temp_mem *tm = get_asm_from_sen(inner_params, true, true);
			for (int k = 0; k < tm->memsiz; ++k)
			{
				mem[writeup++] = tm->mem[k];
			}
			mem[writeup++] = 209;
			mem[writeup++] = (byte8)tm->valuetype;

			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));

			inner_params->release();
			fm->_Delete((byte8 *)inner_params, sizeof(sen));
			return;
		}
		else if (strcmp(funcname, "inp") == 0)
		{
			inner_params->pop_back();
			inner_params->erase(0);
			wbss.dbg_sen(inner_params);
			temp_mem *tm = get_asm_from_sen(inner_params, true, true);
			for (int k = 0; k < tm->memsiz; ++k)
			{
				mem[writeup++] = tm->mem[k];
			}
			mem[writeup++] = 210;

			type_data* std = get_sub_type(tm->valuetype_detail);
			mem[writeup++] = (byte8)get_int_with_basictype(std);

			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));

			inner_params->release();
			fm->_Delete((byte8 *)inner_params, sizeof(sen));
			return;
		}
		else if (strcmp(funcname, "sizeof") == 0)
		{
			inner_params->pop_back();
			inner_params->erase(0);
			// type_data* td = get_type_with_namesen(inner_params);
		}
		else if (strcmp(funcname, "codeptr") == 0)
		{
		}
		else if (strcmp(funcname, "goto") == 0)
		{
		}

		fd = get_func_with_name(code->at(nameloc).data.str);
		// mem[writeup++] = fd->start_pc; // func

		sen *typen = wbss.sen_cut(code, 0, nameloc - 1);

		sen *params_sen = wbss.sen_cut(code, nameloc + 2, loc - 1);
		if (params_sen->size() == 0)
		{
			mem[writeup++] = 189; // FUNC
			mem[writeup++] = 200; // jmp
			*reinterpret_cast<uint *>(&mem[writeup]) = (uint)(fd->start_pc - &mem[0]);
			writeup += 4;

			code->release();
			fm->_Delete((byte8 *)code, sizeof(sen));

			inner_params->release();
			fm->_Delete((byte8 *)inner_params, sizeof(sen));

			params_sen->release();
			fm->_Delete((byte8 *)params_sen, sizeof(sen));
			return;
		}
		int last = params_sen->size() - 1;
		int coma = wbss.search_word_first(0, params_sen, ",");
		int savecoma = 0;

		int addadd = 0;
		int paramid = 0;

		mem[writeup++] = 189;

		while (coma != -1)
		{
			sen *param_sen = wbss.sen_cut(params_sen, savecoma, coma - 1);
			wbss.dbg_sen(param_sen);
			temp_mem *tm = get_asm_from_sen(param_sen, true, true);
			for (int i = 0; i < tm->memsiz; ++i)
			{
				mem[writeup++] = tm->mem[i];
			}

			switch (fd->param_data[paramid].td->typesiz)
			{
			case 1:
				mem[writeup++] = 190; // param
				break;
			case 2:
				mem[writeup++] = 191; // param
				break;
			case 4:
				mem[writeup++] = 192; // param
				break;
			}

			savecoma = coma + 1;
			coma = wbss.search_word_first(savecoma, params_sen, ",");

			++paramid;

			param_sen->release();
			fm->_Delete((byte8 *)param_sen, sizeof(sen));

			release_tempmem(tm);
		}

		wbss.dbg_sen(params_sen);

		sen *param_sen = wbss.sen_cut(params_sen, savecoma, last);
		wbss.dbg_sen(param_sen);
		temp_mem *tm = get_asm_from_sen(param_sen, true, true);
		for (int i = 0; i < tm->memsiz; ++i)
		{
			mem[writeup++] = tm->mem[i];
		}

		switch (fd->param_data[paramid].td->typesiz)
		{
		case 1:
			mem[writeup++] = 190; // param
			break;
		case 2:
			mem[writeup++] = 191; // param
			break;
		case 4:
			mem[writeup++] = 192; // param
			break;
		}

		mem[writeup++] = 200; // jmp
		*reinterpret_cast<uint *>(&mem[writeup]) = (uint)(fd->start_pc - &mem[0]);
		writeup += 4;

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		inner_params->release();
		fm->_Delete((byte8 *)inner_params, sizeof(sen));

		typen->release();
		fm->_Delete((byte8 *)typen, sizeof(sen));

		params_sen->release();
		fm->_Delete((byte8 *)params_sen, sizeof(sen));

		param_sen->release();
		fm->_Delete((byte8 *)param_sen, sizeof(sen));

		release_tempmem(tm);
	}

	void compile_returnInFunction(code_sen *cs)
	{
		sen *code = get_sen_from_codesen(cs);
		int loc = wbss.search_word_first(0, code, "return");
		sen *right_expr = wbss.sen_cut(code, loc + 1, code->size() - 1);
		temp_mem *right_tm = get_asm_from_sen(right_expr, true, true);

		for (int i = 0; i < right_tm->memsiz; ++i)
		{
			mem[writeup++] = right_tm->mem[i];
		}
		mem[writeup++] = (byte8)193;

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		right_expr->release();
		fm->_Delete((byte8 *)right_expr, sizeof(sen));

		release_tempmem(right_tm);
	}

	void compile_addStruct(code_sen *cs)
	{
		sen *code = get_sen_from_codesen(cs);
		int loc = wbss.search_word_first(0, code, "struct");
		struct_data *sd = (struct_data *)fm->_New(sizeof(struct_data), true);
		sd->name = code->at(loc).data.str;
		nextsd = sd;
		nextbd.bs = blockstate::bs_struct;

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));
	}

	void compile_break(code_sen *cs)
	{
		if (nextbd.breakpoints != nullptr)
		{
			mem[writeup++] = 188;
			int last = blockstack.size() - 1;
			while (blockstack.at(last)->bs != blockstate::bs_while)
			{
				last -= 1;
			}
			blockstack.at(last)->breakpoints->push_back(writeup);
			writeup += 4;
		}
	}

	void compile_continue(code_sen *cs)
	{
		if (nextbd.continuepoints != nullptr)
		{
			mem[writeup++] = 188;
			int last = blockstack.size() - 1;
			while (blockstack.at(last)->bs != blockstate::bs_while)
			{
				last -= 1;
			}
			blockstack.at(last)->continuepoints->push_back(writeup);
			writeup += 4;
		}
	}

	void compile_addsetVariable(code_sen *cs)
	{
		sen *code = get_sen_from_codesen(cs);
		// wbss.dbg_sen(code);
		int loc = wbss.search_word_first(0, code, "=");

		code_sen *cs0 = (code_sen *)fm->_New(sizeof(code_sen), true);
		cs0->sen = (char **)fm->_New(sizeof(char *) * loc, true);
		cs0->maxlen = loc;
		cs0->codeblocks = nullptr;
		cs0->fm = fm;
		cs0->ck = codeKind::ck_addVariable;
		for (int i = 0; i < loc; ++i)
		{
			cs0->sen[i] = cs->sen[i];
		}
		compile_addVariable(cs0);

		code_sen *cs1 = (code_sen *)fm->_New(sizeof(code_sen), true);
		int loc1 = cs->maxlen - loc + 1;
		cs1->maxlen = loc1;
		cs1->codeblocks = nullptr;
		cs1->ck = codeKind::ck_setVariable;
		cs1->fm = fm;
		cs1->sen = (char **)fm->_New(sizeof(char *) * loc1, true);
		for (int i = loc - 1; i < cs->maxlen; ++i)
		{
			cs1->sen[i - loc + 1] = cs->sen[i];
		}
		// dbg_codesen(cs1);
		compile_setVariable(cs1);

		code->release();
		fm->_Delete((byte8 *)code, sizeof(sen));

		fm->_Delete((byte8 *)cs0->sen, sizeof(char *) * loc);
		fm->_Delete((byte8 *)cs1->sen, sizeof(char *) * loc1);
		fm->_Delete((byte8 *)cs0, sizeof(sen));
		fm->_Delete((byte8 *)cs1, sizeof(sen));
	}

	int get_typesiz_with_addVariableCs(code_sen *cs)
	{
		if (cs->ck == codeKind::ck_addVariable)
		{
			sen *code = get_sen_from_codesen(cs);
			// wbss.dbg_sen(code);
			int loc = code->up - 1;
			char *variable_name = code->at(loc).data.str;
			sen *type_name = wbss.sen_cut(code, 0, loc - 1);
			type_data *td = get_type_with_namesen(type_name);
			int n = td->typesiz;
			// reqiure releasr typedata
			return n;
		}
		else
		{
			sen *code = get_sen_from_codesen(cs);
			// wbss.dbg_sen(code);
			int loc = wbss.search_word_first(0, code, "=");

			code_sen *cs0 = (code_sen *)fm->_New(sizeof(code_sen), true);
			cs0->sen = (char **)fm->_New(sizeof(char *) * loc, true);
			cs0->maxlen = loc;
			cs0->codeblocks = nullptr;
			cs0->fm = fm;
			cs0->ck = codeKind::ck_addVariable;
			for (int i = 0; i < loc; ++i)
			{
				cs0->sen[i] = cs->sen[i];
			}

			sen *code2 = get_sen_from_codesen(cs0);
			wbss.dbg_sen(code2);
			int loc2 = code2->up - 1;
			char *variable_name = code2->at(loc2).data.str;
			sen *type_name = wbss.sen_cut(code2, 0, loc2 - 1);
			type_data *td = get_type_with_namesen(type_name);
			int n = td->typesiz;
			// reqiure releasr typedata
			return n;
		}
	}

	void compile_code(code_sen *cs)
	{
		cs->start_line = writeup;
		if (cs->ck != codeKind::ck_blocks)
		{
			dbg_codesen(cs);
		}
		switch (cs->ck)
		{
		case codeKind::ck_addVariable:
			compile_addVariable(cs);
			break;
		case codeKind::ck_setVariable:
			compile_setVariable(cs);
			break;
		case codeKind::ck_if:
			compile_if(cs);
			break;
		case codeKind::ck_while:
			compile_while(cs);
			break;
		case codeKind::ck_blocks:
			compile_block(cs);
			break;
		case codeKind::ck_addFunction:
			compile_addFunction(cs);
			break;
		case codeKind::ck_useFunction:
			compile_useFunction(cs);
			break;
		case codeKind::ck_returnInFunction:
			compile_returnInFunction(cs);
			break;
		case codeKind::ck_addStruct:
			compile_addStruct(cs);
			break;
		case codeKind::ck_break:
			compile_break(cs);
			break;
		case codeKind::ck_continue:
			compile_continue(cs);
			break;
		case codeKind::ck_addsetVariable:
			compile_addsetVariable(cs);
			break;
		}
		cs->end_line = writeup - 1;
		// dbg_bakecode();
	}

	void bake_code(const char *filename)
	{
		lcstr *allcodeptr = GetCodeTXT(filename, fm);
		lcstr &allcode = *allcodeptr;
		AddTextBlocks(allcode);

		/*
		for (int i = 0; i < allcode_sen.size(); ++i)
		{
			cout << allcode_sen[i] << " ";
			if (i == 685)
			{
				int a = 0;
			}
		}
		cout << endl;
		*/

		vecarr<code_sen *> *senstptr = AddCodeFromBlockData(allcode_sen, "struct");

		vecarr<code_sen *> *senptr = AddCodeFromBlockData(allcode_sen, "none");
		senptr->islocal = false;

		csarr = senptr;

		cout << endl;

		int gs = 0;
		for (int i = 0; i < senptr->size(); ++i)
		{
			code_sen *cs = senptr->at(i);
			if (cs->ck == codeKind::ck_addVariable || cs->ck == codeKind::ck_addsetVariable)
			{
				// global var count
				gs += get_typesiz_with_addVariableCs(cs);
			}
			dbg_codesen(cs);
		}
		datamem.Init(gs + 8, false);
		datamem.up = gs;
		dataptr = datamem.Arr - mem;

		mem[writeup++] = 189; // func
		mem[writeup++] = 200; // jmp
		writeup += 4;		  // start function address

		for (int i = 0; i < senptr->size(); ++i)
		{
			// fm->dbg_fm1_lifecheck();
			code_sen *cs = senptr->at(i);
			compile_code(cs);
		}

		cout << endl;

		mem[writeup++] = 194;

		dbg_bakecode(csarr, 0);
	}
};

vecarr<InsideCode_Bake *> icbarr;

bool isBreaking = false;
int stopnum = 2417;

int code_control(vecarr<InsideCode_Bake *> *icbarr)
{
	static int stack = 0;

	if(isBreaking == false){
		return 1;
	}

	for (int i = 0; i < icbarr->size(); ++i)
	{
		//cout << "thread[ " << i << " ] next instruction" << endl;
		icbarr->at(i)->dbg_pc();
		icbarr->at(i)->dbg_stack();
		icbarr->at(i)->dbg_data();
		icbarr->at(i)->dbg_registers();
	}

	char c = 1;
	stack++;
	if (stack >= 1)
	{
		scanf("%c", &c);
		stack = 0;
	}
	switch (c)
	{
	case 'e':
		return 0;
	}

	return 1; // keep going
}

void execute(vecarr<InsideCode_Bake *> icbarr, int execodenum,
			 int (*control_func)(vecarr<InsideCode_Bake *> *), bool init)
{
	byte8 *tmptr_b;
	ushort *tmptr_s;
	uint *tmptr_i;
	register uint64_t _a = 0;
	register uint64_t _b = 0;
	register uint64_t _x = 0;
	register uint64_t _y = 0;
	register uint64_t _la = 0;
	uint64_t casting_value = 0;
	uint64_t casted_value = 0;
	void *castend_label = nullptr;

	int n = 0;
	int max = icbarr.size();
	int maxo = max;
	int exed_num = 0;
	uint strmax = 0;
	InsideCode_Bake *icb;

	byte8 **pc = nullptr; // program counter
	byte8 **sp = nullptr; // stack pointer

	byte8 **pcb = nullptr;
	ushort **pcs = nullptr;
	uint **pci = nullptr;

	byte8 **spb = nullptr;
	ushort **sps = nullptr;
	uint **spi = nullptr;

	vecarr<byte8 *> *fsp = nullptr;
	vecarr<byte8 *> *call_stack = nullptr;
	circularArray<uint64_t> _as;
	circularArray<uint64_t> _bs;

	byte8 **rfsp = 0; // function stack pos
	byte8 **lfsp = 0; // last function stack pos

	byte8 **rfspb = nullptr;
	ushort **rfsps = nullptr;
	uint **rfspi = nullptr;

	byte8 **lfspb = nullptr;
	ushort **lfsps = nullptr;
	uint **lfspi = nullptr;

	byte8 *mem = nullptr;
	void **inst = nullptr;
	void **cast = nullptr;
	void **dbgt = nullptr;
	void **inpt = nullptr;

	goto INST_INIT;
INST_INIT_END:

CONTEXT_SWITCH:

	if (n == maxo)
	{
		// control code
		n = 0;
		int k = control_func(&icbarr);
		switch (k)
		{
		case 0:
			goto *inst[194];
			break;
		case 1:
			goto CONTEXT_SWITCH;
			break;
		}
	}
	exed_num = 0;
	icb = icbarr[n];

	_a = icb->_a;
	_b = icb->_b;
	_x = icb->_x;
	_y = icb->_y;
	_la = icb->_la;
	_as = icb->_as;
	_bs = icb->_bs;

	mem = icb->mem;
	inst = icb->inst;
	dbgt = icb->dbgt;
	inpt = icb->inpt;

	pc = &icb->pc; // program counter
	sp = &icb->sp; // stack pointer

	fsp = &icb->fsp;
	call_stack = &icb->call_stack;

	rfsp = &icb->rfsp; // function stack pos
	lfsp = &icb->lfsp; // last function stack pos

	pcb = pc;
	pcs = reinterpret_cast<ushort **>(pc);
	pci = reinterpret_cast<uint **>(pc);

	spb = sp;
	sps = reinterpret_cast<ushort **>(sp);
	spi = reinterpret_cast<uint **>(sp);

	rfspb = rfsp;
	rfsps = reinterpret_cast<ushort **>(rfsp);
	rfspi = reinterpret_cast<uint **>(rfsp);

	lfspb = lfsp;
	lfsps = reinterpret_cast<ushort **>(lfsp);
	lfspi = reinterpret_cast<uint **>(lfsp);

	if((int)(icb->pc - mem) == stopnum){
		//isBreaking = true;
		cout << "Debug BreakPoint Check!" << endl;
	}
	goto *inst[**pc];

INSTEND:
	++exed_num;
	if (exed_num >= execodenum)
	{
		icb->_a = _a;
		icb->_b = _b;
		icb->_x = _x;
		icb->_y = _y;
		icb->_la = _la;
		icb->_as = _as;
		icb->_bs = _bs;

		++n;
		goto CONTEXT_SWITCH;
	}

	if((int)(icb->pc - mem) == stopnum){
		//isBreaking = true;
		cout << "Debug BreakPoint Check!" << endl;
	}

	goto *inst[**pc];

PROGRAMQUIT:
	return;

	// write every inst code

	// loc of sp
ADD_STACK_VARIABLE:
	++*pc;
	*sp = (*rfsp - **pci);
	++*pci;
	goto INSTEND;

	// var_id, value(byte)
SET_STACK_VARIABLE_CONST_1:
	++*pc;
	tmptr_b = (*rfsp - **pci);
	++*pci;
	*tmptr_b = **pcb;
	++*pc;
	goto INSTEND;

	// var_id, value(ushort)
SET_STACK_VARIABLE_CONST_2:
	++*pc;
	tmptr_s = reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	*tmptr_s = **pcs;
	++*pcs;
	goto INSTEND;

	// var_id, value(uint)
SET_STACK_VARIABLE_CONST_4:
	++*pc;
	tmptr_i = reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	*tmptr_i = **pci;
	++*pc;
	goto INSTEND;

	// var_id
SET_STACK_VARIABLE_FROMX_1:
	++*pc;
	*(*rfsp - (**pci)) = (byte8)_x;
	++*pci;
	goto INSTEND;

	// var_id
SET_STACK_VARIABLE_FROMX_2:
	++*pc;
	*reinterpret_cast<ushort *>(*rfsp - (**pci)) = (ushort)_x;
	++*pci;
	goto INSTEND;

	// var_id
SET_STACK_VARIABLE_FROMX_4:
	++*pc;
	*reinterpret_cast<uint *>(*rfsp - (**pci)) = (uint)_x;
	++*pci;
	goto INSTEND;

	// var_id
SET_STACK_VARIABLE_FROMY_1:
	++*pc;
	*(*rfsp - (**pci)) = (byte8)_y;
	++*pci;
	goto INSTEND;

	// var_id
SET_STACK_VARIABLE_FROMY_2:
	++*pc;
	*reinterpret_cast<ushort *>(*rfsp - (**pci)) = (ushort)_y;
	++*pci;
	goto INSTEND;

	// var_id
SET_STACK_VARIABLE_FROMY_4:
	++*pc;
	*reinterpret_cast<uint *>(*rfsp - (**pci)) = (uint)_y;
	++*pci;
	goto INSTEND;

	// var_id, address
SET_STACK_VARIABLE_ADDRESS_1:
	++*pc;
	tmptr_b = (*rfsp - (**pci));
	++*pci;
	*tmptr_b = *reinterpret_cast<byte8*>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id, address
SET_STACK_VARIABLE_ADDRESS_2:
	++*pc;
	tmptr_s = reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	*tmptr_s = *reinterpret_cast<ushort *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id, address
SET_STACK_VARIABLE_ADDRESS_4:
	++*pc;
	tmptr_i = reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	*tmptr_i = *reinterpret_cast<uint *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id, var_id
SET_STACK_VARIABLE_VARIABLE_1:
	++*pc;
	tmptr_b = (*rfsp - (**pci));
	++*pci;
	*tmptr_b = *(*rfsp - (*++*pci));
	++*pci;
	goto INSTEND;

	// var_id, var_id
SET_STACK_VARIABLE_VARIABLE_2:
	++*pc;
	tmptr_s = reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	*tmptr_s = *reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id, var_id
SET_STACK_VARIABLE_VARIABLE_4:
	++*pc;
	tmptr_i = reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	*tmptr_i = *reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// const(byte)
SET_A_CONST_1:
	_as.move_pivot(-1);
	_as[0] = *++*pc;
	++*pc;
	goto INSTEND;

	// const(ushort)
SET_A_CONST_2:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = **pcs;
	++*pcs;
	goto INSTEND;

	// const(uint)
SET_A_CONST_4:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = **pci;
	++*pci;
	goto INSTEND;

	// address
SET_A_ADDRESS_1:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = *reinterpret_cast<byte8*>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_A_ADDRESS_2:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = *reinterpret_cast<ushort *>(mem + (int)**pci);
	++pci;
	goto INSTEND;

	// address
SET_A_ADDRESS_4:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = *reinterpret_cast<uint *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id
SET_A_VARIABLE_1:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = *(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_A_VARIABLE_2:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = *reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_A_VARIABLE_4:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = *reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// const(byte)
SET_B_CONST_1:
	_bs.move_pivot(-1);
	_bs[0] = *++*pc;
	++*pc;
	goto INSTEND;

	// const(ushort)
SET_B_CONST_2:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = **pcs;
	++*pcs;
	goto INSTEND;

	// const(uint)
SET_B_CONST_4:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = **pci;
	++*pci;
	goto INSTEND;

	// address
SET_B_ADDRESS_1:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = *reinterpret_cast<byte8*>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_B_ADDRESS_2:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = *reinterpret_cast<ushort *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_B_ADDRESS_4:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = *reinterpret_cast<uint *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id
SET_B_VARIABLE_1:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = *(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_B_VARIABLE_2:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = *reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_B_VARIABLE_4:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = *reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// const(byte)
SET_X_CONST_1:
	_x = *++*pc;
	++*pc;
	goto INSTEND;

	// const(ushort)
SET_X_CONST_2:
	++*pc;
	_x = **pcs;
	++*pcs;
	goto INSTEND;

	// const(uint)
SET_X_CONST_4:
	++*pc;
	_x = **pci;
	++*pci;
	goto INSTEND;

	// address
SET_X_ADDRESS_1:
	++*pc;
	_x = *reinterpret_cast<byte8*>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_X_ADDRESS_2:
	++*pc;
	_x = *reinterpret_cast<ushort *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_X_ADDRESS_4:
	++*pc;
	_x = *reinterpret_cast<uint *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id
SET_X_VARIABLE_1:
	++*pc;
	_x = *(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_X_VARIABLE_2:
	++*pc;
	_x = *reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_X_VARIABLE_4:
	++*pc;
	_x = *reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// const(byte)
SET_Y_CONST_1:
	_y = *++*pc;
	++*pc;
	goto INSTEND;

	// const(ushort)
SET_Y_CONST_2:
	++*pc;
	_y = **pcs;
	++*pcs;
	goto INSTEND;

	// const(uint)
SET_Y_CONST_4:
	++*pc;
	_y = **pci;
	++*pci;
	goto INSTEND;

	// address
SET_Y_ADDRESS_1:
	++*pc;
	_y = *reinterpret_cast<byte8*>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_Y_ADDRESS_2:
	++*pc;
	_y = *reinterpret_cast<ushort *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// address
SET_Y_ADDRESS_4:
	++*pc;
	_y = *reinterpret_cast<uint *>(mem + (int)**pci);
	++*pci;
	goto INSTEND;

	// var_id
SET_Y_VARIABLE_1:
	++*pc;
	_y = *(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_Y_VARIABLE_2:
	++*pc;
	_y = *reinterpret_cast<ushort *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// var_id
SET_Y_VARIABLE_4:
	++*pc;
	_y = *reinterpret_cast<uint *>(*rfsp - (**pci));
	++*pci;
	goto INSTEND;

	// no param
AXBY:
	_x = _as[0];
	_y = _bs[0];
	++*pc;
	goto INSTEND;

	// ADD (NO PARAM) ...
AU_BYTE_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (char)((char)_x + (char)_y);
	++*pc;
	goto INSTEND;

AU_BYTE_ADD_B:
	_b = (char)((char)_x + (char)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)((byte8)_x + (byte8)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_ADD_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)((byte8)_x + (byte8)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (short)((short)_x + (short)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_ADD_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)((short)_x + (short)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)((ushort)_x + (ushort)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_ADD_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)((ushort)_x + (ushort)_y);
	++*pc;
	goto INSTEND;

AU_INT_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (int)((int)_x + (int)_y);
	++*pc;
	goto INSTEND;

AU_INT_ADD_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)((int)_x + (int)_y);
	++*pc;
	goto INSTEND;

AU_UINT_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (uint)((uint)_x + (uint)_y);
	++*pc;
	goto INSTEND;

AU_UINT_ADD_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)((uint)_x + (uint)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_ADD_A:
	_as.move_pivot(-1);
	_as[0] = (float)((float)_x + (float)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_ADD_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)((float)_x + (float)_y);
	++*pc;
	goto INSTEND;

	// MIN (NO PARAM) ...
AU_BYTE_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (char)((char)_x - (char)_y);
	++*pc;
	goto INSTEND;

AU_BYTE_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)((char)_x - (char)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)((byte8)_x - (byte8)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)((byte8)_x - (byte8)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (short)((short)_x - (short)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)((short)_x - (short)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)((ushort)_x - (ushort)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)((ushort)_x - (ushort)_y);
	++*pc;
	goto INSTEND;

AU_INT_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (int)((int)_x - (int)_y);
	++*pc;
	goto INSTEND;

AU_INT_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)((int)_x - (int)_y);
	++*pc;
	goto INSTEND;

AU_UINT_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (uint)((uint)_x - (uint)_y);
	++*pc;
	goto INSTEND;

AU_UINT_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)((uint)_x - (uint)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_MIN_A:
	_as.move_pivot(-1);
	_as[0] = (float)((float)_x - (float)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_MIN_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)((float)_x - (float)_y);
	++*pc;
	goto INSTEND;

	// MUL (NO PARAM) ...
AU_BYTE_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (char)((char)_x * (char)_y);
	++*pc;
	goto INSTEND;

AU_BYTE_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)((char)_x * (char)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (char)((byte8)_x * (byte8)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)((byte8)_x * (byte8)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (short)((short)_x * (short)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)((short)_x * (short)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)((ushort)_x * (ushort)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)((ushort)_x * (ushort)_y);
	++*pc;
	goto INSTEND;

AU_INT_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (int)((int)_x * (int)_y);
	++*pc;
	goto INSTEND;

AU_INT_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)((int)_x * (int)_y);
	++*pc;
	goto INSTEND;

AU_UINT_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (uint)((uint)_x * (uint)_y);
	++*pc;
	goto INSTEND;

AU_UINT_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)((uint)_x * (uint)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_MUL_A:
	_as.move_pivot(-1);
	_as[0] = (float)((float)_x * (float)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_MUL_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)((float)_x * (float)_y);
	++*pc;
	goto INSTEND;

	// DIV (NO PARAM) ...
AU_BYTE_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (char)((char)_x / (char)_y);
	++*pc;
	goto INSTEND;

AU_BYTE_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)((char)_x / (char)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)((byte8)_x / (byte8)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)((byte8)_x / (byte8)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (short)((short)_x / (short)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)((short)_x / (short)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)((ushort)_x / (ushort)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)((ushort)_x / (ushort)_y);
	++*pc;
	goto INSTEND;

AU_INT_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (int)((int)_x / (int)_y);
	++*pc;
	goto INSTEND;

AU_INT_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)((int)_x / (int)_y);
	++*pc;
	goto INSTEND;

AU_UINT_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (uint)((uint)_x / (uint)_y);
	++*pc;
	goto INSTEND;

AU_UINT_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)((uint)_x / (uint)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_DIV_A:
	_as.move_pivot(-1);
	_as[0] = (float)((float)_x / (float)_y);
	++*pc;
	goto INSTEND;

AU_FLOAT_DIV_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)((float)_x / (float)_y);
	++*pc;
	goto INSTEND;

	// PER (NO PARAM) ...
AU_BYTE_PER_A:
	_as.move_pivot(-1);
	_as[0] = (char)((char)_x % (char)_y);
	++*pc;
	goto INSTEND;

AU_BYTE_PER_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)((char)_x % (char)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_PER_A:
	_as.move_pivot(-1);
	_as[0] = (char)((byte8)_x % (byte8)_y);
	++*pc;
	goto INSTEND;

AU_UBYTE_PER_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)((byte8)_x % (byte8)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_PER_A:
	_as.move_pivot(-1);
	_as[0] = (char)((short)_x % (short)_y);
	++*pc;
	goto INSTEND;

AU_SHORT_PER_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)((short)_x % (short)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_PER_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)((ushort)_x % (ushort)_y);
	++*pc;
	goto INSTEND;

AU_USHORT_PER_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)((ushort)_x % (ushort)_y);
	++*pc;
	goto INSTEND;

AU_INT_PER_A:
	_as.move_pivot(-1);
	_as[0] = (int)((int)_x % (int)_y);
	++*pc;
	goto INSTEND;

AU_INT_PER_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)((int)_x % (int)_y);
	++*pc;
	goto INSTEND;

AU_UINT_PER_A:
	_as.move_pivot(-1);
	_as[0] = (uint)((uint)_x % (uint)_y);
	++*pc;
	goto INSTEND;

AU_UINT_PER_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)((uint)_x % (uint)_y);
	++*pc;
	goto INSTEND;

LU_BOOL_AND_A:
	_as.move_pivot(-1);
	_as[0] = (bool)_x && (bool)_y;
	++*pc;
	goto INSTEND;

LU_BOOL_AND_B:
	_bs.move_pivot(-1);
	_bs[0] = (bool)_x && (bool)_y;
	++*pc;
	goto INSTEND;

LU_BOOL_OR_A:
	_as.move_pivot(-1);
	_as[0] = (bool)_x || (bool)_y;
	++*pc;
	goto INSTEND;

LU_BOOL_OR_B:
	_bs.move_pivot(-1);
	_bs[0] = (bool)_x || (bool)_y;
	++*pc;
	goto INSTEND;

LU_BOOL_NOT_A:
	_as.move_pivot(-1);
	_as[0] = !(bool)_x;
	++*pc;
	goto INSTEND;

LU_BOOL_NOT_B:
	_bs.move_pivot(-1);
	_bs[0] = !(bool)_x;
	++*pc;
	goto INSTEND;

CM_BOOL_SAME_A:
	_as.move_pivot(-1);
	_as[0] = _x == _y;
	++*pc;
	goto INSTEND;

CM_BOOL_SAME_B:
	_bs.move_pivot(-1);
	_bs[0] = _x == _y;
	++*pc;
	goto INSTEND;

CM_BOOL_NOTSAME_A:
	_as.move_pivot(-1);
	_as[0] = _x != _y;
	++*pc;
	goto INSTEND;

CM_BOOL_NOTSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = _x != _y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (char)_x > (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)_x > (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)_x > (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)_x > (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (short)_x > (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)_x > (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)_x > (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)_x > (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (int)_x > (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)_x > (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (uint)_x > (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)_x > (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_LBIG_A:
	_as.move_pivot(-1);
	_as[0] = (float)_x > (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_LBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)_x > (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (char)_x >= (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)_x >= (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)_x >= (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)_x >= (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (short)_x >= (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)_x >= (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)_x >= (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)_x >= (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (int)_x >= (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)_x >= (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (uint)_x >= (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)_x >= (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_LBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (float)_x >= (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_LBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)_x >= (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (char)_x < (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)_x < (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)_x < (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)_x < (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (short)_x < (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)_x < (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)_x < (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)_x < (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (int)_x < (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)_x < (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (uint)_x < (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)_x < (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_RBIG_A:
	_as.move_pivot(-1);
	_as[0] = (float)_x < (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_RBIG_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)_x < (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (char)_x <= (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_BYTE_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (char)_x <= (char)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (byte8)_x <= (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UBYTE_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (byte8)_x <= (byte8)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (short)_x <= (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_SHORT_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (short)_x <= (short)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (ushort)_x <= (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_USHORT_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (ushort)_x <= (ushort)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (int)_x <= (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_INT_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (int)_x <= (int)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (uint)_x <= (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_UINT_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (uint)_x <= (uint)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_RBIGSAME_A:
	_as.move_pivot(-1);
	_as[0] = (float)_x <= (float)_y;
	++*pc;
	goto INSTEND;

CM_BOOL_FLOAT_RBIGSAME_B:
	_bs.move_pivot(-1);
	_bs[0] = (float)_x <= (float)_y;
	++*pc;
	goto INSTEND;

IF:
	++*pc;
	tmptr_i = *pci;
	++*pci;
	if (!(bool)_as[0])
		*pc = &mem[*tmptr_i];
	goto INSTEND;

JMP:
	++*pc;
	tmptr_i = *pci;
	++*pci;
	*pc = &mem[*tmptr_i];
	goto INSTEND;

FUNC:
	*lfsp = *rfsp;
	fsp->push_back(--*sp);
	*rfsp = fsp->last();
	++*pc;
	// *pc = &mem[*++*pci];
	goto INSTEND;

PARAM_1:
	**sp = (byte8)_as[0];
	--*sp;
	++*pc;
	goto INSTEND;

PARAM_2:
	**sps = (ushort)_as[0];
	--*sps;
	++*pc;
	goto INSTEND;

PARAM_4:
	--*spi;
	**spi = (uint)_as[0];
	++*pc;
	goto INSTEND;

RETURN:
	*sp = *rfsp;
	*rfsp = *lfsp;
	fsp->pop_back();
	*lfsp = (*fsp)[fsp->size() - 2];
	*pc = call_stack->last();
	call_stack->pop_back();
	++*pc;
	if(call_stack->size() == 0){
		goto EXIT;
	}
	goto INSTEND;

EXIT:
	++*pc;
	goto PROGRAMQUIT;

WAIT:
	++*pc;
	goto INSTEND;

SET_A_VARIABLE_ADDRESS:
	++*pc;
	_as.move_pivot(-1);
	_as[0] = (*rfsp - **pci) - mem;
	++*pci;
	goto INSTEND;

SET_B_VARIABLE_ADDRESS:
	++*pc;
	_bs.move_pivot(-1);
	_bs[0] = (*rfsp - **pci) - mem;
	++*pci;
	goto INSTEND;

SET_X_VARIABLE_ADDRESS:
	++*pc;
	_x = (*rfsp - **pci) - mem;
	++*pci;
	goto INSTEND;

SET_Y_VARIABLE_ADDRESS:
	++*pc;
	_y = (*rfsp - **pci) - mem;
	++*pci;
	goto INSTEND;

SET_LA_FROM_A:
	_la = _as[0];
	_as.move_pivot(1);
	++*pc;
	goto INSTEND;

FUNCJMP:
	++*pc;
	tmptr_i = *pci;
	++*pci;
	call_stack->push_back(*pc - 1);
	*pc = &mem[*tmptr_i];
	goto INSTEND;

CASTEND_A:
	//_as.move_pivot(-1);
	_as[0] = casted_value;
	++*pc;
	goto INSTEND;

CASTEND_B:
	//_bs.move_pivot(-1);
	_bs[0] = casted_value;
	++*pc;
	goto INSTEND;

CASTEND_X:
	_x = casted_value;
	++*pc;
	goto INSTEND;

CASTEND_Y:
	_y = casted_value;
	++*pc;
	goto INSTEND;

CASTING_A:
	castend_label = &&CASTEND_A;
	casting_value = _as[0];
	casted_value = 0;
	goto *cast[*++*pc];

CASTING_B:
	castend_label = &&CASTEND_B;
	casting_value = _bs[0];
	casted_value = 0;
	goto *cast[*++*pc];

CASTING_X:
	castend_label = &&CASTEND_X;
	casting_value = _x;
	casted_value = 0;
	goto *cast[*++*pc];

CASTING_Y:
	castend_label = &&CASTEND_Y;
	casting_value = _y;
	casted_value = 0;
	goto *cast[*++*pc];

	// CASTING TYPES
CAST_BYTE_TO_SHORT:
	casted_value = (short)*reinterpret_cast<char *>(&casting_value);
	goto *castend_label;

CAST_BYTE_TO_USHORT:
	casted_value = (ushort) * reinterpret_cast<char *>(&casting_value);
	goto *castend_label;

CAST_BYTE_TO_INT:
	casted_value = (int)*reinterpret_cast<char *>(&casting_value);
	goto *castend_label;

CAST_BYTE_TO_UINT:
	casted_value = (uint) * reinterpret_cast<char *>(&casting_value);
	goto *castend_label;

CAST_BYTE_TO_FLOAT:
	casted_value = (float)*reinterpret_cast<char *>(&casting_value);
	goto *castend_label;

CAST_UBYTE_TO_FLOAT:
	casted_value = (float)*reinterpret_cast<byte8 *>(&casting_value);
	goto *castend_label;

CAST_SHORT_TO_BYTE:
	casted_value = (char)*reinterpret_cast<short *>(&casting_value);
	goto *castend_label;

CAST_SHORT_TO_INT:
	casted_value = (int)*reinterpret_cast<short *>(&casting_value);
	goto *castend_label;

CAST_SHORT_TO_FLOAT:
	casted_value = (float)*reinterpret_cast<short *>(&casting_value);
	goto *castend_label;

CAST_USHORT_TO_FLOAT:
	casted_value = (float)*reinterpret_cast<ushort *>(&casting_value);
	goto *castend_label;

CAST_INT_TO_BYTE:
	casted_value = (char)*reinterpret_cast<int *>(&casting_value);
	goto *castend_label;

CAST_INT_TO_SHORT:
	casted_value = (short)*reinterpret_cast<int *>(&casting_value);
	goto *castend_label;

CAST_INT_TO_FLOAT:
	casted_value = (float)*reinterpret_cast<int *>(&casting_value);
	goto *castend_label;

CAST_UINT_TO_FLOAT:
	casted_value = (float)*reinterpret_cast<uint *>(&casting_value);
	goto *castend_label;

CAST_FLOAT_TO_BYTE:
	casted_value = (char)*reinterpret_cast<float *>(&casting_value);
	goto *castend_label;

CAST_FLOAT_TO_UBYTE:
	casted_value = (byte8) * reinterpret_cast<float *>(&casting_value);
	goto *castend_label;

CAST_FLOAT_TO_SHORT:
	casted_value = (short)*reinterpret_cast<float *>(&casting_value);
	goto *castend_label;

CAST_FLOAT_TO_USHORT:
	casted_value = (ushort) * reinterpret_cast<float *>(&casting_value);
	goto *castend_label;

CAST_FLOAT_TO_INT:
	casted_value = (int)*reinterpret_cast<float *>(&casting_value);
	goto *castend_label;

CAST_FLOAT_TO_UINT:
	casted_value = (uint) * reinterpret_cast<float *>(&casting_value);
	goto *castend_label;

GET_VALUE_A:
	_as[0] = *reinterpret_cast<uint *>(mem + (int)_as[0]);
	++*pc;
	goto INSTEND;

GET_VALUE_B:
	_bs[0] = *reinterpret_cast<uint *>(mem + (int)_bs[0]);
	++*pc;
	goto INSTEND;

GET_VALUE_X:
	_x = *reinterpret_cast<uint *>(mem + (int)_x);
	++*pc;
	goto INSTEND;

GET_VALUE_Y:
	_y = *reinterpret_cast<uint *>(mem + (int)_y);
	++*pc;
	goto INSTEND;

DBG_A:
	goto *dbgt[*++*pc];
DBG_END:
	++*pc;
	goto INSTEND;

DBG_A_BYTE:
	printf("%c", (char)_as[0]);
	goto DBG_END;

DBG_A_UBYTE:
	printf("%d", (byte8)_as[0]);
	goto DBG_END;

DBG_A_SHORT:
	printf("%d", (short)_as[0]);
	goto DBG_END;

DBG_A_USHORT:
	printf("%d", (ushort)_as[0]);
	goto DBG_END;

DBG_A_INT:
	printf("%d", (int)_as[0]);
	goto DBG_END;

DBG_A_UINT:
	printf("%d", (uint)_as[0]);
	goto DBG_END;

DBG_A_BOOL:
	printf((bool)_as[0] ? "true" : "false");
	goto DBG_END;

DBG_A_FLOAT:
	printf("%g", (float)_as[0]);
	goto DBG_END;

DBG_A_STRING:
	printf("%s", reinterpret_cast<char *>(mem + (int)_as[0]));
	goto DBG_END;

INP_A_PTR:
	goto *inpt[*++*pc];
INP_A_END:
	++*pc;
	goto INSTEND;

INP_BYTE:
	scanf("%c", reinterpret_cast<char *>(mem + (int)_as[0]));
	goto INP_A_END;

INP_UBYTE:
{
	unsigned int in;
	scanf("%u", &in);
	*reinterpret_cast<byte8 *>(mem + (int)_as[0]) = (byte8)in;
}
	goto INP_A_END;

INP_SHORT:
{
	int in;
	scanf("%d", &in);
	*reinterpret_cast<short *>(mem + (int)_as[0]) = (short)in;
}
	goto INP_A_END;

INP_USHORT:
{
	unsigned int in;
	scanf("%u", &in);
	*reinterpret_cast<ushort *>(mem + (int)_as[0]) = (ushort)in;
}
	goto INP_A_END;

INP_INT:
	scanf("%d", reinterpret_cast<int *>(mem + (int)_as[0]));
	goto INP_A_END;

INP_UINT:
	scanf("%u", reinterpret_cast<uint *>(mem + (int)_as[0]));
	goto INP_A_END;

INP_FLOAT:
	scanf("%f", reinterpret_cast<float *>(mem + (int)_as[0]));
	goto INP_A_END;

INP_BOOL:
{
	char str[8] = {};
	scanf("%s", str);
	if (strcmp(str, "true"))
	{
		*reinterpret_cast<bool *>(mem + (int)_as[0]) = true;
	}
	else
	{
		*reinterpret_cast<bool *>(mem + (int)_as[0]) = false;
	}
}
	goto INP_A_END;

INP_STRING:
	scanf("%s", reinterpret_cast<char *>(mem + (int)_as[0]));
	goto INP_A_END;

SET_ADDRESS_LA_FROM_A_1:
	*reinterpret_cast<byte8*>(mem + (int)_la) = _as[0];
	++*pc;
	goto INSTEND;

SET_ADDRESS_LA_FROM_A_2:
	*reinterpret_cast<ushort *>(mem + (int)_la) = _as[0];
	++*pc;
	goto INSTEND;

SET_ADDRESS_LA_FROM_A_4:
	*reinterpret_cast<uint *>(mem + (int)_la) = _as[0];
	++*pc;
	goto INSTEND;

SET_A_CONST_STRING:
	++*pc;
	strmax = *reinterpret_cast<uint *>(*pci);
	++*pci;
	_as.move_pivot(-1);
	_as[0] = *pc - mem;
	*pc += strmax;
	goto INSTEND;

SET_B_CONST_STRING:
	++*pc;
	strmax = *reinterpret_cast<uint *>(*pci);
	++*pci;
	_bs.move_pivot(-1);
	_bs[0] = *pc - mem;
	*pc += strmax;
	goto INSTEND;

POP_A:
	_as.move_pivot(1);
	++*pc;
	goto INSTEND;

POP_B:
	_bs.move_pivot(1);
	++*pc;
	goto INSTEND;

POP_AB:
	_as.move_pivot(1);
	_bs.move_pivot(1);
	++*pc;
	goto INSTEND;

INST_INIT:
	for (int k = 0; k < icbarr.size(); ++k)
	{
		cast = icbarr[k]->cast;
		inst = icbarr[k]->inst;
		dbgt = icbarr[k]->dbgt;
		inpt = icbarr[k]->inpt;

		uint32_t i = 0;
		inst[i++] = &&ADD_STACK_VARIABLE;
		inst[i++] = &&SET_STACK_VARIABLE_CONST_1;
		inst[i++] = &&SET_STACK_VARIABLE_CONST_2;
		inst[i++] = &&SET_STACK_VARIABLE_CONST_4;
		inst[i++] = &&SET_STACK_VARIABLE_FROMX_1;
		inst[i++] = &&SET_STACK_VARIABLE_FROMX_2;
		inst[i++] = &&SET_STACK_VARIABLE_FROMX_4;
		inst[i++] = &&SET_STACK_VARIABLE_FROMY_1;
		inst[i++] = &&SET_STACK_VARIABLE_FROMY_2;
		inst[i++] = &&SET_STACK_VARIABLE_FROMY_4;
		inst[i++] = &&SET_STACK_VARIABLE_ADDRESS_1;
		inst[i++] = &&SET_STACK_VARIABLE_ADDRESS_2;
		inst[i++] = &&SET_STACK_VARIABLE_ADDRESS_4;
		inst[i++] = &&SET_STACK_VARIABLE_VARIABLE_1;
		inst[i++] = &&SET_STACK_VARIABLE_VARIABLE_2;
		inst[i++] = &&SET_STACK_VARIABLE_VARIABLE_4;

		inst[i++] = &&SET_A_CONST_1;
		inst[i++] = &&SET_A_CONST_2;
		inst[i++] = &&SET_A_CONST_4;
		inst[i++] = &&SET_A_ADDRESS_1;
		inst[i++] = &&SET_A_ADDRESS_2;
		inst[i++] = &&SET_A_ADDRESS_4;
		inst[i++] = &&SET_A_VARIABLE_1;
		inst[i++] = &&SET_A_VARIABLE_2;
		inst[i++] = &&SET_A_VARIABLE_4;

		inst[i++] = &&SET_B_CONST_1;
		inst[i++] = &&SET_B_CONST_2;
		inst[i++] = &&SET_B_CONST_4;
		inst[i++] = &&SET_B_ADDRESS_1;
		inst[i++] = &&SET_B_ADDRESS_2;
		inst[i++] = &&SET_B_ADDRESS_4;
		inst[i++] = &&SET_B_VARIABLE_1;
		inst[i++] = &&SET_B_VARIABLE_2;
		inst[i++] = &&SET_B_VARIABLE_4;

		inst[i++] = &&SET_X_CONST_1;
		inst[i++] = &&SET_X_CONST_2;
		inst[i++] = &&SET_X_CONST_4;
		inst[i++] = &&SET_X_ADDRESS_1;
		inst[i++] = &&SET_X_ADDRESS_2;
		inst[i++] = &&SET_X_ADDRESS_4;
		inst[i++] = &&SET_X_VARIABLE_1;
		inst[i++] = &&SET_X_VARIABLE_2;
		inst[i++] = &&SET_X_VARIABLE_4;

		inst[i++] = &&SET_Y_CONST_1;
		inst[i++] = &&SET_Y_CONST_2;
		inst[i++] = &&SET_Y_CONST_4;
		inst[i++] = &&SET_Y_ADDRESS_1;
		inst[i++] = &&SET_Y_ADDRESS_2;
		inst[i++] = &&SET_Y_ADDRESS_4;
		inst[i++] = &&SET_Y_VARIABLE_1;
		inst[i++] = &&SET_Y_VARIABLE_2;
		inst[i++] = &&SET_Y_VARIABLE_4;

		inst[i++] = &&AXBY;

		inst[i++] = &&AU_BYTE_ADD_A;
		inst[i++] = &&AU_BYTE_ADD_B;
		inst[i++] = &&AU_UBYTE_ADD_A;
		inst[i++] = &&AU_UBYTE_ADD_B;
		inst[i++] = &&AU_SHORT_ADD_A;
		inst[i++] = &&AU_SHORT_ADD_B;
		inst[i++] = &&AU_USHORT_ADD_A;
		inst[i++] = &&AU_USHORT_ADD_B;
		inst[i++] = &&AU_INT_ADD_A;
		inst[i++] = &&AU_INT_ADD_B;
		inst[i++] = &&AU_UINT_ADD_A;
		inst[i++] = &&AU_UINT_ADD_B;
		inst[i++] = &&AU_FLOAT_ADD_A;
		inst[i++] = &&AU_FLOAT_ADD_B;

		inst[i++] = &&AU_BYTE_MIN_A;
		inst[i++] = &&AU_BYTE_MIN_B;
		inst[i++] = &&AU_UBYTE_MIN_A;
		inst[i++] = &&AU_UBYTE_MIN_B;
		inst[i++] = &&AU_SHORT_MIN_A;
		inst[i++] = &&AU_SHORT_MIN_B;
		inst[i++] = &&AU_USHORT_MIN_A;
		inst[i++] = &&AU_USHORT_MIN_B;
		inst[i++] = &&AU_INT_MIN_A;
		inst[i++] = &&AU_INT_MIN_B;
		inst[i++] = &&AU_UINT_MIN_A;
		inst[i++] = &&AU_UINT_MIN_B;
		inst[i++] = &&AU_FLOAT_MIN_A;
		inst[i++] = &&AU_FLOAT_MIN_B;

		inst[i++] = &&AU_BYTE_MUL_A;
		inst[i++] = &&AU_BYTE_MUL_B;
		inst[i++] = &&AU_UBYTE_MUL_A;
		inst[i++] = &&AU_UBYTE_MUL_B;
		inst[i++] = &&AU_SHORT_MUL_A;
		inst[i++] = &&AU_SHORT_MUL_B;
		inst[i++] = &&AU_USHORT_MUL_A;
		inst[i++] = &&AU_USHORT_MUL_B;
		inst[i++] = &&AU_INT_MUL_A;
		inst[i++] = &&AU_INT_MUL_B;
		inst[i++] = &&AU_UINT_MUL_A;
		inst[i++] = &&AU_UINT_MUL_B;
		inst[i++] = &&AU_FLOAT_MUL_A;
		inst[i++] = &&AU_FLOAT_MUL_B;

		inst[i++] = &&AU_BYTE_DIV_A;
		inst[i++] = &&AU_BYTE_DIV_B;
		inst[i++] = &&AU_UBYTE_DIV_A;
		inst[i++] = &&AU_UBYTE_DIV_B;
		inst[i++] = &&AU_SHORT_DIV_A;
		inst[i++] = &&AU_SHORT_DIV_B;
		inst[i++] = &&AU_USHORT_DIV_A;
		inst[i++] = &&AU_USHORT_DIV_B;
		inst[i++] = &&AU_INT_DIV_A;
		inst[i++] = &&AU_INT_DIV_B;
		inst[i++] = &&AU_UINT_DIV_A;
		inst[i++] = &&AU_UINT_DIV_B;
		inst[i++] = &&AU_FLOAT_DIV_A;
		inst[i++] = &&AU_FLOAT_DIV_B;

		inst[i++] = &&AU_BYTE_PER_A;
		inst[i++] = &&AU_BYTE_PER_B;
		inst[i++] = &&AU_UBYTE_PER_A;
		inst[i++] = &&AU_UBYTE_PER_B;
		inst[i++] = &&AU_SHORT_PER_A;
		inst[i++] = &&AU_SHORT_PER_B;
		inst[i++] = &&AU_USHORT_PER_A;
		inst[i++] = &&AU_USHORT_PER_B;
		inst[i++] = &&AU_INT_PER_A;
		inst[i++] = &&AU_INT_PER_B;
		inst[i++] = &&AU_UINT_PER_A;
		inst[i++] = &&AU_UINT_PER_B;

		inst[i++] = &&LU_BOOL_AND_A;
		inst[i++] = &&LU_BOOL_AND_B;

		inst[i++] = &&LU_BOOL_OR_A;
		inst[i++] = &&LU_BOOL_OR_B;

		inst[i++] = &&LU_BOOL_NOT_A;
		inst[i++] = &&LU_BOOL_NOT_B;

		inst[i++] = &&CM_BOOL_SAME_A;
		inst[i++] = &&CM_BOOL_SAME_B;

		inst[i++] = &&CM_BOOL_NOTSAME_A;
		inst[i++] = &&CM_BOOL_NOTSAME_B;

		inst[i++] = &&CM_BOOL_BYTE_LBIG_A;
		inst[i++] = &&CM_BOOL_BYTE_LBIG_B;
		inst[i++] = &&CM_BOOL_UBYTE_LBIG_A;
		inst[i++] = &&CM_BOOL_UBYTE_LBIG_B;
		inst[i++] = &&CM_BOOL_SHORT_LBIG_A;
		inst[i++] = &&CM_BOOL_SHORT_LBIG_B;
		inst[i++] = &&CM_BOOL_USHORT_LBIG_A;
		inst[i++] = &&CM_BOOL_USHORT_LBIG_B;
		inst[i++] = &&CM_BOOL_INT_LBIG_A;
		inst[i++] = &&CM_BOOL_INT_LBIG_B;
		inst[i++] = &&CM_BOOL_UINT_LBIG_A;
		inst[i++] = &&CM_BOOL_UINT_LBIG_B;
		inst[i++] = &&CM_BOOL_FLOAT_LBIG_A;
		inst[i++] = &&CM_BOOL_FLOAT_LBIG_B;

		inst[i++] = &&CM_BOOL_BYTE_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_BYTE_LBIGSAME_B;
		inst[i++] = &&CM_BOOL_UBYTE_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_UBYTE_LBIGSAME_B;
		inst[i++] = &&CM_BOOL_SHORT_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_SHORT_LBIGSAME_B;
		inst[i++] = &&CM_BOOL_USHORT_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_USHORT_LBIGSAME_B;
		inst[i++] = &&CM_BOOL_INT_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_INT_LBIGSAME_B;
		inst[i++] = &&CM_BOOL_UINT_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_UINT_LBIGSAME_B;
		inst[i++] = &&CM_BOOL_FLOAT_LBIGSAME_A;
		inst[i++] = &&CM_BOOL_FLOAT_LBIGSAME_B;

		inst[i++] = &&CM_BOOL_BYTE_RBIG_A;
		inst[i++] = &&CM_BOOL_BYTE_RBIG_B;
		inst[i++] = &&CM_BOOL_UBYTE_RBIG_A;
		inst[i++] = &&CM_BOOL_UBYTE_RBIG_B;
		inst[i++] = &&CM_BOOL_SHORT_RBIG_A;
		inst[i++] = &&CM_BOOL_SHORT_RBIG_B;
		inst[i++] = &&CM_BOOL_USHORT_RBIG_A;
		inst[i++] = &&CM_BOOL_USHORT_RBIG_B;
		inst[i++] = &&CM_BOOL_INT_RBIG_A;
		inst[i++] = &&CM_BOOL_INT_RBIG_B;
		inst[i++] = &&CM_BOOL_UINT_RBIG_A;
		inst[i++] = &&CM_BOOL_UINT_RBIG_B;
		inst[i++] = &&CM_BOOL_FLOAT_RBIG_A;
		inst[i++] = &&CM_BOOL_FLOAT_RBIG_B;

		inst[i++] = &&CM_BOOL_BYTE_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_BYTE_RBIGSAME_B;
		inst[i++] = &&CM_BOOL_UBYTE_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_UBYTE_RBIGSAME_B;
		inst[i++] = &&CM_BOOL_SHORT_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_SHORT_RBIGSAME_B;
		inst[i++] = &&CM_BOOL_USHORT_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_USHORT_RBIGSAME_B;
		inst[i++] = &&CM_BOOL_INT_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_INT_RBIGSAME_B;
		inst[i++] = &&CM_BOOL_UINT_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_UINT_RBIGSAME_B;
		inst[i++] = &&CM_BOOL_FLOAT_RBIGSAME_A;
		inst[i++] = &&CM_BOOL_FLOAT_RBIGSAME_B;

		inst[i++] = &&IF;

		inst[i++] = &&JMP;

		inst[i++] = &&FUNC;

		inst[i++] = &&PARAM_1;
		inst[i++] = &&PARAM_2;
		inst[i++] = &&PARAM_4;

		inst[i++] = &&RETURN;

		inst[i++] = &&EXIT;

		inst[i++] = &&SET_A_VARIABLE_ADDRESS;
		inst[i++] = &&SET_B_VARIABLE_ADDRESS;
		inst[i++] = &&SET_X_VARIABLE_ADDRESS;
		inst[i++] = &&SET_Y_VARIABLE_ADDRESS;

		inst[i++] = &&SET_LA_FROM_A;
		inst[i++] = &&FUNCJMP;

		inst[i++] = &&CASTING_A;
		inst[i++] = &&CASTING_B;
		inst[i++] = &&CASTING_X;
		inst[i++] = &&CASTING_Y;

		inst[i++] = &&GET_VALUE_A;
		inst[i++] = &&GET_VALUE_B;
		inst[i++] = &&GET_VALUE_X;
		inst[i++] = &&GET_VALUE_Y;

		inst[i++] = &&DBG_A;
		inst[i++] = &&INP_A_PTR;

		inst[i++] = &&SET_ADDRESS_LA_FROM_A_1;
		inst[i++] = &&SET_ADDRESS_LA_FROM_A_2;
		inst[i++] = &&SET_ADDRESS_LA_FROM_A_4;

		inst[i++] = &&SET_A_CONST_STRING;
		inst[i++] = &&SET_B_CONST_STRING;

		inst[i++] = &&POP_A;
		inst[i++] = &&POP_B;
		inst[i++] = &&POP_AB;

		int c = 0;
		cast[c++] = &&CAST_BYTE_TO_SHORT;
		cast[c++] = &&CAST_BYTE_TO_USHORT;
		cast[c++] = &&CAST_BYTE_TO_INT;
		cast[c++] = &&CAST_BYTE_TO_UINT;
		cast[c++] = &&CAST_BYTE_TO_FLOAT;
		cast[c++] = &&CAST_UBYTE_TO_FLOAT;
		cast[c++] = &&CAST_SHORT_TO_BYTE;
		cast[c++] = &&CAST_SHORT_TO_INT;
		cast[c++] = &&CAST_SHORT_TO_FLOAT;
		cast[c++] = &&CAST_USHORT_TO_FLOAT;
		cast[c++] = &&CAST_INT_TO_BYTE;
		cast[c++] = &&CAST_INT_TO_SHORT;
		cast[c++] = &&CAST_INT_TO_FLOAT;
		cast[c++] = &&CAST_UINT_TO_FLOAT;
		cast[c++] = &&CAST_FLOAT_TO_BYTE;
		cast[c++] = &&CAST_FLOAT_TO_UBYTE;
		cast[c++] = &&CAST_FLOAT_TO_SHORT;
		cast[c++] = &&CAST_FLOAT_TO_USHORT;
		cast[c++] = &&CAST_FLOAT_TO_INT;
		cast[c++] = &&CAST_FLOAT_TO_UINT;

		int d = 0;
		dbgt[d++] = &&DBG_A_BYTE;
		dbgt[d++] = &&DBG_A_UBYTE;
		dbgt[d++] = &&DBG_A_SHORT;
		dbgt[d++] = &&DBG_A_USHORT;
		dbgt[d++] = &&DBG_A_INT;
		dbgt[d++] = &&DBG_A_UINT;
		dbgt[d++] = &&DBG_A_FLOAT;
		dbgt[d++] = &&DBG_A_BOOL;
		dbgt[d++] = &&DBG_A_STRING;

		int n = 0;
		inpt[n++] = &&INP_BYTE;
		inpt[n++] = &&INP_UBYTE;
		inpt[n++] = &&INP_SHORT;
		inpt[n++] = &&INP_USHORT;
		inpt[n++] = &&INP_INT;
		inpt[n++] = &&INP_UINT;
		inpt[n++] = &&INP_FLOAT;
		inpt[n++] = &&INP_BOOL;
		inpt[n++] = &&INP_STRING;

		cout << "instruction size : " << i << endl;

		for (i = i; i < InsideCode_Bake::max_instruction; ++i)
		{
			inst[i] = &&WAIT;
		}
	}

	goto INST_INIT_END;
}

int main()
{
	InsideCode_Bake icb;
	FM_System0 *fm = new FM_System0();
	fm->SetHeapData(4096, 4096, 4096, 4096);
	icb.setfm(fm);
	icb.init();
	icb.bake_code("code.txt");

	vecarr<InsideCode_Bake *> exeicbs;
	exeicbs.NULLState();
	exeicbs.Init(2, false);
	exeicbs.push_back(&icb);
	execute(exeicbs, 1000, code_control, true);
}