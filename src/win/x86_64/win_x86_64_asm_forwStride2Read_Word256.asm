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
win_x86_64_asm_forwStride2Read_Word256 proc

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
    cmp rax,rdx     ; have we completed the target total number of words to access?
    jae done        ; if the number of words accessed >= the target number, then we are done

myloop:
    ; Unroll 64 loads of 256-bit words (32 bytes is 20h) in strides of 2 words before checking loop condition.
    vmovdqa ymm0, ymmword ptr [rcx+0000h]               
    vmovdqa ymm0, ymmword ptr [rcx+0040h]
    vmovdqa ymm0, ymmword ptr [rcx+0080h]
    vmovdqa ymm0, ymmword ptr [rcx+00C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0100h]               
    vmovdqa ymm0, ymmword ptr [rcx+0140h]
    vmovdqa ymm0, ymmword ptr [rcx+0180h]
    vmovdqa ymm0, ymmword ptr [rcx+01C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0200h]               
    vmovdqa ymm0, ymmword ptr [rcx+0240h]
    vmovdqa ymm0, ymmword ptr [rcx+0280h]
    vmovdqa ymm0, ymmword ptr [rcx+02C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0300h]               
    vmovdqa ymm0, ymmword ptr [rcx+0340h]
    vmovdqa ymm0, ymmword ptr [rcx+0380h]
    vmovdqa ymm0, ymmword ptr [rcx+03C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0400h]               
    vmovdqa ymm0, ymmword ptr [rcx+0440h]
    vmovdqa ymm0, ymmword ptr [rcx+0480h]
    vmovdqa ymm0, ymmword ptr [rcx+04C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0500h]               
    vmovdqa ymm0, ymmword ptr [rcx+0540h]
    vmovdqa ymm0, ymmword ptr [rcx+0580h]
    vmovdqa ymm0, ymmword ptr [rcx+05C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0600h]               
    vmovdqa ymm0, ymmword ptr [rcx+0640h]
    vmovdqa ymm0, ymmword ptr [rcx+0680h]
    vmovdqa ymm0, ymmword ptr [rcx+06C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0700h]               
    vmovdqa ymm0, ymmword ptr [rcx+0740h]
    vmovdqa ymm0, ymmword ptr [rcx+0780h]
    vmovdqa ymm0, ymmword ptr [rcx+07C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0800h]               
    vmovdqa ymm0, ymmword ptr [rcx+0840h]
    vmovdqa ymm0, ymmword ptr [rcx+0880h]
    vmovdqa ymm0, ymmword ptr [rcx+08C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0900h]               
    vmovdqa ymm0, ymmword ptr [rcx+0940h]
    vmovdqa ymm0, ymmword ptr [rcx+0980h]
    vmovdqa ymm0, ymmword ptr [rcx+09C0h]
    vmovdqa ymm0, ymmword ptr [rcx+0A00h]               
    vmovdqa ymm0, ymmword ptr [rcx+0A40h]
    vmovdqa ymm0, ymmword ptr [rcx+0A80h]
    vmovdqa ymm0, ymmword ptr [rcx+0AC0h]
    vmovdqa ymm0, ymmword ptr [rcx+0B00h]               
    vmovdqa ymm0, ymmword ptr [rcx+0B40h]
    vmovdqa ymm0, ymmword ptr [rcx+0B80h]
    vmovdqa ymm0, ymmword ptr [rcx+0BC0h]
    vmovdqa ymm0, ymmword ptr [rcx+0C00h]               
    vmovdqa ymm0, ymmword ptr [rcx+0C40h]
    vmovdqa ymm0, ymmword ptr [rcx+0C80h]
    vmovdqa ymm0, ymmword ptr [rcx+0CC0h]
    vmovdqa ymm0, ymmword ptr [rcx+0D00h]               
    vmovdqa ymm0, ymmword ptr [rcx+0D40h]
    vmovdqa ymm0, ymmword ptr [rcx+0D80h]
    vmovdqa ymm0, ymmword ptr [rcx+0DC0h]
    vmovdqa ymm0, ymmword ptr [rcx+0E00h]               
    vmovdqa ymm0, ymmword ptr [rcx+0E40h]
    vmovdqa ymm0, ymmword ptr [rcx+0E80h]
    vmovdqa ymm0, ymmword ptr [rcx+0EC0h]
    vmovdqa ymm0, ymmword ptr [rcx+0F00h]               
    vmovdqa ymm0, ymmword ptr [rcx+0F40h]
    vmovdqa ymm0, ymmword ptr [rcx+0F80h]
    vmovdqa ymm0, ymmword ptr [rcx+0FC0h]

    add rax,64     ; Just did 64 accesses

    cmp rax,rdx     ; have we completed the target number of accesses in total yet?
    jb myloop       ; make another unrolled pass on the memory
    
done:
    xor eax,eax     ; return 0
    ret

win_x86_64_asm_forwStride2Read_Word256 endp
end
