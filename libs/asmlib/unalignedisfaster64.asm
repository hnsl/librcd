;*************************  unalignedisfaster64.asm  ******************************
; Author:           Agner Fog
; Date created:     2011-07-09
; Last modified:    2011-07-09
; Source URL:       www.agner.org/optimize
; Project:          asmlib.zip
; Language:         assembly, NASM/YASM syntax, 64 bit
;
; C++ prototype:
; extern "C" int UnalignedIsFaster(void);
;
; Description:
; This function returns finds out if unaligned 16-bytes memory read is
; faster than aligned read followed by an alignment shift (PALIGNR) on the
; current CPU.
;
; Return value:
; 0:   Unaligned read is probably slower than alignment shift
; 1:   Unknown
; 2:   Unaligned read is probably faster than alignment shift
;
; Copyright (c) 2011 GNU General Public License www.gnu.org/licenses
;******************************************************************************
;
; C++ prototype:
; extern "C" int UnalignedIsFaster(void);

global UnalignedIsFaster: function
extern CpuType


SECTION .text

UnalignedIsFaster:

%ifdef  UNIX
        push    0                      ; vendor
        mov     rdi, rsp
        push    0                      ; family 
        mov     rsi, rsp
        push    0                      ; model
        mov     rdx, rsp 
%else   ; WINDOWS
        push    0                      ; vendor
        mov     rcx, rsp
        push    0                      ; family 
        mov     rdx, rsp
        push    0                      ; model
        mov     r8,  rsp 
%endif
        
        call    CpuType                ; get vendor, family, model
        pop     rdx                    ; model
        pop     rcx                    ; family
        pop     r8                     ; vendor
        xor     eax, eax               ; return value
        dec     r8d
        jz      Intel
        dec     r8d
        jz      AMD
        dec     r8d
        jz      VIA
        ; unknown vendor
        inc     eax
        jmp     Uend
        
Intel:  ; Unaligned read is faster on Intel Nehalem and later, but not Atom
        ; Nehalem  = family 6, model 1AH
        ; Atom     = family 6, model 1CH
        ; Netburst = family 0FH
        ; Future models are likely to be family 6, mayby > 6, model > 1C
        cmp     ecx, 6
        jb      Uend                   ; old Pentium 1, etc
        cmp     ecx, 0FH
        je      Uend                   ; old Netburst architecture
        cmp     edx, 1AH
        jb      Uend                   ; earlier than Nehalem
        cmp     edx, 1CH
        je      Uend                   ; Intel Atom
        or      eax, 2                 ; Intel Nehalem and later, except Atom
        jmp     Uend
        
AMD:    ; Uanligned read is fast on AMD K10 and later
        ; The PALIGNR instruction is slow on AMD Bobcat
        ; K10    = family 10H
        ; Bobcat = family 14H
        cmp     ecx, 10H
        jb      Uend                   ; AMD K8 or earlier
        or      eax, 2                 ; AMD K10 or later
        jmp     Uend
        
VIA:    ; Unaligned read is not faster than PALIGNR on VIA Nano 2000 and 3000                
        cmp     ecx, 0FH
        jna     Uend                   ; VIA Nano
        inc     eax                    ; Future versions: unknown
       ;jmp     Uend
        
Uend:   ret

;UnalignedIsFaster ENDP
