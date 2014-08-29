/* fast SSE2 memchr using 64 byte loop and pmaxub instruction. */

/* Copyright (C) 2011-2014 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

.text
.global __memchr
.global memchr
.type __memchr,@function
.type memchr,@function
__memchr:
memchr:
    movd	%rsi, %xmm1
    mov	%rdi, %rcx

    punpcklbw %xmm1, %xmm1
    test	%rdx, %rdx
    jz	return_null
    punpcklbw %xmm1, %xmm1

    and	$63, %rcx
    pshufd	$0, %xmm1, %xmm1

    cmp	$48, %rcx
    ja	crosscache

    movdqu	(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb %xmm0, %eax
    test	%eax, %eax

    jnz	matches_1
    sub	$16, %rdx
    jbe	return_null
    add	$16, %rdi
    and	$15, %rcx
    and	$-16, %rdi
    add	%rcx, %rdx
    sub	$64, %rdx
    jbe	exit_loop
    jmp	loop_prolog

    .p2align 4
crosscache:
    and	$15, %rcx
    and	$-16, %rdi
    movdqa	(%rdi), %xmm0

    pcmpeqb	%xmm1, %xmm0
/* Check if there is a match.  */
    pmovmskb %xmm0, %eax
/* Remove the leading bytes.  */
    sar	%cl, %eax
    test	%eax, %eax
    je	unaligned_no_match
/* Check which byte is a match.  */
    bsf	%eax, %eax

    sub	%rax, %rdx
    jbe	return_null
    add	%rdi, %rax
    add	%rcx, %rax
    ret

    .p2align 4
unaligned_no_match:
    add	%rcx, %rdx
    sub	$16, %rdx
    jbe	return_null
    add	$16, %rdi
    sub	$64, %rdx
    jbe	exit_loop

    .p2align 4
loop_prolog:
    movdqa	(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb %xmm0, %eax
    test	%eax, %eax
    jnz	matches

    movdqa	16(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb %xmm2, %eax
    test	%eax, %eax
    jnz	matches16

    movdqa	32(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb %xmm3, %eax
    test	%eax, %eax
    jnz	matches32

    movdqa	48(%rdi), %xmm4
    pcmpeqb	%xmm1, %xmm4
    add	$64, %rdi
    pmovmskb %xmm4, %eax
    test	%eax, %eax
    jnz	matches0

    test	$0x3f, %rdi
    jz	align64_loop

    sub	$64, %rdx
    jbe	exit_loop

    movdqa	(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb %xmm0, %eax
    test	%eax, %eax
    jnz	matches

    movdqa	16(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb %xmm2, %eax
    test	%eax, %eax
    jnz	matches16

    movdqa	32(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb %xmm3, %eax
    test	%eax, %eax
    jnz	matches32

    movdqa	48(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb %xmm3, %eax

    add	$64, %rdi
    test	%eax, %eax
    jnz	matches0

    mov	%rdi, %rcx
    and	$-64, %rdi
    and	$63, %rcx
    add	%rcx, %rdx

    .p2align 4
align64_loop:
    sub	$64, %rdx
    jbe	exit_loop
    movdqa	(%rdi), %xmm0
    movdqa	16(%rdi), %xmm2
    movdqa	32(%rdi), %xmm3
    movdqa	48(%rdi), %xmm4

    pcmpeqb	%xmm1, %xmm0
    pcmpeqb	%xmm1, %xmm2
    pcmpeqb	%xmm1, %xmm3
    pcmpeqb	%xmm1, %xmm4

    pmaxub	%xmm0, %xmm3
    pmaxub	%xmm2, %xmm4
    pmaxub	%xmm3, %xmm4
    pmovmskb %xmm4, %eax

    add	$64, %rdi

    test	%eax, %eax
    jz	align64_loop

    sub	$64, %rdi

    pmovmskb %xmm0, %eax
    test	%eax, %eax
    jnz	matches

    pmovmskb %xmm2, %eax
    test	%eax, %eax
    jnz	matches16

    movdqa	32(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3

    pcmpeqb	48(%rdi), %xmm1
    pmovmskb %xmm3, %eax
    test	%eax, %eax
    jnz	matches32

    pmovmskb %xmm1, %eax
    bsf	%eax, %eax
    lea	48(%rdi, %rax), %rax
    ret

    .p2align 4
exit_loop:
    add	$32, %rdx
    jle	exit_loop_32

    movdqa	(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb %xmm0, %eax
    test	%eax, %eax
    jnz	matches

    movdqa	16(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb %xmm2, %eax
    test	%eax, %eax
    jnz	matches16

    movdqa	32(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb %xmm3, %eax
    test	%eax, %eax
    jnz	matches32_1
    sub	$16, %rdx
    jle	return_null

    pcmpeqb	48(%rdi), %xmm1
    pmovmskb %xmm1, %eax
    test	%eax, %eax
    jnz	matches48_1
    xor	%rax, %rax
    ret

    .p2align 4
exit_loop_32:
    add	$32, %rdx
    movdqa	(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb %xmm0, %eax
    test	%eax, %eax
    jnz	matches_1
    sub	$16, %rdx
    jbe	return_null

    pcmpeqb	16(%rdi), %xmm1
    pmovmskb %xmm1, %eax
    test	%eax, %eax
    jnz	matches16_1
    xor	%rax, %rax
    ret

    .p2align 4
matches0:
    bsf	%eax, %eax
    lea	-16(%rax, %rdi), %rax
    ret

    .p2align 4
matches:
    bsf	%eax, %eax
    add	%rdi, %rax
    ret

    .p2align 4
matches16:
    bsf	%eax, %eax
    lea	16(%rax, %rdi), %rax
    ret

    .p2align 4
matches32:
    bsf	%eax, %eax
    lea	32(%rax, %rdi), %rax
    ret

    .p2align 4
matches_1:
    bsf	%eax, %eax
    sub	%rax, %rdx
    jbe	return_null
    add	%rdi, %rax
    ret

    .p2align 4
matches16_1:
    bsf	%eax, %eax
    sub	%rax, %rdx
    jbe	return_null
    lea	16(%rdi, %rax), %rax
    ret

    .p2align 4
matches32_1:
    bsf	%eax, %eax
    sub	%rax, %rdx
    jbe	return_null
    lea	32(%rdi, %rax), %rax
    ret

    .p2align 4
matches48_1:
    bsf	%eax, %eax
    sub	%rax, %rdx
    jbe	return_null
    lea	48(%rdi, %rax), %rax
    ret

    .p2align 4
return_null:
    xor	%rax, %rax
    ret


.text
.global __memrchr
.global memrchr
.type __memrchr,@function
.type memrchr,@function
__memrchr:
memrchr:
    movd	%rsi, %xmm1

    sub	$16, %rdx
    jbe	r_length_less16

    punpcklbw	%xmm1, %xmm1
    punpcklbw	%xmm1, %xmm1

    add	%rdx, %rdi
    pshufd	$0, %xmm1, %xmm1

    movdqu	(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0

/* Check if there is a match.  */
    pmovmskb	%xmm0, %eax
    test	%eax, %eax
    jnz	r_matches0

    sub	$64, %rdi
    mov	%rdi, %rcx
    and	$15, %rcx
    jz	r_loop_prolog

    add	$16, %rdi
    add	$16, %rdx
    and	$-16, %rdi
    sub	%rcx, %rdx

    .p2align 4
r_loop_prolog:
    sub	$64, %rdx
    jbe	r_exit_loop

    movdqa	48(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb	%xmm0, %eax
    test	%eax, %eax
    jnz	r_matches48

    movdqa	32(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb	%xmm2, %eax
    test	%eax, %eax
    jnz	r_matches32

    movdqa	16(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb	%xmm3, %eax
    test	%eax, %eax
    jnz	r_matches16

    movdqa	(%rdi), %xmm4
    pcmpeqb	%xmm1, %xmm4
    pmovmskb	%xmm4, %eax
    test	%eax, %eax
    jnz	r_matches0

    sub	$64, %rdi
    sub	$64, %rdx
    jbe	r_exit_loop

    movdqa	48(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb	%xmm0, %eax
    test	%eax, %eax
    jnz	r_matches48

    movdqa	32(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb	%xmm2, %eax
    test	%eax, %eax
    jnz	r_matches32

    movdqa	16(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb	%xmm3, %eax
    test	%eax, %eax
    jnz	r_matches16

    movdqa	(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb	%xmm3, %eax
    test	%eax, %eax
    jnz	r_matches0

    mov	%rdi, %rcx
    and	$63, %rcx
    jz	r_align64_loop

    add	$64, %rdi
    add	$64, %rdx
    and	$-64, %rdi
    sub	%rcx, %rdx

    .p2align 4
r_align64_loop:
    sub	$64, %rdi
    sub	$64, %rdx
    jbe	r_exit_loop

    movdqa	(%rdi), %xmm0
    movdqa	16(%rdi), %xmm2
    movdqa	32(%rdi), %xmm3
    movdqa	48(%rdi), %xmm4

    pcmpeqb	%xmm1, %xmm0
    pcmpeqb	%xmm1, %xmm2
    pcmpeqb	%xmm1, %xmm3
    pcmpeqb	%xmm1, %xmm4

    pmaxub	%xmm3, %xmm0
    pmaxub	%xmm4, %xmm2
    pmaxub	%xmm0, %xmm2
    pmovmskb	%xmm2, %eax

    test	%eax, %eax
    jz	r_align64_loop

    pmovmskb	%xmm4, %eax
    test	%eax, %eax
    jnz	r_matches48

    pmovmskb	%xmm3, %eax
    test	%eax, %eax
    jnz	r_matches32

    movdqa	16(%rdi), %xmm2

    pcmpeqb	%xmm1, %xmm2
    pcmpeqb	(%rdi), %xmm1

    pmovmskb	%xmm2, %eax
    test	%eax, %eax
    jnz	r_matches16

    pmovmskb	%xmm1, %eax
    bsr	%eax, %eax

    add	%rdi, %rax
    ret

    .p2align 4
r_exit_loop:
    add	$64, %rdx
    cmp	$32, %rdx
    jbe	r_exit_loop_32

    movdqa	48(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb	%xmm0, %eax
    test	%eax, %eax
    jnz	r_matches48

    movdqa	32(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb	%xmm2, %eax
    test	%eax, %eax
    jnz	r_matches32

    movdqa	16(%rdi), %xmm3
    pcmpeqb	%xmm1, %xmm3
    pmovmskb	%xmm3, %eax
    test	%eax, %eax
    jnz	r_matches16_1
    cmp	$48, %rdx
    jbe	r_return_null

    pcmpeqb	(%rdi), %xmm1
    pmovmskb	%xmm1, %eax
    test	%eax, %eax
    jnz	r_matches0_1
    xor	%eax, %eax
    ret

    .p2align 4
r_exit_loop_32:
    movdqa	48(%rdi), %xmm0
    pcmpeqb	%xmm1, %xmm0
    pmovmskb	%xmm0, %eax
    test	%eax, %eax
    jnz	r_matches48_1
    cmp	$16, %rdx
    jbe	r_return_null

    pcmpeqb	32(%rdi), %xmm1
    pmovmskb	%xmm1, %eax
    test	%eax, %eax
    jnz	r_matches32_1
    xor	%eax, %eax
    ret

    .p2align 4
r_matches0:
    bsr	%eax, %eax
    add	%rdi, %rax
    ret

    .p2align 4
r_matches16:
    bsr	%eax, %eax
    lea	16(%rax, %rdi), %rax
    ret

    .p2align 4
r_matches32:
    bsr	%eax, %eax
    lea	32(%rax, %rdi), %rax
    ret

    .p2align 4
r_matches48:
    bsr	%eax, %eax
    lea	48(%rax, %rdi), %rax
    ret

    .p2align 4
r_matches0_1:
    bsr	%eax, %eax
    sub	$64, %rdx
    add	%rax, %rdx
    jl	r_return_null
    add	%rdi, %rax
    ret

    .p2align 4
r_matches16_1:
    bsr	%eax, %eax
    sub	$48, %rdx
    add	%rax, %rdx
    jl	r_return_null
    lea	16(%rdi, %rax), %rax
    ret

    .p2align 4
r_matches32_1:
    bsr	%eax, %eax
    sub	$32, %rdx
    add	%rax, %rdx
    jl	r_return_null
    lea	32(%rdi, %rax), %rax
    ret

    .p2align 4
r_matches48_1:
    bsr	%eax, %eax
    sub	$16, %rdx
    add	%rax, %rdx
    jl	r_return_null
    lea	48(%rdi, %rax), %rax
    ret

    .p2align 4
r_return_null:
    xor	%rax, %rax
    ret

    .p2align 4
r_length_less16_offset0:
    mov	%dl, %cl
    pcmpeqb	(%rdi), %xmm1

    mov	$1, %edx
    sal	%cl, %edx
    sub	$1, %edx

    pmovmskb	%xmm1, %eax

    and	%edx, %eax
    test	%eax, %eax
    jz	r_return_null

    bsr	%eax, %eax
    add	%rdi, %rax
    ret

    .p2align 4
r_length_less16:
    punpcklbw	%xmm1, %xmm1
    punpcklbw	%xmm1, %xmm1

    add	$16, %rdx

    pshufd	$0, %xmm1, %xmm1

    mov	%rdi, %rcx
    and	$15, %rcx
    jz	r_length_less16_offset0

    mov	%rdi, %rcx
    and	$15, %rcx
    mov	%cl, %dh
    mov	%rcx, %r8
    add	%dl, %dh
    and	$-16, %rdi

    sub	$16, %dh
    ja	r_length_less16_part2

    pcmpeqb	(%rdi), %xmm1
    pmovmskb	%xmm1, %eax

    sar	%cl, %eax
    mov	%dl, %cl

    mov	$1, %edx
    sal	%cl, %edx
    sub	$1, %edx

    and	%edx, %eax
    test	%eax, %eax
    jz	r_return_null

    bsr	%eax, %eax
    add	%rdi, %rax
    add	%r8, %rax
    ret

    .p2align 4
r_length_less16_part2:
    movdqa	16(%rdi), %xmm2
    pcmpeqb	%xmm1, %xmm2
    pmovmskb	%xmm2, %eax

    mov	%dh, %cl
    mov	$1, %edx
    sal	%cl, %edx
    sub	$1, %edx

    and	%edx, %eax

    test	%eax, %eax
    jnz	r_length_less16_part2_return

    pcmpeqb	(%rdi), %xmm1
    pmovmskb	%xmm1, %eax

    mov	%r8, %rcx
    sar	%cl, %eax
    test	%eax, %eax
    jz	r_return_null

    bsr	%eax, %eax
    add	%rdi, %rax
    add	%r8, %rax
    ret

    .p2align 4
r_length_less16_part2_return:
    bsr	%eax, %eax
    lea	16(%rax, %rdi), %rax
    ret

