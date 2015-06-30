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
win_x86_64_asm_forwStride2Write_Word256 proc

; Arguments:
; rcx is address of the first 256-bit word in the array
; rdx is address of the last 256-bit word in the array

; rax holds number of words accessed
; rcx holds the first 256-bit word address
; rdx holds the target total number of words to access
; xmm0 holds result from reading the memory 256-bit wide

    xor rax,rax     ; initialize number of words accessed to 0
    sub rdx,rcx     ; Get total number of 256-bit words between starting and ending addresses
    shr rdx,4       

	; initialize ymm0 to the value to write to memory. We will use all 1s, e.g. 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    ; I am using the first 256-bits of the memory array to initialize ymm0. Is there a better way to do this?
    mov qword ptr [rcx],0FFFFFFFFh
    mov qword ptr [rcx+8h],0FFFFFFFFh
    mov qword ptr [rcx+10h],0FFFFFFFFh
    mov qword ptr [rcx+18h],0FFFFFFFFh
    mov qword ptr [rcx+20h],0FFFFFFFFh
    mov qword ptr [rcx+28h],0FFFFFFFFh
    mov qword ptr [rcx+30h],0FFFFFFFFh
    mov qword ptr [rcx+38h],0FFFFFFFFh
    vmovdqu ymm0, ymmword ptr[rcx]

    cmp rax,rdx     ; have we completed the target total number of words to access?
    jae done        ; if the number of words accessed >= the target number, then we are done

myloop:
    ; Unroll 64 loads of 256-bit words (32 bytes is 20h) in strides of 2 words before checking loop condition.
    vmovdqa ymmword ptr [rcx+0000h], ymm0
    vmovdqa ymmword ptr [rcx+0040h], ymm0
    vmovdqa ymmword ptr [rcx+0080h], ymm0
    vmovdqa ymmword ptr [rcx+00C0h], ymm0
    vmovdqa ymmword ptr [rcx+0100h], ymm0               
    vmovdqa ymmword ptr [rcx+0140h], ymm0
    vmovdqa ymmword ptr [rcx+0180h], ymm0
    vmovdqa ymmword ptr [rcx+01C0h], ymm0
    vmovdqa ymmword ptr [rcx+0200h], ymm0               
    vmovdqa ymmword ptr [rcx+0240h], ymm0
    vmovdqa ymmword ptr [rcx+0280h], ymm0
    vmovdqa ymmword ptr [rcx+02C0h], ymm0
    vmovdqa ymmword ptr [rcx+0300h], ymm0               
    vmovdqa ymmword ptr [rcx+0340h], ymm0
    vmovdqa ymmword ptr [rcx+0380h], ymm0
    vmovdqa ymmword ptr [rcx+03C0h], ymm0
    vmovdqa ymmword ptr [rcx+0400h], ymm0               
    vmovdqa ymmword ptr [rcx+0440h], ymm0
    vmovdqa ymmword ptr [rcx+0480h], ymm0
    vmovdqa ymmword ptr [rcx+04C0h], ymm0
    vmovdqa ymmword ptr [rcx+0500h], ymm0               
    vmovdqa ymmword ptr [rcx+0540h], ymm0
    vmovdqa ymmword ptr [rcx+0580h], ymm0
    vmovdqa ymmword ptr [rcx+05C0h], ymm0
    vmovdqa ymmword ptr [rcx+0600h], ymm0               
    vmovdqa ymmword ptr [rcx+0640h], ymm0
    vmovdqa ymmword ptr [rcx+0680h], ymm0
    vmovdqa ymmword ptr [rcx+06C0h], ymm0
    vmovdqa ymmword ptr [rcx+0700h], ymm0               
    vmovdqa ymmword ptr [rcx+0740h], ymm0
    vmovdqa ymmword ptr [rcx+0780h], ymm0
    vmovdqa ymmword ptr [rcx+07C0h], ymm0
    vmovdqa ymmword ptr [rcx+0800h], ymm0               
    vmovdqa ymmword ptr [rcx+0840h], ymm0
    vmovdqa ymmword ptr [rcx+0880h], ymm0
    vmovdqa ymmword ptr [rcx+08C0h], ymm0
    vmovdqa ymmword ptr [rcx+0900h], ymm0               
    vmovdqa ymmword ptr [rcx+0940h], ymm0
    vmovdqa ymmword ptr [rcx+0980h], ymm0
    vmovdqa ymmword ptr [rcx+09C0h], ymm0
    vmovdqa ymmword ptr [rcx+0A00h], ymm0               
    vmovdqa ymmword ptr [rcx+0A40h], ymm0
    vmovdqa ymmword ptr [rcx+0A80h], ymm0
    vmovdqa ymmword ptr [rcx+0AC0h], ymm0
    vmovdqa ymmword ptr [rcx+0B00h], ymm0               
    vmovdqa ymmword ptr [rcx+0B40h], ymm0
    vmovdqa ymmword ptr [rcx+0B80h], ymm0
    vmovdqa ymmword ptr [rcx+0BC0h], ymm0
    vmovdqa ymmword ptr [rcx+0C00h], ymm0               
    vmovdqa ymmword ptr [rcx+0C40h], ymm0
    vmovdqa ymmword ptr [rcx+0C80h], ymm0
    vmovdqa ymmword ptr [rcx+0CC0h], ymm0
    vmovdqa ymmword ptr [rcx+0D00h], ymm0               
    vmovdqa ymmword ptr [rcx+0D40h], ymm0
    vmovdqa ymmword ptr [rcx+0D80h], ymm0
    vmovdqa ymmword ptr [rcx+0DC0h], ymm0
    vmovdqa ymmword ptr [rcx+0E00h], ymm0               
    vmovdqa ymmword ptr [rcx+0E40h], ymm0
    vmovdqa ymmword ptr [rcx+0E80h], ymm0
    vmovdqa ymmword ptr [rcx+0EC0h], ymm0
    vmovdqa ymmword ptr [rcx+0F00h], ymm0               
    vmovdqa ymmword ptr [rcx+0F40h], ymm0
    vmovdqa ymmword ptr [rcx+0F80h], ymm0
    vmovdqa ymmword ptr [rcx+0FC0h], ymm0

    add rax,64     ; Just did 64 accesses

    cmp rax,rdx     ; have we completed the target number of accesses in total yet?
    jb myloop       ; make another unrolled pass on the memory
    
done:
    xor eax,eax     ; return 0
    ret

win_x86_64_asm_forwStride2Write_Word256 endp
end
