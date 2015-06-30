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
win_x86_64_asm_revSequentialWrite_Word128 proc

; Arguments:
; rcx is address of the last 128-bit word in the array
; rdx is address of the first 128-bit word in the array

; rax holds current 128-bit word address
; xmm0 holds result from reading the memory 128-bit wide

    mov rax,rcx     ; initialize current word address to start of the array
    
    ; initialize xmm0 to the value to write to memory. We will use all 1s, e.g. 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    ; I am using the first 256-bits of the memory array to initialize ymm0. Is there a better way to do this?
    mov qword ptr [rdx],0FFFFFFFFh
    mov qword ptr [rdx+8h],0FFFFFFFFh
    mov qword ptr [rdx+10h],0FFFFFFFFh
    mov qword ptr [rdx+18h],0FFFFFFFFh
    vmovdqu xmm0, xmmword ptr[rdx]

    cmp rax,rdx     ; have we reached the first word yet?
    jbe done        ; if current word address is <= first word address, jump to done

myloop:
    ; Unroll 256 stores of 128-bit words (16 bytes is 10h) before checking loop condition.
    vmovdqa xmmword ptr [rax-0000h], xmm0
    vmovdqa xmmword ptr [rax-0010h], xmm0
    vmovdqa xmmword ptr [rax-0020h], xmm0
    vmovdqa xmmword ptr [rax-0030h], xmm0
    vmovdqa xmmword ptr [rax-0040h], xmm0
    vmovdqa xmmword ptr [rax-0050h], xmm0
    vmovdqa xmmword ptr [rax-0060h], xmm0
    vmovdqa xmmword ptr [rax-0070h], xmm0
    vmovdqa xmmword ptr [rax-0080h], xmm0
    vmovdqa xmmword ptr [rax-0090h], xmm0
    vmovdqa xmmword ptr [rax-00A0h], xmm0
    vmovdqa xmmword ptr [rax-00B0h], xmm0
    vmovdqa xmmword ptr [rax-00C0h], xmm0
    vmovdqa xmmword ptr [rax-00D0h], xmm0
    vmovdqa xmmword ptr [rax-00E0h], xmm0
    vmovdqa xmmword ptr [rax-00F0h], xmm0
    vmovdqa xmmword ptr [rax-0100h], xmm0           
    vmovdqa xmmword ptr [rax-0110h], xmm0
    vmovdqa xmmword ptr [rax-0120h], xmm0
    vmovdqa xmmword ptr [rax-0130h], xmm0
    vmovdqa xmmword ptr [rax-0140h], xmm0
    vmovdqa xmmword ptr [rax-0150h], xmm0
    vmovdqa xmmword ptr [rax-0160h], xmm0
    vmovdqa xmmword ptr [rax-0170h], xmm0
    vmovdqa xmmword ptr [rax-0180h], xmm0
    vmovdqa xmmword ptr [rax-0190h], xmm0
    vmovdqa xmmword ptr [rax-01A0h], xmm0
    vmovdqa xmmword ptr [rax-01B0h], xmm0
    vmovdqa xmmword ptr [rax-01C0h], xmm0
    vmovdqa xmmword ptr [rax-01D0h], xmm0
    vmovdqa xmmword ptr [rax-01E0h], xmm0
    vmovdqa xmmword ptr [rax-01F0h], xmm0
    vmovdqa xmmword ptr [rax-0200h], xmm0           
    vmovdqa xmmword ptr [rax-0210h], xmm0
    vmovdqa xmmword ptr [rax-0220h], xmm0
    vmovdqa xmmword ptr [rax-0230h], xmm0
    vmovdqa xmmword ptr [rax-0240h], xmm0
    vmovdqa xmmword ptr [rax-0250h], xmm0
    vmovdqa xmmword ptr [rax-0260h], xmm0
    vmovdqa xmmword ptr [rax-0270h], xmm0
    vmovdqa xmmword ptr [rax-0280h], xmm0
    vmovdqa xmmword ptr [rax-0290h], xmm0
    vmovdqa xmmword ptr [rax-02A0h], xmm0
    vmovdqa xmmword ptr [rax-02B0h], xmm0
    vmovdqa xmmword ptr [rax-02C0h], xmm0
    vmovdqa xmmword ptr [rax-02D0h], xmm0
    vmovdqa xmmword ptr [rax-02E0h], xmm0
    vmovdqa xmmword ptr [rax-02F0h], xmm0
    vmovdqa xmmword ptr [rax-0300h], xmm0           
    vmovdqa xmmword ptr [rax-0310h], xmm0
    vmovdqa xmmword ptr [rax-0320h], xmm0
    vmovdqa xmmword ptr [rax-0330h], xmm0
    vmovdqa xmmword ptr [rax-0340h], xmm0
    vmovdqa xmmword ptr [rax-0350h], xmm0
    vmovdqa xmmword ptr [rax-0360h], xmm0
    vmovdqa xmmword ptr [rax-0370h], xmm0
    vmovdqa xmmword ptr [rax-0380h], xmm0
    vmovdqa xmmword ptr [rax-0390h], xmm0
    vmovdqa xmmword ptr [rax-03A0h], xmm0
    vmovdqa xmmword ptr [rax-03B0h], xmm0
    vmovdqa xmmword ptr [rax-03C0h], xmm0
    vmovdqa xmmword ptr [rax-03D0h], xmm0
    vmovdqa xmmword ptr [rax-03E0h], xmm0
    vmovdqa xmmword ptr [rax-03F0h], xmm0
    vmovdqa xmmword ptr [rax-0400h], xmm0           
    vmovdqa xmmword ptr [rax-0410h], xmm0
    vmovdqa xmmword ptr [rax-0420h], xmm0
    vmovdqa xmmword ptr [rax-0430h], xmm0
    vmovdqa xmmword ptr [rax-0440h], xmm0
    vmovdqa xmmword ptr [rax-0450h], xmm0
    vmovdqa xmmword ptr [rax-0460h], xmm0
    vmovdqa xmmword ptr [rax-0470h], xmm0
    vmovdqa xmmword ptr [rax-0480h], xmm0
    vmovdqa xmmword ptr [rax-0490h], xmm0
    vmovdqa xmmword ptr [rax-04A0h], xmm0
    vmovdqa xmmword ptr [rax-04B0h], xmm0
    vmovdqa xmmword ptr [rax-04C0h], xmm0
    vmovdqa xmmword ptr [rax-04D0h], xmm0
    vmovdqa xmmword ptr [rax-04E0h], xmm0
    vmovdqa xmmword ptr [rax-04F0h], xmm0
    vmovdqa xmmword ptr [rax-0500h], xmm0           
    vmovdqa xmmword ptr [rax-0510h], xmm0
    vmovdqa xmmword ptr [rax-0520h], xmm0
    vmovdqa xmmword ptr [rax-0530h], xmm0
    vmovdqa xmmword ptr [rax-0540h], xmm0
    vmovdqa xmmword ptr [rax-0550h], xmm0
    vmovdqa xmmword ptr [rax-0560h], xmm0
    vmovdqa xmmword ptr [rax-0570h], xmm0
    vmovdqa xmmword ptr [rax-0580h], xmm0
    vmovdqa xmmword ptr [rax-0590h], xmm0
    vmovdqa xmmword ptr [rax-05A0h], xmm0
    vmovdqa xmmword ptr [rax-05B0h], xmm0
    vmovdqa xmmword ptr [rax-05C0h], xmm0
    vmovdqa xmmword ptr [rax-05D0h], xmm0
    vmovdqa xmmword ptr [rax-05E0h], xmm0
    vmovdqa xmmword ptr [rax-05F0h], xmm0
    vmovdqa xmmword ptr [rax-0600h], xmm0           
    vmovdqa xmmword ptr [rax-0610h], xmm0
    vmovdqa xmmword ptr [rax-0620h], xmm0
    vmovdqa xmmword ptr [rax-0630h], xmm0
    vmovdqa xmmword ptr [rax-0640h], xmm0
    vmovdqa xmmword ptr [rax-0650h], xmm0
    vmovdqa xmmword ptr [rax-0660h], xmm0
    vmovdqa xmmword ptr [rax-0670h], xmm0
    vmovdqa xmmword ptr [rax-0680h], xmm0
    vmovdqa xmmword ptr [rax-0690h], xmm0
    vmovdqa xmmword ptr [rax-06A0h], xmm0
    vmovdqa xmmword ptr [rax-06B0h], xmm0
    vmovdqa xmmword ptr [rax-06C0h], xmm0
    vmovdqa xmmword ptr [rax-06D0h], xmm0
    vmovdqa xmmword ptr [rax-06E0h], xmm0
    vmovdqa xmmword ptr [rax-06F0h], xmm0
    vmovdqa xmmword ptr [rax-0700h], xmm0           
    vmovdqa xmmword ptr [rax-0710h], xmm0
    vmovdqa xmmword ptr [rax-0720h], xmm0
    vmovdqa xmmword ptr [rax-0730h], xmm0
    vmovdqa xmmword ptr [rax-0740h], xmm0
    vmovdqa xmmword ptr [rax-0750h], xmm0
    vmovdqa xmmword ptr [rax-0760h], xmm0
    vmovdqa xmmword ptr [rax-0770h], xmm0
    vmovdqa xmmword ptr [rax-0780h], xmm0
    vmovdqa xmmword ptr [rax-0790h], xmm0
    vmovdqa xmmword ptr [rax-07A0h], xmm0
    vmovdqa xmmword ptr [rax-07B0h], xmm0
    vmovdqa xmmword ptr [rax-07C0h], xmm0
    vmovdqa xmmword ptr [rax-07D0h], xmm0
    vmovdqa xmmword ptr [rax-07E0h], xmm0
    vmovdqa xmmword ptr [rax-07F0h], xmm0
    vmovdqa xmmword ptr [rax-0800h], xmm0
    vmovdqa xmmword ptr [rax-0810h], xmm0
    vmovdqa xmmword ptr [rax-0820h], xmm0
    vmovdqa xmmword ptr [rax-0830h], xmm0
    vmovdqa xmmword ptr [rax-0840h], xmm0
    vmovdqa xmmword ptr [rax-0850h], xmm0
    vmovdqa xmmword ptr [rax-0860h], xmm0
    vmovdqa xmmword ptr [rax-0870h], xmm0
    vmovdqa xmmword ptr [rax-0880h], xmm0
    vmovdqa xmmword ptr [rax-0890h], xmm0
    vmovdqa xmmword ptr [rax-08A0h], xmm0
    vmovdqa xmmword ptr [rax-08B0h], xmm0
    vmovdqa xmmword ptr [rax-08C0h], xmm0
    vmovdqa xmmword ptr [rax-08D0h], xmm0
    vmovdqa xmmword ptr [rax-08E0h], xmm0
    vmovdqa xmmword ptr [rax-08F0h], xmm0
    vmovdqa xmmword ptr [rax-0900h], xmm0           
    vmovdqa xmmword ptr [rax-0910h], xmm0
    vmovdqa xmmword ptr [rax-0920h], xmm0
    vmovdqa xmmword ptr [rax-0930h], xmm0
    vmovdqa xmmword ptr [rax-0940h], xmm0
    vmovdqa xmmword ptr [rax-0950h], xmm0
    vmovdqa xmmword ptr [rax-0960h], xmm0
    vmovdqa xmmword ptr [rax-0970h], xmm0
    vmovdqa xmmword ptr [rax-0980h], xmm0
    vmovdqa xmmword ptr [rax-0990h], xmm0
    vmovdqa xmmword ptr [rax-09A0h], xmm0
    vmovdqa xmmword ptr [rax-09B0h], xmm0
    vmovdqa xmmword ptr [rax-09C0h], xmm0
    vmovdqa xmmword ptr [rax-09D0h], xmm0
    vmovdqa xmmword ptr [rax-09E0h], xmm0
    vmovdqa xmmword ptr [rax-09F0h], xmm0
    vmovdqa xmmword ptr [rax-0A00h], xmm0           
    vmovdqa xmmword ptr [rax-0A10h], xmm0
    vmovdqa xmmword ptr [rax-0A20h], xmm0
    vmovdqa xmmword ptr [rax-0A30h], xmm0
    vmovdqa xmmword ptr [rax-0A40h], xmm0
    vmovdqa xmmword ptr [rax-0A50h], xmm0
    vmovdqa xmmword ptr [rax-0A60h], xmm0
    vmovdqa xmmword ptr [rax-0A70h], xmm0
    vmovdqa xmmword ptr [rax-0A80h], xmm0
    vmovdqa xmmword ptr [rax-0A90h], xmm0
    vmovdqa xmmword ptr [rax-0AA0h], xmm0
    vmovdqa xmmword ptr [rax-0AB0h], xmm0
    vmovdqa xmmword ptr [rax-0AC0h], xmm0
    vmovdqa xmmword ptr [rax-0AD0h], xmm0
    vmovdqa xmmword ptr [rax-0AE0h], xmm0
    vmovdqa xmmword ptr [rax-0AF0h], xmm0
    vmovdqa xmmword ptr [rax-0B00h], xmm0           
    vmovdqa xmmword ptr [rax-0B10h], xmm0
    vmovdqa xmmword ptr [rax-0B20h], xmm0
    vmovdqa xmmword ptr [rax-0B30h], xmm0
    vmovdqa xmmword ptr [rax-0B40h], xmm0
    vmovdqa xmmword ptr [rax-0B50h], xmm0
    vmovdqa xmmword ptr [rax-0B60h], xmm0
    vmovdqa xmmword ptr [rax-0B70h], xmm0
    vmovdqa xmmword ptr [rax-0B80h], xmm0
    vmovdqa xmmword ptr [rax-0B90h], xmm0
    vmovdqa xmmword ptr [rax-0BA0h], xmm0
    vmovdqa xmmword ptr [rax-0BB0h], xmm0
    vmovdqa xmmword ptr [rax-0BC0h], xmm0
    vmovdqa xmmword ptr [rax-0BD0h], xmm0
    vmovdqa xmmword ptr [rax-0BE0h], xmm0
    vmovdqa xmmword ptr [rax-0BF0h], xmm0
    vmovdqa xmmword ptr [rax-0C00h], xmm0           
    vmovdqa xmmword ptr [rax-0C10h], xmm0
    vmovdqa xmmword ptr [rax-0C20h], xmm0
    vmovdqa xmmword ptr [rax-0C30h], xmm0
    vmovdqa xmmword ptr [rax-0C40h], xmm0
    vmovdqa xmmword ptr [rax-0C50h], xmm0
    vmovdqa xmmword ptr [rax-0C60h], xmm0
    vmovdqa xmmword ptr [rax-0C70h], xmm0
    vmovdqa xmmword ptr [rax-0C80h], xmm0
    vmovdqa xmmword ptr [rax-0C90h], xmm0
    vmovdqa xmmword ptr [rax-0CA0h], xmm0
    vmovdqa xmmword ptr [rax-0CB0h], xmm0
    vmovdqa xmmword ptr [rax-0CC0h], xmm0
    vmovdqa xmmword ptr [rax-0CD0h], xmm0
    vmovdqa xmmword ptr [rax-0CE0h], xmm0
    vmovdqa xmmword ptr [rax-0CF0h], xmm0
    vmovdqa xmmword ptr [rax-0D00h], xmm0           
    vmovdqa xmmword ptr [rax-0D10h], xmm0
    vmovdqa xmmword ptr [rax-0D20h], xmm0
    vmovdqa xmmword ptr [rax-0D30h], xmm0
    vmovdqa xmmword ptr [rax-0D40h], xmm0
    vmovdqa xmmword ptr [rax-0D50h], xmm0
    vmovdqa xmmword ptr [rax-0D60h], xmm0
    vmovdqa xmmword ptr [rax-0D70h], xmm0
    vmovdqa xmmword ptr [rax-0D80h], xmm0
    vmovdqa xmmword ptr [rax-0D90h], xmm0
    vmovdqa xmmword ptr [rax-0DA0h], xmm0
    vmovdqa xmmword ptr [rax-0DB0h], xmm0
    vmovdqa xmmword ptr [rax-0DC0h], xmm0
    vmovdqa xmmword ptr [rax-0DD0h], xmm0
    vmovdqa xmmword ptr [rax-0DE0h], xmm0
    vmovdqa xmmword ptr [rax-0DF0h], xmm0
    vmovdqa xmmword ptr [rax-0E00h], xmm0           
    vmovdqa xmmword ptr [rax-0E10h], xmm0
    vmovdqa xmmword ptr [rax-0E20h], xmm0
    vmovdqa xmmword ptr [rax-0E30h], xmm0
    vmovdqa xmmword ptr [rax-0E40h], xmm0
    vmovdqa xmmword ptr [rax-0E50h], xmm0
    vmovdqa xmmword ptr [rax-0E60h], xmm0
    vmovdqa xmmword ptr [rax-0E70h], xmm0
    vmovdqa xmmword ptr [rax-0E80h], xmm0
    vmovdqa xmmword ptr [rax-0E90h], xmm0
    vmovdqa xmmword ptr [rax-0EA0h], xmm0
    vmovdqa xmmword ptr [rax-0EB0h], xmm0
    vmovdqa xmmword ptr [rax-0EC0h], xmm0
    vmovdqa xmmword ptr [rax-0ED0h], xmm0
    vmovdqa xmmword ptr [rax-0EE0h], xmm0
    vmovdqa xmmword ptr [rax-0EF0h], xmm0
    vmovdqa xmmword ptr [rax-0F00h], xmm0           
    vmovdqa xmmword ptr [rax-0F10h], xmm0
    vmovdqa xmmword ptr [rax-0F20h], xmm0
    vmovdqa xmmword ptr [rax-0F30h], xmm0
    vmovdqa xmmword ptr [rax-0F40h], xmm0
    vmovdqa xmmword ptr [rax-0F50h], xmm0
    vmovdqa xmmword ptr [rax-0F60h], xmm0
    vmovdqa xmmword ptr [rax-0F70h], xmm0
    vmovdqa xmmword ptr [rax-0F80h], xmm0
    vmovdqa xmmword ptr [rax-0F90h], xmm0
    vmovdqa xmmword ptr [rax-0FA0h], xmm0
    vmovdqa xmmword ptr [rax-0FB0h], xmm0
    vmovdqa xmmword ptr [rax-0FC0h], xmm0
    vmovdqa xmmword ptr [rax-0FD0h], xmm0
    vmovdqa xmmword ptr [rax-0FE0h], xmm0
    vmovdqa xmmword ptr [rax-0FF0h], xmm0

    sub rax,1000h                           ; End of one unrolled loop iteration. Decrement pointer by 256 words of size 16 bytes, which is 4096 bytes.
    
    cmp rax,rdx     ; have we reached the last word yet?
    jbe done        ; if current word address is <= first word address, jump to done
    jmp myloop      ; continue loop

done:
    xor eax,eax     ; return 0
    ret

win_x86_64_asm_revSequentialWrite_Word128 endp
end
