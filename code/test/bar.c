#include "syscall.h"
int
main()
{
	Exec("../test/exec.noff");
	Halt();
}