# InsideCode
Compiler module for c++ code that allows developers to set the grammar of the code<br />

If you use this code with your project,<br />
While your program is running, you can input a string and compile it.<br />

You can also get all the content of the code entered while the code is running<br />
(such as the name, address, type, etc.) <br />
so that you can change the code you read while the compiler is running. <br />
You can change the code that runs in real time.<br />

Simply put, the code executed is controlled by making it a variable.<br />

Using these things, you can create your own grammar in your language.<br />
You can implement several convenient functions in your language.<br />
Or you can discover new possibilities of coding.<br />

# The Three Level of InsideCode

There are programming languages for various functions in the world,<br /> 
and each code has its own advantages.<br />
InsideCodes are divided into three types of InsideCodes <br />
to develop their advantages comprehensively and to be used in a variety of places.<br />

## 1. String Parsing Level
It is a level that takes and interprets the string of the code as it is and executes it at the same time.<br />
It is designed to have the advantages of interpreter languages.<br />
Examples would be Python, BASIC, and Lisp. But it is almost c interpreter.<br />

This can be useful, for example,<br />
if you want to give the user the ability to run a part of the program, or if you want to run a method of coding,<br />
It may be unnecessary for code that spends a lot of time translating,<br />
so much of what is required of most products is performance-critical.<br />

## 2. BakeCode Level
Because it prevents the operating system from modifying the memory of the code in the process memory space for safety,<br />
so a idea that cannot be expanded,<br />
there is a method of compiling the original code into byte code first,<br />
and executing it through a virtual machine when executing the byte code to allow some performance by product<br />
or to help programmers experiment with the code and catch errors.<br />

C#, JAVA, etc. use this method and are sometimes called JIT compilers. ex> LuaJIT<br />
It is the bake code that does this.<br />

## 3. AtomLevel
Running code while running inside a program costs more than you think.<br />
Memory and speed become more inefficient than existing code,<br />
and it is difficult to properly use a number of special commands (ex> SIMD) provided by multiple architectures.<br />
In addition, because the cache of modern CPUs requires fast access to the same memory to achieve optimal performance,<br />
the bake code level or string parsing level, which must go through more than one memory,<br />
can never keep up with the speed and optimization of code executed in machine language.<br />

Therefore, in order to overcome this, it is necessary to compile the code,<br />
transfer the machine language data to the heap memory,<br />
and call and use it like a function. That is the atom level.<br />

Usually, there are so many programming languages with these compilers (C, C++, Rust),<br />
but most of them only exist in the program itself,<br />
and there were no compilers that could go into a program and add additional functionality.<br />

## 4. Shader
But I think there's a better way to perform than these three, and that's Shader. Shader didn't feel the need to make it because he could already type in a string from a graphics library or GPU library.
However, Shader won't be able to solve all the problems more effectively. Although there are many processors inside the GPU, they are specialized in doing a lot of simple tasks. Therefore, due to a number of problems, such as the speed of communication with the CPU and the problem of deciding where to store the results, it is not usually write the main logic.
Therefore, if you need to do a lot of simple tasks very quickly, manipulating the GPU may be a better way than doing something like InsideCode.

# ETC
But it's not complete yet.<br />
It contains a lot of information, so every time you add a variable, you use a lot more memory.<br />
Free heap memory has not yet been implemented.<br />
To be honest, it's not the quality that I can use yet, but I'll try to complete it.<br />

# How To Test It?
Currently, the code is written in the <code.txt> file, and the compiler code is in the <main.cpp> and <main.h> files.<br />
To use the compiler, you can enter the code in the <code.txt> file.<br />
However, if you want to change the grammar or compiler code, please modify the <main.cpp> file.<br />

# Early Grammer
The grammar currently recorded in main.cpp is very similar to language C. But the details can be different.<br />

## Variable declaration <br />
int a;<br />
float b;<br />
bool c;<br />
char d;<br />

## Variable Set <br />
a = 1;<br />
b = 0.4;<br />
c = false;<br />
d = 'a';<br />

## if, while, for <br />
It's like C language.<br />

## Array declaration <br />
int[50] a;<br />
int[num] b;<br />

## pointer useage <br />
int a;<br />
a = 10;<br />
int* aptr;<br />
aptr = &a;<br />

## struct useage <br />
struct ABC{<br />
  int a;<br />
  float b;<br />
  bool c;<br />
};<br />

int main(){<br />
  ABC abc;<br />
  abc.a = 10;<br />
  return 0;<br />
}<br />

## input and output<br />
int num;<br />
input(&num);<br />
output("num : %v \n", num);<br />
