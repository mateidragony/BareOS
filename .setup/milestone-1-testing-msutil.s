	.globl 	_ms_recovery_status
	.equ REGSZ, 8

_ms_recovery_space:
	.dword 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
_ms_recovery_status:
	.word 0

	.globl 	_ms_recover
_ms_recover:
	sw	a0,_ms_recovery_status,t1   # -. Set the status global to `1` and long jumps to the termination of `_ms_safe_call`
	j 	_ms_rcv_ptr                 # -'

	.globl _ms_safe_call
_ms_safe_call:
	la 	t0,_ms_recovery_space
	sd	ra,0*REGSZ(t0)
	sd	s0,1*REGSZ(t0)
	sd 	s1,2*REGSZ(t0)
	sd	s2,3*REGSZ(t0)
	sd	s3,4*REGSZ(t0)
	sd	s4,5*REGSZ(t0)
	sd	s5,6*REGSZ(t0)
	sd	s6,7*REGSZ(t0)
	sd	s7,8*REGSZ(t0)
	sd	s8,9*REGSZ(t0)
	sd	s9,10*REGSZ(t0)
	sd	s10,11*REGSZ(t0)
	sd	s11,12*REGSZ(t0)
	sd	a0,13*REGSZ(t0)
	sd	a1,14*REGSZ(t0)
	sd	a2,15*REGSZ(t0)
	sd	a3,16*REGSZ(t0)
	sd	a4,17*REGSZ(t0)
	sd	a5,18*REGSZ(t0)
	sd	a6,19*REGSZ(t0)
	sd	a7,20*REGSZ(t0)
	sd 	sp,21*REGSZ(t0)

	addi    t0,a0,0     # --
	addi	a0,a1,0     #  |
	addi 	a1,a2,0     #  |
	addi	a2,a3,0     #  |  Shift arguments into position for the protected call
	addi	a3,a4,0     #  |
	addi	a4,a5,0     #  |
	addi	a5,a6,0     #  |
	addi 	a6,a7,0     # --

	li	t1, 0x0                    # --
	sw	t1,_ms_recovery_status,t2  #  |  Call the protected function and jump to `__ms_complete_recover` when done
	jalr 	t0                         #  |
	j	 __ms_complete_recover     # --
_ms_rcv_ptr:
	addi 	a0,zero,-1            # -- Fix stack pointer after long jump
__ms_complete_recover:
	la	t0,_ms_recovery_space
	ld	ra,0*REGSZ(t0)
	ld	s0,1*REGSZ(t0)
	ld 	s1,2*REGSZ(t0)
	ld	s2,3*REGSZ(t0)
	ld	s3,4*REGSZ(t0)
	ld	s4,5*REGSZ(t0)
	ld	s5,6*REGSZ(t0)
	ld	s6,7*REGSZ(t0)
	ld	s7,8*REGSZ(t0)
	ld	s8,9*REGSZ(t0)
	ld	s9,10*REGSZ(t0)
	ld	s10,11*REGSZ(t0)
	ld	s11,12*REGSZ(t0)
	ld	a1,14*REGSZ(t0)
	ld	a2,15*REGSZ(t0)
	ld	a3,16*REGSZ(t0)
	ld	a4,17*REGSZ(t0)
	ld	a5,18*REGSZ(t0)
	ld	a6,19*REGSZ(t0)
	ld	a7,20*REGSZ(t0)
	ld 	sp,21*REGSZ(t0)
	ret
