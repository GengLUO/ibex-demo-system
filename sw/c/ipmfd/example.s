	.file	"Main.c"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	2
.LC0:
	.string	"--------IPM-FD n = %d, k = %d--------\n "
	.align	2
.LC1:
	.string	""
	.align	2
.LC2:
	.string	"----------------------------------\n"
	.align	2
.LC3:
	.string	"X = %d\n"
	.align	2
.LC4:
	.string	"masked data "
	.align	2
.LC5:
	.string	"Unmasked = %d\n\n"
	.align	2
.LC6:
	.string	"R:"
	.align	2
.LC7:
	.string	"Unmasked = %x\n\n"
	.align	2
.LC8:
	.string	"---------------- Test IPM_FD_Mult --------------\n"
	.align	2
.LC9:
	.string	"Y = %d\n"
	.align	2
.LC10:
	.string	"(Sharing) masked data Q "
	.align	2
.LC11:
	.string	"X*Y = %d\n"
	.align	2
.LC12:
	.string	"Z*Q "
	.align	2
.LC13:
	.string	"------------------ End of Test -----------------\n"
	.text
	.align	1
	.globl	test
	.type	test, @function
test:
	addi	sp,sp,-80
	sw	ra,76(sp)
	sw	s0,72(sp)
	addi	s0,sp,80
	call	IPM_FD_Setup
	li	a3,2
	li	a2,5
	lui	a5,%hi(.LC0)
	addi	a1,a5,%lo(.LC0)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	sw	zero,-20(s0)
	j	.L2
.L3:
	lw	a4,-20(s0)
	mv	a5,a4
	slli	a5,a5,2
	add	a5,a5,a4
	lui	a4,%hi(L)
	addi	a4,a4,%lo(L)
	add	a4,a5,a4
	lui	a5,%hi(.LC1)
	addi	a1,a5,%lo(.LC1)
	mv	a0,a4
	call	print
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L2:
	lw	a4,-20(s0)
	li	a5,1
	ble	a4,a5,.L3
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	puts
	li	a5,33
	sb	a5,-21(s0)
	lbu	a5,-21(s0)
	mv	a2,a5
	lui	a5,%hi(.LC3)
	addi	a1,a5,%lo(.LC3)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	lbu	a4,-21(s0)
	addi	a5,s0,-44
	mv	a1,a4
	mv	a0,a5
	call	mask_hardware
	addi	a4,s0,-44
	lui	a5,%hi(.LC4)
	addi	a1,a5,%lo(.LC4)
	mv	a0,a4
	call	print
	addi	a5,s0,-44
	mv	a0,a5
	call	unmask_hardware
	mv	a5,a0
	sb	a5,-21(s0)
	lbu	a5,-21(s0)
	mv	a2,a5
	lui	a5,%hi(.LC5)
	addi	a1,a5,%lo(.LC5)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	li	a5,55
	sb	a5,-22(s0)
	lbu	a4,-22(s0)
	addi	a5,s0,-52
	mv	a1,a4
	mv	a0,a5
	call	mask_hardware
	addi	a3,s0,-52
	addi	a4,s0,-44
	addi	a5,s0,-60
	mv	a2,a3
	mv	a1,a4
	mv	a0,a5
	call	IPM_FD_Mult_hardware
	addi	a4,s0,-60
	lui	a5,%hi(.LC6)
	addi	a1,a5,%lo(.LC6)
	mv	a0,a4
	call	print
	addi	a5,s0,-60
	mv	a0,a5
	call	unmask_hardware
	sw	a0,-28(s0)
	lw	a2,-28(s0)
	lui	a5,%hi(.LC7)
	addi	a1,a5,%lo(.LC7)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	lui	a5,%hi(.LC8)
	addi	a0,a5,%lo(.LC8)
	call	puts
	lbu	a5,-21(s0)
	sb	a5,-29(s0)
	lbu	a5,-29(s0)
	mv	a2,a5
	lui	a5,%hi(.LC9)
	addi	a1,a5,%lo(.LC9)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	lbu	a4,-29(s0)
	addi	a5,s0,-68
	mv	a1,a4
	mv	a0,a5
	call	mask_hardware
	addi	a4,s0,-68
	lui	a5,%hi(.LC10)
	addi	a1,a5,%lo(.LC10)
	mv	a0,a4
	call	print
	addi	a5,s0,-68
	mv	a0,a5
	call	unmask_hardware
	sw	a0,-36(s0)
	lw	a2,-36(s0)
	lui	a5,%hi(.LC5)
	addi	a1,a5,%lo(.LC5)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	lbu	a4,-29(s0)
	lbu	a5,-21(s0)
	mv	a1,a4
	mv	a0,a5
	call	GF256_Mult
	mv	a5,a0
	mv	a2,a5
	lui	a5,%hi(.LC11)
	addi	a1,a5,%lo(.LC11)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	addi	a3,s0,-68
	addi	a4,s0,-44
	addi	a5,s0,-76
	mv	a2,a3
	mv	a1,a4
	mv	a0,a5
	call	IPM_FD_Mult_hardware
	addi	a4,s0,-76
	lui	a5,%hi(.LC12)
	addi	a1,a5,%lo(.LC12)
	mv	a0,a4
	call	print
	addi	a5,s0,-76
	mv	a0,a5
	call	unmask_hardware
	sw	a0,-36(s0)
	lw	a2,-36(s0)
	lui	a5,%hi(.LC5)
	addi	a1,a5,%lo(.LC5)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	lui	a5,%hi(.LC13)
	addi	a0,a5,%lo(.LC13)
	call	puts
	nop
	lw	ra,76(sp)
	lw	s0,72(sp)
	addi	sp,sp,80
	jr	ra
	.size	test, .-test
	.section	.rodata
	.align	2
.LC14:
	.string	"\n-------------AES CIPHER------------\n"
	.align	2
.LC18:
	.string	"in\t"
	.align	2
.LC19:
	.string	"key\t"
	.align	2
.LC20:
	.string	"Without countermeasure:\n"
	.align	2
.LC21:
	.string	"-----------Pure Software-----------\n"
	.align	2
.LC22:
	.string	"\nWith extended IPM-FD n = %d, k = %d :\n"
	.align	2
.LC23:
	.string	"Faults detected => need to restart over\n"
	.align	2
.LC24:
	.string	"out\t"
	.align	2
.LC25:
	.string	"outex\t"
	.align	2
.LC26:
	.string	"-----------------------------------\n"
	.align	2
.LC27:
	.string	"--------------Hardware-------------\n"
	.align	2
.LC28:
	.string	"-----------------------------------\n\n"
	.align	2
.LC15:
	.string	""
	.ascii	"\001\002\003\004\005\006\007\b\t\n\013\f\r\016\017"
	.align	2
.LC16:
	.string	""
	.ascii	"\021\"3DUfw\210\231\252\273\314\335\356\377"
	.align	2
.LC17:
	.ascii	"i\304\340\330j{\0040\330\315\267\200p\264\305Z"
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-96
	sw	ra,92(sp)
	sw	s0,88(sp)
	addi	s0,sp,96
	lui	a5,%hi(.LC14)
	addi	a0,a5,%lo(.LC14)
	call	puts
	lui	a5,%hi(.LC15)
	addi	a5,a5,%lo(.LC15)
	lw	a2,0(a5)
	lw	a3,4(a5)
	lw	a4,8(a5)
	lw	a5,12(a5)
	sw	a2,-44(s0)
	sw	a3,-40(s0)
	sw	a4,-36(s0)
	sw	a5,-32(s0)
	lui	a5,%hi(.LC16)
	addi	a5,a5,%lo(.LC16)
	lw	a2,0(a5)
	lw	a3,4(a5)
	lw	a4,8(a5)
	lw	a5,12(a5)
	sw	a2,-60(s0)
	sw	a3,-56(s0)
	sw	a4,-52(s0)
	sw	a5,-48(s0)
	lui	a5,%hi(.LC17)
	addi	a5,a5,%lo(.LC17)
	lw	a2,0(a5)
	lw	a3,4(a5)
	lw	a4,8(a5)
	lw	a5,12(a5)
	sw	a2,-76(s0)
	sw	a3,-72(s0)
	sw	a4,-68(s0)
	sw	a5,-64(s0)
	addi	a5,s0,-60
	mv	a1,a5
	lui	a5,%hi(.LC18)
	addi	a0,a5,%lo(.LC18)
	call	printMes
	addi	a5,s0,-44
	mv	a1,a5
	lui	a5,%hi(.LC19)
	addi	a0,a5,%lo(.LC19)
	call	printMes
	sw	zero,-20(s0)
	j	.L5
.L6:
	lw	a5,-20(s0)
	addi	a4,s0,-16
	add	a5,a4,a5
	sb	zero,-76(a5)
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L5:
	lw	a4,-20(s0)
	li	a5,15
	ble	a4,a5,.L6
	li	a5,16
	sw	a5,-24(s0)
	li	a5,-1
	sw	a5,-96(s0)
	li	a5,1
	sw	a5,-28(s0)
	lui	a5,%hi(.LC20)
	addi	a0,a5,%lo(.LC20)
	call	puts
	addi	a2,s0,-44
	addi	a4,s0,-92
	addi	a5,s0,-60
	lw	a3,-28(s0)
	mv	a1,a4
	mv	a0,a5
	call	run_aes
	mv	a4,a0
	lui	a5,%hi(time_costs)
	addi	a5,a5,%lo(time_costs)
	sw	a4,0(a5)
	addi	a4,s0,-76
	addi	a5,s0,-92
	lw	a2,-24(s0)
	mv	a1,a4
	mv	a0,a5
	call	check_ciphertext
	lui	a5,%hi(.LC21)
	addi	a0,a5,%lo(.LC21)
	call	puts
	sw	zero,-20(s0)
	j	.L7
.L8:
	lw	a5,-20(s0)
	addi	a4,s0,-16
	add	a5,a4,a5
	sb	zero,-76(a5)
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L7:
	lw	a4,-20(s0)
	lw	a5,-24(s0)
	blt	a4,a5,.L8
	li	a3,2
	li	a2,5
	lui	a5,%hi(.LC22)
	addi	a1,a5,%lo(.LC22)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	call	IPM_FD_Setup
	addi	a3,s0,-96
	addi	a2,s0,-44
	addi	a1,s0,-92
	addi	a5,s0,-60
	lw	a4,-28(s0)
	mv	a0,a5
	call	run_aes_share
	mv	a4,a0
	lui	a5,%hi(time_costs)
	addi	a5,a5,%lo(time_costs)
	sw	a4,4(a5)
	lw	a5,-96(s0)
	bne	a5,zero,.L9
	lui	a5,%hi(.LC23)
	addi	a0,a5,%lo(.LC23)
	call	puts
.L9:
	addi	a4,s0,-76
	addi	a5,s0,-92
	lw	a2,-24(s0)
	mv	a1,a4
	mv	a0,a5
	call	check_ciphertext
	mv	a5,a0
	beq	a5,zero,.L10
	addi	a5,s0,-92
	mv	a1,a5
	lui	a5,%hi(.LC24)
	addi	a0,a5,%lo(.LC24)
	call	printMes
	addi	a5,s0,-76
	mv	a1,a5
	lui	a5,%hi(.LC25)
	addi	a0,a5,%lo(.LC25)
	call	printMes
.L10:
	lui	a5,%hi(.LC26)
	addi	a0,a5,%lo(.LC26)
	call	puts
	lui	a5,%hi(.LC27)
	addi	a0,a5,%lo(.LC27)
	call	puts
	sw	zero,-20(s0)
	j	.L11
.L12:
	lw	a5,-20(s0)
	addi	a4,s0,-16
	add	a5,a4,a5
	sb	zero,-76(a5)
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L11:
	lw	a4,-20(s0)
	lw	a5,-24(s0)
	blt	a4,a5,.L12
	li	a3,2
	li	a2,5
	lui	a5,%hi(.LC22)
	addi	a1,a5,%lo(.LC22)
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	sprintf
	lui	a5,%hi(msg)
	addi	a0,a5,%lo(msg)
	call	puts
	call	IPM_FD_Setup
	addi	a3,s0,-96
	addi	a2,s0,-44
	addi	a1,s0,-92
	addi	a5,s0,-60
	lw	a4,-28(s0)
	mv	a0,a5
	call	run_aes_share_hardware
	mv	a4,a0
	lui	a5,%hi(time_costs)
	addi	a5,a5,%lo(time_costs)
	sw	a4,8(a5)
	lw	a5,-96(s0)
	bne	a5,zero,.L13
	lui	a5,%hi(.LC23)
	addi	a0,a5,%lo(.LC23)
	call	puts
.L13:
	addi	a4,s0,-76
	addi	a5,s0,-92
	lw	a2,-24(s0)
	mv	a1,a4
	mv	a0,a5
	call	check_ciphertext
	mv	a5,a0
	beq	a5,zero,.L14
	addi	a5,s0,-92
	mv	a1,a5
	lui	a5,%hi(.LC24)
	addi	a0,a5,%lo(.LC24)
	call	printMes
	addi	a5,s0,-76
	mv	a1,a5
	lui	a5,%hi(.LC25)
	addi	a0,a5,%lo(.LC25)
	call	printMes
.L14:
	lui	a5,%hi(.LC26)
	addi	a0,a5,%lo(.LC26)
	call	puts
	call	report_cycle
	lui	a5,%hi(.LC28)
	addi	a0,a5,%lo(.LC28)
	call	puts
.L15:
	j	.L15
	.size	main, .-main
	.ident	"GCC: (crosstool-NG 1.24.0.498_5075e1f) 10.2.0"
