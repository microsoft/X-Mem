; The MIT License (MIT)
;
; Copyright (c) 2014 Microsoft
; 
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;
; Author: Mark Gottscho <mgottscho@ucla.edu>

.code
win_x86_64_asm_revSequentialWrite_Word256 proc

; Arguments:
; rcx is address of the last 256-bit word in the array
; rdx is address of the first 256-bit word in the array

; rax holds current 256-bit word address
; ymm0 holds result from reading the memory 256-bit wide

    mov rax,rcx         ; initialize current word address to start of the array

    ; initialize ymm0 to the value to write to memory. We will use all 1s, e.g. 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    ; I am using the first 256-bits of the memory array to initialize ymm0. Is there a better way to do this?
    mov qword ptr [rdx],0FFFFFFFFh
    mov qword ptr [rdx+8h],0FFFFFFFFh
    mov qword ptr [rdx+10h],0FFFFFFFFh
    mov qword ptr [rdx+18h],0FFFFFFFFh
    mov qword ptr [rdx+20h],0FFFFFFFFh
    mov qword ptr [rdx+28h],0FFFFFFFFh
    mov qword ptr [rdx+30h],0FFFFFFFFh
    mov qword ptr [rdx+38h],0FFFFFFFFh
    vmovdqu ymm0, ymmword ptr[rdx]
    
    cmp rax,rdx     ; have we reached the first word yet?
    jbe done        ; if current word address is <= first word address, jump to done

myloop:
    ; Unroll 128 loads of 256-bit words (32 bytes is 20h) before checking loop condition.

    vmovdqa ymmword ptr [rax-0000h], ymm0       
    vmovdqa ymmword ptr [rax-0020h], ymm0
    vmovdqa ymmword ptr [rax-0040h], ymm0
    vmovdqa ymmword ptr [rax-0060h], ymm0
    vmovdqa ymmword ptr [rax-0080h], ymm0
    vmovdqa ymmword ptr [rax-00A0h], ymm0
    vmovdqa ymmword ptr [rax-00C0h], ymm0
    vmovdqa ymmword ptr [rax-00E0h], ymm0
    vmovdqa ymmword ptr [rax-0100h], ymm0
    vmovdqa ymmword ptr [rax-0120h], ymm0
    vmovdqa ymmword ptr [rax-0140h], ymm0
    vmovdqa ymmword ptr [rax-0160h], ymm0
    vmovdqa ymmword ptr [rax-0180h], ymm0
    vmovdqa ymmword ptr [rax-01A0h], ymm0
    vmovdqa ymmword ptr [rax-01C0h], ymm0
    vmovdqa ymmword ptr [rax-01E0h], ymm0
    vmovdqa ymmword ptr [rax-0200h], ymm0
    vmovdqa ymmword ptr [rax-0220h], ymm0
    vmovdqa ymmword ptr [rax-0240h], ymm0
    vmovdqa ymmword ptr [rax-0260h], ymm0
    vmovdqa ymmword ptr [rax-0280h], ymm0
    vmovdqa ymmword ptr [rax-02A0h], ymm0
    vmovdqa ymmword ptr [rax-02C0h], ymm0
    vmovdqa ymmword ptr [rax-02E0h], ymm0
    vmovdqa ymmword ptr [rax-0300h], ymm0
    vmovdqa ymmword ptr [rax-0320h], ymm0
    vmovdqa ymmword ptr [rax-0340h], ymm0
    vmovdqa ymmword ptr [rax-0360h], ymm0
    vmovdqa ymmword ptr [rax-0380h], ymm0
    vmovdqa ymmword ptr [rax-03A0h], ymm0
    vmovdqa ymmword ptr [rax-03C0h], ymm0
    vmovdqa ymmword ptr [rax-03E0h], ymm0
    vmovdqa ymmword ptr [rax-0400h], ymm0
    vmovdqa ymmword ptr [rax-0420h], ymm0
    vmovdqa ymmword ptr [rax-0440h], ymm0
    vmovdqa ymmword ptr [rax-0460h], ymm0
    vmovdqa ymmword ptr [rax-0480h], ymm0
    vmovdqa ymmword ptr [rax-04A0h], ymm0
    vmovdqa ymmword ptr [rax-04C0h], ymm0
    vmovdqa ymmword ptr [rax-04E0h], ymm0
    vmovdqa ymmword ptr [rax-0500h], ymm0
    vmovdqa ymmword ptr [rax-0520h], ymm0
    vmovdqa ymmword ptr [rax-0540h], ymm0
    vmovdqa ymmword ptr [rax-0560h], ymm0
    vmovdqa ymmword ptr [rax-0580h], ymm0
    vmovdqa ymmword ptr [rax-05A0h], ymm0
    vmovdqa ymmword ptr [rax-05C0h], ymm0
    vmovdqa ymmword ptr [rax-05E0h], ymm0
    vmovdqa ymmword ptr [rax-0600h], ymm0
    vmovdqa ymmword ptr [rax-0620h], ymm0
    vmovdqa ymmword ptr [rax-0640h], ymm0
    vmovdqa ymmword ptr [rax-0660h], ymm0
    vmovdqa ymmword ptr [rax-0680h], ymm0
    vmovdqa ymmword ptr [rax-06A0h], ymm0
    vmovdqa ymmword ptr [rax-06C0h], ymm0
    vmovdqa ymmword ptr [rax-06E0h], ymm0
    vmovdqa ymmword ptr [rax-0700h], ymm0
    vmovdqa ymmword ptr [rax-0720h], ymm0
    vmovdqa ymmword ptr [rax-0740h], ymm0
    vmovdqa ymmword ptr [rax-0760h], ymm0
    vmovdqa ymmword ptr [rax-0780h], ymm0
    vmovdqa ymmword ptr [rax-07A0h], ymm0
    vmovdqa ymmword ptr [rax-07C0h], ymm0
    vmovdqa ymmword ptr [rax-07E0h], ymm0
    vmovdqa ymmword ptr [rax-0800h], ymm0
    vmovdqa ymmword ptr [rax-0820h], ymm0
    vmovdqa ymmword ptr [rax-0840h], ymm0
    vmovdqa ymmword ptr [rax-0860h], ymm0
    vmovdqa ymmword ptr [rax-0880h], ymm0
    vmovdqa ymmword ptr [rax-08A0h], ymm0
    vmovdqa ymmword ptr [rax-08C0h], ymm0
    vmovdqa ymmword ptr [rax-08E0h], ymm0
    vmovdqa ymmword ptr [rax-0900h], ymm0
    vmovdqa ymmword ptr [rax-0920h], ymm0
    vmovdqa ymmword ptr [rax-0940h], ymm0
    vmovdqa ymmword ptr [rax-0960h], ymm0
    vmovdqa ymmword ptr [rax-0980h], ymm0
    vmovdqa ymmword ptr [rax-09A0h], ymm0
    vmovdqa ymmword ptr [rax-09C0h], ymm0
    vmovdqa ymmword ptr [rax-09E0h], ymm0
    vmovdqa ymmword ptr [rax-0A00h], ymm0
    vmovdqa ymmword ptr [rax-0A20h], ymm0
    vmovdqa ymmword ptr [rax-0A40h], ymm0
    vmovdqa ymmword ptr [rax-0A60h], ymm0
    vmovdqa ymmword ptr [rax-0A80h], ymm0
    vmovdqa ymmword ptr [rax-0AA0h], ymm0
    vmovdqa ymmword ptr [rax-0AC0h], ymm0
    vmovdqa ymmword ptr [rax-0AE0h], ymm0
    vmovdqa ymmword ptr [rax-0B00h], ymm0
    vmovdqa ymmword ptr [rax-0B20h], ymm0
    vmovdqa ymmword ptr [rax-0B40h], ymm0
    vmovdqa ymmword ptr [rax-0B60h], ymm0
    vmovdqa ymmword ptr [rax-0B80h], ymm0
    vmovdqa ymmword ptr [rax-0BA0h], ymm0
    vmovdqa ymmword ptr [rax-0BC0h], ymm0
    vmovdqa ymmword ptr [rax-0BE0h], ymm0
    vmovdqa ymmword ptr [rax-0C00h], ymm0
    vmovdqa ymmword ptr [rax-0C20h], ymm0
    vmovdqa ymmword ptr [rax-0C40h], ymm0
    vmovdqa ymmword ptr [rax-0C60h], ymm0
    vmovdqa ymmword ptr [rax-0C80h], ymm0
    vmovdqa ymmword ptr [rax-0CA0h], ymm0
    vmovdqa ymmword ptr [rax-0CC0h], ymm0
    vmovdqa ymmword ptr [rax-0CE0h], ymm0
    vmovdqa ymmword ptr [rax-0D00h], ymm0
    vmovdqa ymmword ptr [rax-0D20h], ymm0
    vmovdqa ymmword ptr [rax-0D40h], ymm0
    vmovdqa ymmword ptr [rax-0D60h], ymm0
    vmovdqa ymmword ptr [rax-0D80h], ymm0
    vmovdqa ymmword ptr [rax-0DA0h], ymm0
    vmovdqa ymmword ptr [rax-0DC0h], ymm0
    vmovdqa ymmword ptr [rax-0DE0h], ymm0
    vmovdqa ymmword ptr [rax-0E00h], ymm0
    vmovdqa ymmword ptr [rax-0E20h], ymm0
    vmovdqa ymmword ptr [rax-0E40h], ymm0
    vmovdqa ymmword ptr [rax-0E60h], ymm0
    vmovdqa ymmword ptr [rax-0E80h], ymm0
    vmovdqa ymmword ptr [rax-0EA0h], ymm0
    vmovdqa ymmword ptr [rax-0EC0h], ymm0
    vmovdqa ymmword ptr [rax-0EE0h], ymm0
    vmovdqa ymmword ptr [rax-0F00h], ymm0
    vmovdqa ymmword ptr [rax-0F20h], ymm0
    vmovdqa ymmword ptr [rax-0F40h], ymm0
    vmovdqa ymmword ptr [rax-0F60h], ymm0
    vmovdqa ymmword ptr [rax-0F80h], ymm0
    vmovdqa ymmword ptr [rax-0FA0h], ymm0
    vmovdqa ymmword ptr [rax-0FC0h], ymm0
    vmovdqa ymmword ptr [rax-0FE0h], ymm0

    sub rax,1000h                           ; End of one unrolled loop iteration. Decrement pointer by 128 words of size 32 bytes, which is 4096 bytes.
    
    cmp rax,rdx     ; have we reached the first word yet?
    jbe done        ; if current word address is <= first word address, jump to done
    jmp myloop      ; continue loop

done:
    xor eax,eax     ; return 0
    ret

win_x86_64_asm_revSequentialWrite_Word256 endp
end
