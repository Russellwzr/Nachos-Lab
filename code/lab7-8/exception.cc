// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//      arg1 -- r4
//      arg2 -- r5
//      arg3 -- r6
//      arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions 
//  are in machine.h.
//----------------------------------------------------------------------


void AdvancePC(){
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

void StartProcess(int spaceId){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
    ASSERT(FALSE);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    //printf("The exception type is %d.\n", type);

    if(which == SyscallException){
        switch(type){

            case SC_Halt:{
                printf("SC_Halt, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            }

            case SC_Exec:{
                printf("SC_Exec, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int addr = machine->ReadRegister(4);
                char filename[128];
                int i = 0;
                do{
                    machine->ReadMem(addr+i, 1, (int *)&filename[i]);
                }while(filename[i++] != '\0');

                OpenFile *executable = fileSystem->Open(filename);
                if(executable == NULL) {
                    printf("Unable to open file %s\n",filename);
                    return;
                }
                AddrSpace *space = new AddrSpace(executable);
                // space->Print();   
                delete executable;  
                Thread *thread = new Thread(filename);
                printf("new Thread, SpaceId: %d, Name: %s\n",space->getSpaceId(),filename);
                thread->Fork(StartProcess,(int)space->getSpaceId());
                thread->space = space;
                machine->WriteRegister(2, space->getSpaceId());
                AdvancePC();
                break;
            }

            case SC_Exit:{
                printf("SC_Exit, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int exitCode = machine->ReadRegister(4);
                machine->WriteRegister(2, exitCode);
                currentThread->setExitCode(exitCode);
                if(exitCode == 99){
                    List *terminatedList = scheduler->getTerminatedList();
                    scheduler->emptyList(terminatedList);
                }
                delete currentThread->space;
                currentThread->Finish();
                AdvancePC();
                break;
            }

            case SC_Join:{
                printf("SC_Join, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int spaceId = machine->ReadRegister(4);
                currentThread->Join(spaceId);
                // 返回 Joinee 的退出码
                machine->WriteRegister(2, currentThread->waitExitCode());
                AdvancePC();
                break;
            }

            case SC_Yield:{
                printf("SC_Yield, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                currentThread->Yield();
                AdvancePC();
                break;
            }

#ifdef FILESYS_STUB
            case SC_Create:{
                printf("SC_Create, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int base = machine->ReadRegister(4);
                int value;
                int i = 0;
                char filename[128];
                do{
                    machine->ReadMem(base + i, 1, &value);
                    filename[i] = *(char*)&value;
                    i ++;
                }while(*(char*)&value != '\0' && i < 128);

                int fileDescriptor = OpenForWrite(filename);
                if(fileDescriptor == -1) 
                    printf("Create file %s failed!\n",filename);
                else 
                    printf("Create file %s succeed, the file id is %d\n",filename,fileDescriptor);
                Close(fileDescriptor);
                AdvancePC();
                break;
            }
            case SC_Open:{
                printf("SC_Open, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int base = machine->ReadRegister(4);
                int value;
                int i = 0;
                char filename[128];
                do{
                    machine->ReadMem(base + i, 1, &value);
                    filename[i] = *(char*)&value;
                    i ++;
                }while(*(char*)&value != '\0' && i < 128);

                int fileDescriptor = OpenForReadWrite(filename, FALSE);
                if(fileDescriptor == -1) 
                    printf("Open file %s failed!\n",filename);
                else 
                    printf("Open file %s succeed, the file id is %d\n",filename, fileDescriptor);
                machine->WriteRegister(2, fileDescriptor);
                AdvancePC();
                break;
            }

            case SC_Write:{
                printf("SC_Write, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());

                int base = machine->ReadRegister(4);       // buffer
                int size = machine->ReadRegister(5);       // buffer written to file
                int fileId = machine->ReadRegister(6);     // fd
                int value;
                int i = 0;

                OpenFile *openfile = new OpenFile(fileId);
                ASSERT(openfile != NULL);

                char buffer[128];
                do{
                    machine->ReadMem(base + i, 1, &value);
                    buffer[i] = *(char*)&value;
                    i ++;
                }while(&value != '\0' && i < size);
                buffer[size] = '\0';

                int writePos;
                if(fileId == 1) 
                    writePos = 0;
                else 
                    writePos = openfile->Length();
               
                int writtenBytes = openfile->WriteAt(buffer, size, writePos);
                if(writtenBytes == 0) 
                    printf("Write file failed!\n");
                else 
                    printf("\"%s\" has wrote in file %d succeed!\n",buffer,fileId);
                AdvancePC();
                break;
            }
            case SC_Read:{
                printf("SC_Read, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());

                int base = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);       
                int fileId = machine->ReadRegister(6);     

                OpenFile *openfile = new OpenFile(fileId);
                char buffer[size+1];
                int readnum = openfile->Read(buffer,size);

                for(int i = 0; i < size; i++)
                    if(!machine->WriteMem(base, 1, buffer[i])) 
                        printf("This is something Wrong.\n");
                buffer[size] = '\0';
                printf("Read succeed, the content is \"%s\", the length is %d\n",buffer,size);
                machine->WriteRegister(2,readnum);
                AdvancePC();
                break;
            }
            case SC_Close:{
                printf("SC_Close, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int fileId = machine->ReadRegister(4);
                Close(fileId);
                printf("File %d closed succeed!\n",fileId);
                AdvancePC();
                break;
            }
#else
           case SC_Create:{
                printf("SC_Create, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int base = machine->ReadRegister(4);
                int value;
                int i = 0;
                char filename[128];
                do{
                    machine->ReadMem(base + i, 1, &value);
                    filename[i] = *(char*)&value;
                    i ++;
                }while(*(char*)&value != '\0' && i < 128);

                if(!fileSystem->Create(filename, 0)) 
                    printf("Create file %s failed!\n",filename);
                else 
                    printf("Create file %s succeed!\n",filename);
                AdvancePC();
                break;
            }
            case SC_Open:{
                printf("SC_Open, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int base = machine->ReadRegister(4);
                int value;
                int i = 0;
                char filename[128];
                do{
                    machine->ReadMem(base + i, 1, &value);
                    filename[i] = *(char*)&value;
                    i ++;
                }while(*(char*)&value != '\0' && i < 128);
                int fileId;
                OpenFile *openfile = fileSystem->Open(filename);
                if(openfile == NULL) {
                    printf("File \"%s\" not Exists, could not open it.\n",filename);
                    fileId = -1;
                }
                else{
                    fileId = currentThread->space->getFileDescriptor(openfile);
                    if(fileId < 0) 
                        printf("Too many files opened!\n");
                    else 
                        printf("file:\"%s\" open succeed, the file id is %d\n",filename,fileId);
                }
                machine->WriteRegister(2,fileId);
                AdvancePC();
                break;
            }


            case SC_Write:{
                printf("SC_Write, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int base = machine->ReadRegister(4);      
                int size = machine->ReadRegister(5);       
                int fileId = machine->ReadRegister(6); 
                int value;
                int i = 0;    
                
                OpenFile *openfile = new OpenFile(fileId);
                ASSERT(openfile != NULL);

                char buffer[128];
                do{
                    machine->ReadMem(base + i, 1, &value);
                    buffer[i] = *(char*)&value;
                    i ++;
                }while(&value != '\0' && i < size);
                buffer[size] = '\0';

                openfile = currentThread->space->getFileId(fileId);

                if(openfile == NULL) {
                    printf("Failed to Open file \"%d\".\n",fileId);
                    AdvancePC();
                    break;
                }

                if(fileId == 1 || fileId == 2){
                    openfile->WriteStdout(buffer,size);
                    AdvancePC();
                    break;
                }

                int writePos = openfile->Length();
                openfile->Seek(writePos);

                int writtenBytes = openfile->Write(buffer,size);
                if(writtenBytes == 0) 
                    printf("Write file failed!\n");
                else if(fileId != 1 & fileId != 2)
                    printf("\"%s\" has wrote in file %d succeed!\n",buffer,fileId);
                AdvancePC();
                break;
            }
            case SC_Read:{
                printf("SC_Read, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int base = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                int fileId = machine->ReadRegister(6); 

                OpenFile *openfile = currentThread->space->getFileId(fileId);

                char buffer[size+1];
                int readnum = 0;
                if(fileId == 0) 
                    readnum = openfile->ReadStdin(buffer, size);
                else 
                    readnum = openfile->Read(buffer, size);

                // printf("readnum:%d,fileId:%d,size:%d\n",readnum,fileId,size);
                for(int i = 0; i < readnum; i++)
                    machine->WriteMem(base, 1, buffer[i]);
                buffer[readnum] = '\0';

                for(int i = 0; i < readnum; i++)
                    if(buffer[i] >= 0 && buffer[i] <= 9) 
                        buffer[i] = buffer[i] + 0x30;

                char *buf = buffer;
                if(readnum > 0){
                    if(fileId != 0)
                        printf("Read file (%d) succeed! the content is \"%s\", the length is %d\n",fileId, buf, readnum);
                }
                else 
                    printf("Read file failed!\n");
                machine->WriteRegister(2,readnum);
                AdvancePC();
                break;
            }
            case SC_Close:{
                printf("SC_Close, CurrentThreadId: %d\n",(currentThread->space)->getSpaceId());
                int fileId = machine->ReadRegister(4);
                OpenFile *openfile = currentThread->space->getFileId(fileId);
                if(openfile != NULL) {
                    openfile->WriteBack(); 
                    delete openfile;
                    currentThread->space->releaseFileDescriptor(fileId);
                    printf("File %d closed succeed!\n",fileId);
                }
                else 
                    printf("Failed to Close File %d.\n",fileId);
                AdvancePC();
                break;
            }
#endif            
            default:{
                printf("Unexpected user mode exception %d %d\n", which, type);
                ASSERT(FALSE);
                break;
            }
        }
    }
    else{
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }


}
