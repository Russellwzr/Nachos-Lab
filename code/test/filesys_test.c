#include "syscall.h"

int main()
{
	int sz;
	char buffer[100];
	OpenFileId fp,fp1;
	Create("Test");
	Create("Test1");
	fp = Open("Test");
	fp1 = Open("Test1");
	Write("hello world!\n", 20, fp);
	Write("hello world!\n", 20, fp1);
	sz = Read(buffer, 20, fp);
	Close(fp);
	Close(fp1);
	Exit(0);
}

