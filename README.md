# InsideCode
Compiler module for c++ code that allows developers to set the grammar of the code

If you use this code with your project,
While your program is running, you can input a string and compile it.

You can also get all the content of the code entered while the code is running
(such as the name, address, type, etc.) 
so that you can change the code you read while the compiler is running. 
You can change the code that runs in real time.

Simply put, the code executed is controlled by making it a variable.

Using these things, you can create your own grammar in your language.
You can implement several convenient functions in your language.
Or you can discover new possibilities of coding.

But it's not complete yet.
It contains a lot of information, so every time you add a variable, you use a lot more memory.
Free heap memory has not yet been implemented.
To be honest, it's not the quality that I can use yet, but I'll try to complete it.

Currently, the code is written in the <code.txt> file, and the compiler code is in the <main.cpp> and <main.h> files.
To use the compiler, you can enter the code in the <code.txt> file.
However, if you want to change the grammar or compiler code, please modify the <main.cpp> file.

The grammar currently recorded in main.cpp is very similar to language C. But the details can be different.

//Variable declaration : 
int a;
float b;
bool c;
char d;

//Variable Set : 
a = 1;
b = 0.4;
c = false;
d = 'a';

//if, while, for : 
It's like C language.

//Array declaration : 
int[50] a;
int[num] b;

//pointer useage : 
int a;
a = 10;
int* aptr;
aptr = &a;

//struct useage : 
struct ABC{
  int a;
  float b;
  bool c;
};

int main(){
  ABC abc;
  abc.a = 10;
  return 0;
}
