#   Copyright 2008-2016 Intel Corporation
#All rights reserved.
#Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:

 #   Redistributions of source code must retain the above copyright notice,
 #   this list of conditions and the following disclaimer.
 #   Redistributions in binary form must reproduce the above copyright notice,
 #   this list of conditions and the following disclaimer in the documentation
 #   and/or other materials provided with the distribution.
 #   Neither the name of the Intel Corp. nor the names of its contributors
 #   may be used to endorse or promote products derived from this software
 #   without specific prior written permission.


#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
#WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#   get_cpuid.asm
#   wrapper function to retrieve CPUID leaf and subleaf data,
#   returns CPUID leaf/subleaf raw data in a data structure
#
#  Written by Patrick Fay
#
#
#  caller supplies three parameters;
#  pointer to a data structure
#  leaf index
#  sub leaf index
#
.intel_syntax noprefix

# -------------------------- 64bit ----------------------
           .text
#get_cpuid_info PROC
#The ABI for passing integer arguments is different in windows and linux.
#In linux (that's what I used for the port), %rdi, %rsi, %rdx, %rcx, %r8
#and %r9 are used to pass INT type parameters, in windows rcx, rdx, r8 and
#r9 are used.
#
#REGISTER USAGE FOR WINDOWS:
#parameter regs        RCX, RDX, R8, R9, XMM0-XMM3
#scratch registers     RAX, RCX, RDX, R8-R11, ST(0)-ST(7), XMM0-XMM5
#callee-save registers RBX, RSI, RDI, RBP, R12-R15, xmm6-xmm15
#registers for return  RAX, XMM0

#REGISTER RULES FOR LINUX:
#parameter registers   RDI, RSI, RDX, RCX, R8, R9, XMM0-XMM7
#scratch registers     RAX, RCX, RDX, RSI, RDI, R8-R11, ST(0)-ST(7), XMM0-XMM15
#callee-save registers RBX, RBP, R12-R15
#registers for return  RAX, RDX, XMM0, XMM1, st(0), st(1)
#
# rax   rcx   rdx   rbx   rsp   rbp   rsi   rdi
# eax   ecx   edx   ebx   esp   ebp   esi   edi
#  ax    cx    dx    bx    sp    bp    si    di
#  ah    ch    dh    bh    sph?  bph?  sih?  dih?
#  al    cl    dl    bl    spl   bpl   sil   dil

           .global     get_cpuid_info
get_cpuid_info:
           mov r8, rdi   #  array addr
           mov r9, rsi   #  leaf
           mov r10, rdx  #  subleaf

           push        rax
           push        rbx
           push        rcx
           push        rdx
           mov         eax, r9d
           mov         ecx, r10d
           cpuid
           mov         DWORD PTR [r8], eax
           mov         DWORD PTR [r8+4], ebx
           mov         DWORD PTR [r8+8], ecx
           mov         DWORD PTR [r8+12], edx
           pop         rdx
           pop         rcx
           pop         rbx
           pop         rax
           ret         0
#get_cpuid_info ENDP
#_TEXT     ENDS

#if defined __ELF__ && defined __linux__
	.section	.note.GNU-stack,"",@progbits
#endif
