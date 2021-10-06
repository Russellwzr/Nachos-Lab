// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

#define MAX_USERPROCESSES 256
BitMap *AddrSpace::pageMap = new BitMap(NumPhysPages);
BitMap *AddrSpace::pidMap = new BitMap(MAX_USERPROCESSES);

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{

    // 分配 pid  
    spaceId = pidMap->Find() + 100;     // 100~256 for user process
    if(spaceId == 99){
      printf("Too many processes in Nachos!\n");
      return;
    }

    NoffHeader noffH;           
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);   
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);          
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // 计算地址空间大小
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;   
    numPages = divRoundUp(size, PageSize);  
    size = numPages * PageSize;             
    ASSERT(numPages <= NumPhysPages);       

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);

    // 创建页表
    pageTable = new TranslationEntry[numPages];
    
    // 初始化页表
    ASSERT(pageMap->NumClear() >= numPages);   
    for (i = 0; i < numPages; i++) {
      pageTable[i].virtualPage = i;        
      pageTable[i].physicalPage = pageMap->Find();  
      pageTable[i].valid = TRUE;
      pageTable[i].use = FALSE;
      pageTable[i].dirty = FALSE;
      pageTable[i].readOnly = FALSE;       
    }
    
    // 将代码段和数据段载入内存
    if (noffH.code.size > 0) {
      int pagePos = pageTable[noffH.code.virtualAddr/PageSize].physicalPage * PageSize;
      int offSet = noffH.code.virtualAddr % PageSize;

      executable->ReadAt(&(machine->mainMemory[pagePos+offSet]),
        noffH.code.size, noffH.code.inFileAddr); 
    }
    if (noffH.initData.size > 0) {
      int pagePos = pageTable[noffH.initData.virtualAddr/PageSize].physicalPage * PageSize;
      int offSet = noffH.initData.virtualAddr % PageSize;

      executable->ReadAt(&(machine->mainMemory[pagePos+offSet]),
                noffH.initData.size, noffH.initData.inFileAddr);
    }

#ifdef FILESYS
    // 文件描述符初始化
    for(int i = 3; i < 10; i++) fileDescriptor[i] = NULL;
    OpenFile *StdinFile = new OpenFile("stdin");
    OpenFile *StdoutFile = new OpenFile("stdout");
    OpenFile *StderrFile = new OpenFile("stderr");
    fileDescriptor[0] = StdinFile;
    fileDescriptor[1] = StdoutFile;
    fileDescriptor[2] = StderrFile;
#endif

}


#ifdef FILESYS
int AddrSpace::getFileDescriptor(OpenFile *openfile) {
  for(int i = 3; i < 10; i++)
    if(fileDescriptor[i] == NULL){
      fileDescriptor[i] = openfile;
      return i;
    }
  return -1;
}

OpenFile* AddrSpace::getFileId(int fd) {
  ASSERT((fd >= 0) && (fd < UserProgramNum));
  return fileDescriptor[fd];
}

void AddrSpace::releaseFileDescriptor(int fd) {
  ASSERT((fd >= 0) && (fd < UserProgramNum));
  fileDescriptor[fd] = NULL;
}
#endif

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    pidMap->Clear(spaceId - 100);
    for(int i = 0; i < numPages; i ++){
        pageMap->Clear(pageTable[i].physicalPage);
    }
    delete [] pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	   machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {
    pageTable = machine->pageTable;
    numPages = machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

void AddrSpace::Print()
{
    printf("page table dump:  %d pages in total.\n",numPages);
    printf("=============================================\n");
    printf("\tVirPage,\tPhysPage\n");
    for(int i = 0;i < numPages; i ++){
        printf("\t %d,\t\t%d\n", pageTable[i].virtualPage, pageTable[i].physicalPage);
    }   
    printf("=============================================\n\n");
}
