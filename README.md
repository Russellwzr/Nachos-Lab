## 实验总结

操作系统课程设计到此为止也算告一段落了，回顾过去的几个实验，我通过阅读分析Nachos系统的实现代码并基于源码来对原系统进行扩充，从多个角度更加深刻地理解了操作系统内核的工作原理以及具体的实现细节：

实验一主要是安装配置Nachos系统并测试相关功能，通过Nachos相关源代码的阅读，我理解了Nachos的启动、运行、终止等的工作原理，又通过跟踪Nachos线程的上下文切换过程，学习并掌握了GDB调试工具的基本使用方法及其相关应用。

实验三则主要在Nachos实现的信号量的基础上模拟了生产者消费者问题，理解了Nachos中实现线程同步的方法。此外，通过阅读Thread类、Scheduler类、Interrupt类、Timer类的代码，我也从较为底层的角度理解了Nachos系统对于线程、调度、中断、计时器以及它们相关算法的实现。

实验四与实验五，我首先通过观察项目文件之间的相互调用关系，从宏观上理解了Nachos文件系统不同模块之间的组织关系，然后又通过阅读不同模块代码的具体实现，深入理解了Nachos文件系统的模拟方式、组织策略以及实现原理。之后又对Nachos文件系统进行测试，以不同格式与要求输出Nachos硬盘信息，将输出结果与实际执行命令、代码实现相结合，进一步理解了Nachos文件系统相关命令的实现方法。在阅读理解Nachos文件系统相关代码的过程中，我还找出了原有系统的一些缺陷，然后针对这些缺陷，基于原有系统，我又相继完成了-ap, -hap, -nap 等命令，实现了对Nachos文件系统的扩展。最后我又对Nachos文件系统的目录结构进行了扩展，将原有的单级目录扩展为目录树实现的多级目录，增加了Nachos文件系统的灵活性。

实验六则首先通过阅读Makefile文件理解了Nachos用户程序的交叉编译流程，然后又通过阅读相关类的声明与实现，理解了Nachos可执行文件的格式与结构、页表的设计与实现、Instruction类以及Machine类对指令和机器的模拟等，最后又通过代码跟踪与运行调试，理解了用户进程的创建与执行过程以及核心线程执行用户程序的原理，了解了用户进程通过系统调用与Nachos内核进行交互的方法，为后续设计与实现不同的系统调用方法奠定了基础。

实验七与实验八则首先通过扩展进程的地址空间，使Nachos系统支持多进程机制，然后又基于对Nachos系统调用的执行流程、入口汇编代码、参数传递方法等的理解，对源码进行修改，实现了Join()、Exec()、Exit() 等15种系统调用，理解了系统调用与操作系统内核进行交互的工作原理与实现细节，也在设计与实现相关算法的过程中，对操作系统的进程管理机制有了更加深入的理解与认识。

## 实验任务

### 实验一

（1）安装 Linux 操作系统 <br/>
（2）安装 Nachos 及 gcc mips 交叉编译程序<br/>
（3）编译测试 Nachos<br/>
（4）熟悉 gdb 调试工具<br/>
（5）熟悉 Nachos 中的上下文切换过程<br/>

### 实验三

（1）分析../threads/threadtest.cc，理解利用Thread::Fork()创建线程的方法及过程<br/>
（2）分析../threads/synch.cc，理解Nachos中信号量是如何实现的<br/>
（3）分析../monitor/prodcons++.cc，理解信号量的创建与使用方法<br/>
（4）分析Thread::Fork(), Thread::Yiled(), Thread::Sleep(), 
Thread::Finish(), Scheduler::ReadyToRun(), Scheduler::FindNextToRun(), Scheduler::Run()等相关函数，理解线程调度及上下文切换的工作过程<br/>
（5）分析Nachos对参数 -rs 的处理过程，理解时钟中断的实现，以及RR调度算法的实现方法<br/>
（6）阅读code/lab3目录下的ring,h、ring.cc、main.cc及prodcons++.cc，在理解它们工作机理的基础上，补充目录lab3中提供的代码，利用Nachos实现的信号量写一个producer/consumer problem测试程序<br/>

### 实验四

（1）../lab5/main.cc调用了../threads/system.cc中的Initialize()创建了硬盘DISK。分析../threads/ synchdisk.cc及../machine/disk.cc，理解Nachos创建硬盘的过程与方法；
<br/>（2）分析../lab5/main.cc，了解Nachos文件系统提供了哪些命令，对每个命令进行测试，根据执行结果观察哪些命令已经实现（正确运行），哪些无法正确运行（尚未完全实现，需要你自己完善）；分析../lab5/fstest.cc及../filesys/filessys.cc，理解Nachos对这些命令的处理过程与方法；
<br/>（3）分析../filesys/filessys.cc，特别是构造函数FileSystem::FileSystem(..)，理解Nachos硬盘”DISK”的创建及硬盘格式化（创建文件系统）的处理过程；
<br/>（4）利用命令hexdump –C DISK查看硬盘格式化后硬盘的布局，理解格式化硬盘所完成的工作，以及文件系统管理涉及到的一些数据结构组织与使用，如文件头（FCB）、目录表与目录项、空闲块管理位示图等；
结合输出结果，分析FileSystem::FileSystem(..)初始化文件系统时涉及到的几个模块，如../filesys/filehdr.h(filehdr.cc)，directory.h(directory.cc)，../userprog/bitmap.h（bitmap.cc），理解文件头（FCB）的结构与组织、硬盘空闲块管理使用的位示图文件、目录表文件及目录下的组织与结构，以及它们在硬盘上的位置；
<br/>（5）利用命令nachos –cp ../test/small samll复制文件../test/small到硬盘DISK中；
<br/>（6）利用命令hexdump –C DISK查看硬盘格式化后硬盘的布局，理解创建一个文件后相关的结构在硬盘上的存储布局；
<br/>（7）复制更多的文件到DISK中，然后删除一个文件，利用hexdump –C DISK查看文件的布局，分析文件系统的管理策略。

### 实验五

修改Nachos的文件系统，以满足：
<br/>（1）文件创建时，其大小可初始化为0；
<br/>（2）当一个文件写入更多的数据时，其大小可随之增大；
<br/>（3）要求能够在从一个文件的任何位置开始写入数据，即能够正确处理命令行参数 -ap, -hap,及-nap；

### 实验六

（1）阅读../bin/noff.h，分析Nachos可执行程序.noff文件的格式组成；
<br/>（2）阅读../test目录下的几个Nachos应用程序，理解Nachos应用程序的编程语法，了解用户进程是如何通过系统调用与操作系统内核进行交互的；
<br/>（3） 阅读../test/Makefile，掌握如何利用交叉编译生成Nachos的可执行程序；
<br/>（4）阅读../threads/main.cc，../userprog/ progtest.cc，根据对命令行参数-x的处理过程，理解系统如何为应用程序创建进程，并启动进程的；
<br/>（5）阅读../userprog/ progtest.cc，../threads/scheduler.cc（Run()），理解如何将用户线程映射到核心线程，以及核心线程执行用户程序的原理与方法；
<br/>（6）阅读../userprog/ progtest.cc，../machine/translate.cc，理解当前进程的页表是如何与CPU使用的页表进行关联的；

### 实验七

（1）阅读../prog/protest.cc，深入理解Nachos创建应用程序进程的详细过程；
<br/>（2）阅读理解类AddrSpace，然后对其进行修改，使Nachos能够支持多进程机制，允许Nachos同时运行多个用户线程；
<br/>（3）在类AddrSpace中添加完善Print()函数；
<br/>（4）在类AddrSpace中实例化类Bitmap的一个全局对象，用于管理空闲帧；
<br/>（5）如果将SpaceId直接作为进程号Pid是否合适？如果感觉不是很合适，应该如何为进程分配相应的pid？
<br/>（6）为实现Join(pid)，考虑如何在该进程相关联的核心线程中保存进程号；
<br/>（7）根据进程创建时系统为其所做的工作，考虑进程退出时应该做哪些工作；
<br/>（8）考虑系统调用Exec()与Exit()的设计实现方案；

### 实验八

（1）阅读../userprog/exception.cc，理解系统调用Halt() 的实现原理；
<br/>（2）基于实验6、7中所完成的工作，利用Nachos提供的文件管理、内存管理及线程管理等功能，编程实现 Exec() 与 Exit() 等系统调用；
