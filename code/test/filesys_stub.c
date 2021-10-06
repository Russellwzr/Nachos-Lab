#include "syscall.h"

int main()
{
	int sz;
	char buffer[100];
	OpenFileId fp;
	Create("Test");
	fp = Open("Test");
	Write("hello world!\n", 20, fp);
	sz = Read(buffer, 20, fp);
	Close(fp);
	Exit(0);
}

