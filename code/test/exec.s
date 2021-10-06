	.file	1 "exec.c"
gcc2_compiled.:
__gnu_compiled_c:
	.rdata
	.align	2
$LC0:
	.ascii	"../test/halt.noff\000"	# 用户地址空间
	.text
	.align	2						# 2 字节对齐
	.globl	main 					# 全局变量
	.ent	main
main:
	# frame 用来声明堆栈布局：
	# 第一个参数：声明用于访问局部堆栈读寄存器
	# 第二个参数：声明该函数已分配堆栈读大小
	# 第三个参数：用来保存返回地址的寄存器
	.frame	$fp,32,$31		# vars= 8, regs= 2/0, args= 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	
	# 栈采用向下生长的方式，即由大地址向小地址生长，栈指针指向栈的最小地址
	subu	$sp,$sp,32		# $sp - 32->$sp，构造mian()的栈frmae
							# $sp的原值应该是执行main()之前的栈
							# 上一函数对应栈frmae的顶（最小地址处）
	sw	$31,28($sp)			# $31->memory[$sp+28]
	sw	$fp,24($sp)			# $fp ->memory[$sp+24] 
	move	$fp,$sp 		# $sp->$fp，执行Exec()会修改$sp

	jal	__main  			# PC+4->$31, goto __main

	la	$4,$LC0				# $LC0->$4 ,将Exec的参数的地址传给 $4
             				# $4-$7：传递函数的前四个参数给子程序，不够的用堆栈

	jal	Exec 				# 转到start.s中的Exec处执行
							# PC+4->$31, goto Exec；
							# PC是调用函数时的指令地址，
							# PC+4是函数的下条指令地址，以便从函数返回时从调用
							# 函数的下条指令开始继续执行原程序

	sw	$2,16($fp)			# $2->memory[$fp+16]，Exec()的返回值
							# $2,$3: 存放函数的返回值，当这两个寄存器不够存放
							# 返回值时，编译器通过内存来完成。
							# $sp一直指向main()对应stack frame的栈顶（最小地址）
							# 由于在调用Exec()时要用到$sp，前面将$sp->$fp，
							# 因此$fp也是指向main()对应stack frame的栈顶

	jal	Halt 				# PC+4->$31, goto Halt，Halt()无参，无返回值

$L1:
	move	$sp,$fp 		# $fp->$sp
	lw	$31,28($sp) 		# memory[/$sp+28]->$31，取main()的返回值
	lw	$fp,24($sp) 		# memory[$sp+24]->$fp，恢复$fp
	addu	$sp,$sp,32 		# $sp+32->$sp，释放main()对应的在栈中的frame

	j	$31					# goto $31, mian()函数返回
							# $31: Return address for subrouting

	.end	main
