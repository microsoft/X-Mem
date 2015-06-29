; The MIT License (MIT)
;
; Copyright (c) 2015 Microsoft
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
win_x86_64_asm_revStride2Read_Word128 proc

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
    xor rax,rax     ; initialize number of words accessed to 0
    cmp rax,rdx     ; have we completed the target total number of words to access?
    jae done        ; if the number of words accessed >= the target number, then we are done

myloop:
    ; Unroll 128 loads of 128-bit words (16 bytes is 10h) in strides of 2 words before checking loop condition.
    vmovdqa xmm0, xmmword ptr [rcx-0000h]               
    vmovdqa xmm0, xmmword ptr [rcx-0020h]
    vmovdqa xmm0, xmmword ptr [rcx-0040h]
    vmovdqa xmm0, xmmword ptr [rcx-0060h]
    vmovdqa xmm0, xmmword ptr [rcx-0080h]
    vmovdqa xmm0, xmmword ptr [rcx-00A0h]
    vmovdqa xmm0, xmmword ptr [rcx-00C0h]
    vmovdqa xmm0, xmmword ptr [rcx-00E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0100h]               
    vmovdqa xmm0, xmmword ptr [rcx-0120h]
    vmovdqa xmm0, xmmword ptr [rcx-0140h]
    vmovdqa xmm0, xmmword ptr [rcx-0160h]
    vmovdqa xmm0, xmmword ptr [rcx-0180h]
    vmovdqa xmm0, xmmword ptr [rcx-01A0h]
    vmovdqa xmm0, xmmword ptr [rcx-01C0h]
    vmovdqa xmm0, xmmword ptr [rcx-01E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0200h]               
    vmovdqa xmm0, xmmword ptr [rcx-0220h]
    vmovdqa xmm0, xmmword ptr [rcx-0240h]
    vmovdqa xmm0, xmmword ptr [rcx-0260h]
    vmovdqa xmm0, xmmword ptr [rcx-0280h]
    vmovdqa xmm0, xmmword ptr [rcx-02A0h]
    vmovdqa xmm0, xmmword ptr [rcx-02C0h]
    vmovdqa xmm0, xmmword ptr [rcx-02E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0300h]               
    vmovdqa xmm0, xmmword ptr [rcx-0320h]
    vmovdqa xmm0, xmmword ptr [rcx-0340h]
    vmovdqa xmm0, xmmword ptr [rcx-0360h]
    vmovdqa xmm0, xmmword ptr [rcx-0380h]
    vmovdqa xmm0, xmmword ptr [rcx-03A0h]
    vmovdqa xmm0, xmmword ptr [rcx-03C0h]
    vmovdqa xmm0, xmmword ptr [rcx-03E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0400h]               
    vmovdqa xmm0, xmmword ptr [rcx-0420h]
    vmovdqa xmm0, xmmword ptr [rcx-0440h]
    vmovdqa xmm0, xmmword ptr [rcx-0460h]
    vmovdqa xmm0, xmmword ptr [rcx-0480h]
    vmovdqa xmm0, xmmword ptr [rcx-04A0h]
    vmovdqa xmm0, xmmword ptr [rcx-04C0h]
    vmovdqa xmm0, xmmword ptr [rcx-04E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0500h]               
    vmovdqa xmm0, xmmword ptr [rcx-0520h]
    vmovdqa xmm0, xmmword ptr [rcx-0540h]
    vmovdqa xmm0, xmmword ptr [rcx-0560h]
    vmovdqa xmm0, xmmword ptr [rcx-0580h]
    vmovdqa xmm0, xmmword ptr [rcx-05A0h]
    vmovdqa xmm0, xmmword ptr [rcx-05C0h]
    vmovdqa xmm0, xmmword ptr [rcx-05E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0600h]               
    vmovdqa xmm0, xmmword ptr [rcx-0620h]
    vmovdqa xmm0, xmmword ptr [rcx-0640h]
    vmovdqa xmm0, xmmword ptr [rcx-0660h]
    vmovdqa xmm0, xmmword ptr [rcx-0680h]
    vmovdqa xmm0, xmmword ptr [rcx-06A0h]
    vmovdqa xmm0, xmmword ptr [rcx-06C0h]
    vmovdqa xmm0, xmmword ptr [rcx-06E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0700h]               
    vmovdqa xmm0, xmmword ptr [rcx-0720h]
    vmovdqa xmm0, xmmword ptr [rcx-0740h]
    vmovdqa xmm0, xmmword ptr [rcx-0760h]
    vmovdqa xmm0, xmmword ptr [rcx-0780h]
    vmovdqa xmm0, xmmword ptr [rcx-07A0h]
    vmovdqa xmm0, xmmword ptr [rcx-07C0h]
    vmovdqa xmm0, xmmword ptr [rcx-07E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0800h]               
    vmovdqa xmm0, xmmword ptr [rcx-0820h]
    vmovdqa xmm0, xmmword ptr [rcx-0840h]
    vmovdqa xmm0, xmmword ptr [rcx-0860h]
    vmovdqa xmm0, xmmword ptr [rcx-0880h]
    vmovdqa xmm0, xmmword ptr [rcx-08A0h]
    vmovdqa xmm0, xmmword ptr [rcx-08C0h]
    vmovdqa xmm0, xmmword ptr [rcx-08E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0900h]               
    vmovdqa xmm0, xmmword ptr [rcx-0920h]
    vmovdqa xmm0, xmmword ptr [rcx-0940h]
    vmovdqa xmm0, xmmword ptr [rcx-0960h]
    vmovdqa xmm0, xmmword ptr [rcx-0980h]
    vmovdqa xmm0, xmmword ptr [rcx-09A0h]
    vmovdqa xmm0, xmmword ptr [rcx-09C0h]
    vmovdqa xmm0, xmmword ptr [rcx-09E0h]
    vmovdqa xmm0, xmmword ptr [rcx-0A00h]               
    vmovdqa xmm0, xmmword ptr [rcx-0A20h]
    vmovdqa xmm0, xmmword ptr [rcx-0A40h]
    vmovdqa xmm0, xmmword ptr [rcx-0A60h]
    vmovdqa xmm0, xmmword ptr [rcx-0A80h]
    vmovdqa xmm0, xmmword ptr [rcx-0AA0h]
    vmovdqa xmm0, xmmword ptr [rcx-0AC0h]
    vmovdqa xmm0, xmmword ptr [rcx-0AE0h]
    vmovdqa xmm0, xmmword ptr [rcx-0B00h]               
    vmovdqa xmm0, xmmword ptr [rcx-0B20h]
    vmovdqa xmm0, xmmword ptr [rcx-0B40h]
    vmovdqa xmm0, xmmword ptr [rcx-0B60h]
    vmovdqa xmm0, xmmword ptr [rcx-0B80h]
    vmovdqa xmm0, xmmword ptr [rcx-0BA0h]
    vmovdqa xmm0, xmmword ptr [rcx-0BC0h]
    vmovdqa xmm0, xmmword ptr [rcx-0BE0h]
    vmovdqa xmm0, xmmword ptr [rcx-0C00h]               
    vmovdqa xmm0, xmmword ptr [rcx-0C20h]
    vmovdqa xmm0, xmmword ptr [rcx-0C40h]
    vmovdqa xmm0, xmmword ptr [rcx-0C60h]
    vmovdqa xmm0, xmmword ptr [rcx-0C80h]
    vmovdqa xmm0, xmmword ptr [rcx-0CA0h]
    vmovdqa xmm0, xmmword ptr [rcx-0CC0h]
    vmovdqa xmm0, xmmword ptr [rcx-0CE0h]
    vmovdqa xmm0, xmmword ptr [rcx-0D00h]               
    vmovdqa xmm0, xmmword ptr [rcx-0D20h]
    vmovdqa xmm0, xmmword ptr [rcx-0D40h]
    vmovdqa xmm0, xmmword ptr [rcx-0D60h]
    vmovdqa xmm0, xmmword ptr [rcx-0D80h]
    vmovdqa xmm0, xmmword ptr [rcx-0DA0h]
    vmovdqa xmm0, xmmword ptr [rcx-0DC0h]
    vmovdqa xmm0, xmmword ptr [rcx-0DE0h]
    vmovdqa xmm0, xmmword ptr [rcx-0E00h]               
    vmovdqa xmm0, xmmword ptr [rcx-0E20h]
    vmovdqa xmm0, xmmword ptr [rcx-0E40h]
    vmovdqa xmm0, xmmword ptr [rcx-0E60h]
    vmovdqa xmm0, xmmword ptr [rcx-0E80h]
    vmovdqa xmm0, xmmword ptr [rcx-0EA0h]
    vmovdqa xmm0, xmmword ptr [rcx-0EC0h]
    vmovdqa xmm0, xmmword ptr [rcx-0EE0h]
    vmovdqa xmm0, xmmword ptr [rcx-0F00h]               
    vmovdqa xmm0, xmmword ptr [rcx-0F20h]
    vmovdqa xmm0, xmmword ptr [rcx-0F40h]
    vmovdqa xmm0, xmmword ptr [rcx-0F60h]
    vmovdqa xmm0, xmmword ptr [rcx-0F80h]
    vmovdqa xmm0, xmmword ptr [rcx-0FA0h]
    vmovdqa xmm0, xmmword ptr [rcx-0FC0h]
    vmovdqa xmm0, xmmword ptr [rcx-0FE0h]

    add rax,128     ; Just did 128 accesses

    cmp rax,rdx     ; have we completed the target number of accesses in total yet?
    jb myloop       ; make another unrolled pass on the memory
    
done:
    xor eax,eax     ; return 0
    ret

win_x86_64_asm_revStride2Read_Word128 endp
end
