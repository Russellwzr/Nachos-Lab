	.file	1 "halt.c"
gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	main
	.ent	main
main:						# vars= 16, regs= 2/0, args= 16, extra= 0
	.frame	$fp,40,$31		# 返回地址为 $31  main() stack frame 被分配 40 空间大小
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,40		# 创建 stack 指针 （分配 40 空间大小）
	sw	$31,36($sp)			# 保存返回地址
	sw	$fp,32($sp)			# 保存fp:栈帧指针
	move	$fp,$sp			# $sp -> $fp 保存栈顶指针
	jal	__main				# jump to main()
	li	$2,3				# 0x00000003
	sw	$2,24($fp)			# k = 3
	li	$2,2				# 0x00000002
	sw	$2,16($fp)			# i = 2
	lw	$2,20($fp)			# $2 = j
	addu	$3,$2,-1    	# $3 = $2 - 1
	sw	$3,20($fp)			# j = $3
	lw	$2,16($fp)			# $2 = i
	lw	$3,20($fp)			# $3 = j
	subu	$2,$2,$3		# $2 = $2 - $3 (i - j)
	lw	$3,24($fp)			# $3 = k
	addu	$2,$3,$2		# $2 = $2 + $3 (i - j + k)
	sw	$2,24($fp)			# k = $2
	jal	Halt 				# jump to Halt()
$L1:
	move	$sp,$fp			# $fp -> $sp 恢复栈顶指针
	lw	$31,36($sp)			# 恢复 $31
	lw	$fp,32($sp)			# 恢复 fp 指针
	addu	$sp,$sp,40		# 撤销 stack 指针（回收分配的空间）
	j	$31					# 跳转到返回地址
	.end	main
