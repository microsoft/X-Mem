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
win_x86_64_asm_revStride4Write_Word128 proc

; Arguments:
; rcx is address of the last 128-bit word in the array
; rdx is address of the first 128-bit word in the array

; rax holds number of words accessed
; rcx holds the last 128-bit word address
; rdx holds the target total number of words to access
; xmm0 holds result from reading the memory 128-bit wide

    mov rax,rcx     ; Temporarily save last word address
    sub rcx,rdx     ; Get total number of 128-bit words between starting and ending addresses
    shr rcx,4       
    mov rdx,rcx     ; Set target number of words
    mov rcx,rax     ; Restore last word address
    
    ; initialize ymm0 to the value to write to memory. We will use all 1s, e.g. 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    ; I am using the last 128-bits of the memory array to initialize xmm0. Is there a better way to do this?
    mov qword ptr [rcx],0FFFFFFFFh
    mov qword ptr [rcx+8h],0FFFFFFFFh
    mov qword ptr [rcx+10h],0FFFFFFFFh
    mov qword ptr [rcx+18h],0FFFFFFFFh
    vmovdqu xmm0, xmmword ptr[rcx]

    xor rax,rax     ; initialize number of words accessed to 0
    cmp rax,rdx     ; have we completed the target total number of words to access?
    jae done        ; if the number of words accessed >= the target number, then we are done

myloop:
    ; Unroll 64 loads of 128-bit words (16 bytes is 10h) in strides of 4 words before checking loop condition.
    vmovdqa xmmword ptr [rcx-0000h], xmm0
    vmovdqa xmmword ptr [rcx-0040h], xmm0
    vmovdqa xmmword ptr [rcx-0080h], xmm0
    vmovdqa xmmword ptr [rcx-00C0h], xmm0
    vmovdqa xmmword ptr [rcx-0100h], xmm0               
    vmovdqa xmmword ptr [rcx-0140h], xmm0
    vmovdqa xmmword ptr [rcx-0180h], xmm0
    vmovdqa xmmword ptr [rcx-01C0h], xmm0
    vmovdqa xmmword ptr [rcx-0200h], xmm0               
    vmovdqa xmmword ptr [rcx-0240h], xmm0
    vmovdqa xmmword ptr [rcx-0280h], xmm0
    vmovdqa xmmword ptr [rcx-02C0h], xmm0
    vmovdqa xmmword ptr [rcx-0300h], xmm0               
    vmovdqa xmmword ptr [rcx-0340h], xmm0
    vmovdqa xmmword ptr [rcx-0380h], xmm0
    vmovdqa xmmword ptr [rcx-03C0h], xmm0
    vmovdqa xmmword ptr [rcx-0400h], xmm0               
    vmovdqa xmmword ptr [rcx-0440h], xmm0
    vmovdqa xmmword ptr [rcx-0480h], xmm0
    vmovdqa xmmword ptr [rcx-04C0h], xmm0
    vmovdqa xmmword ptr [rcx-0500h], xmm0               
    vmovdqa xmmword ptr [rcx-0540h], xmm0
    vmovdqa xmmword ptr [rcx-0580h], xmm0
    vmovdqa xmmword ptr [rcx-05C0h], xmm0
    vmovdqa xmmword ptr [rcx-0600h], xmm0               
    vmovdqa xmmword ptr [rcx-0640h], xmm0
    vmovdqa xmmword ptr [rcx-0680h], xmm0
    vmovdqa xmmword ptr [rcx-06C0h], xmm0
    vmovdqa xmmword ptr [rcx-0700h], xmm0               
    vmovdqa xmmword ptr [rcx-0740h], xmm0
    vmovdqa xmmword ptr [rcx-0780h], xmm0
    vmovdqa xmmword ptr [rcx-07C0h], xmm0
    vmovdqa xmmword ptr [rcx-0800h], xmm0               
    vmovdqa xmmword ptr [rcx-0840h], xmm0
    vmovdqa xmmword ptr [rcx-0880h], xmm0
    vmovdqa xmmword ptr [rcx-08C0h], xmm0
    vmovdqa xmmword ptr [rcx-0900h], xmm0               
    vmovdqa xmmword ptr [rcx-0940h], xmm0
    vmovdqa xmmword ptr [rcx-0980h], xmm0
    vmovdqa xmmword ptr [rcx-09C0h], xmm0
    vmovdqa xmmword ptr [rcx-0A00h], xmm0               
    vmovdqa xmmword ptr [rcx-0A40h], xmm0
    vmovdqa xmmword ptr [rcx-0A80h], xmm0
    vmovdqa xmmword ptr [rcx-0AC0h], xmm0
    vmovdqa xmmword ptr [rcx-0B00h], xmm0               
    vmovdqa xmmword ptr [rcx-0B40h], xmm0
    vmovdqa xmmword ptr [rcx-0B80h], xmm0
    vmovdqa xmmword ptr [rcx-0BC0h], xmm0
    vmovdqa xmmword ptr [rcx-0C00h], xmm0               
    vmovdqa xmmword ptr [rcx-0C40h], xmm0
    vmovdqa xmmword ptr [rcx-0C80h], xmm0
    vmovdqa xmmword ptr [rcx-0CC0h], xmm0
    vmovdqa xmmword ptr [rcx-0D00h], xmm0               
    vmovdqa xmmword ptr [rcx-0D40h], xmm0
    vmovdqa xmmword ptr [rcx-0D80h], xmm0
    vmovdqa xmmword ptr [rcx-0DC0h], xmm0
    vmovdqa xmmword ptr [rcx-0E00h], xmm0               
    vmovdqa xmmword ptr [rcx-0E40h], xmm0
    vmovdqa xmmword ptr [rcx-0E80h], xmm0
    vmovdqa xmmword ptr [rcx-0EC0h], xmm0
    vmovdqa xmmword ptr [rcx-0F00h], xmm0               
    vmovdqa xmmword ptr [rcx-0F40h], xmm0
    vmovdqa xmmword ptr [rcx-0F80h], xmm0
    vmovdqa xmmword ptr [rcx-0FC0h], xmm0

    add rax,64     ; Just did 64 accesses

    cmp rax,rdx     ; have we completed the target number of accesses in total yet?
    jb myloop       ; make another unrolled pass on the memory
    
done:
    xor eax,eax     ; return 0
    ret

win_x86_64_asm_revStride4Write_Word128 endp
end
