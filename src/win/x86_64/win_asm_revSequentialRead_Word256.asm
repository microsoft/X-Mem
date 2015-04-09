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

.code
win_asm_revSequentialRead_Word256 proc

; Arguments:
; rcx is address of the last 256-bit word in the array
; rdx is address of the first 256-bit word in the array

; rax holds current 256-bit word address
; ymm0 holds result from reading the memory 256-bit wide

	mov rax,rcx		; initialize current word address to start of the array
	cmp rax,rdx		; have we reached the first word yet?
	jbe done		; if current word address is <= first word address, jump to done

myloop:
	; Unroll 128 loads of 256-bit words (32 bytes is 20h) before checking loop condition.

	vmovdqa ymm0, ymmword ptr [rax]			
	vmovdqa ymm0, ymmword ptr [rax-0020h]
	vmovdqa ymm0, ymmword ptr [rax-0040h]
	vmovdqa ymm0, ymmword ptr [rax-0060h]
	vmovdqa ymm0, ymmword ptr [rax-0080h]
	vmovdqa ymm0, ymmword ptr [rax-00A0h]
	vmovdqa ymm0, ymmword ptr [rax-00C0h]
	vmovdqa ymm0, ymmword ptr [rax-00E0h]
	vmovdqa ymm0, ymmword ptr [rax-0100h]
	vmovdqa ymm0, ymmword ptr [rax-0120h]
	vmovdqa ymm0, ymmword ptr [rax-0140h]
	vmovdqa ymm0, ymmword ptr [rax-0160h]
	vmovdqa ymm0, ymmword ptr [rax-0180h]
	vmovdqa ymm0, ymmword ptr [rax-01A0h]
	vmovdqa ymm0, ymmword ptr [rax-01C0h]
	vmovdqa ymm0, ymmword ptr [rax-01E0h]
	vmovdqa ymm0, ymmword ptr [rax-0200h]
	vmovdqa ymm0, ymmword ptr [rax-0220h]
	vmovdqa ymm0, ymmword ptr [rax-0240h]
	vmovdqa ymm0, ymmword ptr [rax-0260h]
	vmovdqa ymm0, ymmword ptr [rax-0280h]
	vmovdqa ymm0, ymmword ptr [rax-02A0h]
	vmovdqa ymm0, ymmword ptr [rax-02C0h]
	vmovdqa ymm0, ymmword ptr [rax-02E0h]
	vmovdqa ymm0, ymmword ptr [rax-0300h]
	vmovdqa ymm0, ymmword ptr [rax-0320h]
	vmovdqa ymm0, ymmword ptr [rax-0340h]
	vmovdqa ymm0, ymmword ptr [rax-0360h]
	vmovdqa ymm0, ymmword ptr [rax-0380h]
	vmovdqa ymm0, ymmword ptr [rax-03A0h]
	vmovdqa ymm0, ymmword ptr [rax-03C0h]
	vmovdqa ymm0, ymmword ptr [rax-03E0h]
	vmovdqa ymm0, ymmword ptr [rax-0400h]
	vmovdqa ymm0, ymmword ptr [rax-0420h]
	vmovdqa ymm0, ymmword ptr [rax-0440h]
	vmovdqa ymm0, ymmword ptr [rax-0460h]
	vmovdqa ymm0, ymmword ptr [rax-0480h]
	vmovdqa ymm0, ymmword ptr [rax-04A0h]
	vmovdqa ymm0, ymmword ptr [rax-04C0h]
	vmovdqa ymm0, ymmword ptr [rax-04E0h]
	vmovdqa ymm0, ymmword ptr [rax-0500h]
	vmovdqa ymm0, ymmword ptr [rax-0520h]
	vmovdqa ymm0, ymmword ptr [rax-0540h]
	vmovdqa ymm0, ymmword ptr [rax-0560h]
	vmovdqa ymm0, ymmword ptr [rax-0580h]
	vmovdqa ymm0, ymmword ptr [rax-05A0h]
	vmovdqa ymm0, ymmword ptr [rax-05C0h]
	vmovdqa ymm0, ymmword ptr [rax-05E0h]
	vmovdqa ymm0, ymmword ptr [rax-0600h]
	vmovdqa ymm0, ymmword ptr [rax-0620h]
	vmovdqa ymm0, ymmword ptr [rax-0640h]
	vmovdqa ymm0, ymmword ptr [rax-0660h]
	vmovdqa ymm0, ymmword ptr [rax-0680h]
	vmovdqa ymm0, ymmword ptr [rax-06A0h]
	vmovdqa ymm0, ymmword ptr [rax-06C0h]
	vmovdqa ymm0, ymmword ptr [rax-06E0h]
	vmovdqa ymm0, ymmword ptr [rax-0700h]
	vmovdqa ymm0, ymmword ptr [rax-0720h]
	vmovdqa ymm0, ymmword ptr [rax-0740h]
	vmovdqa ymm0, ymmword ptr [rax-0760h]
	vmovdqa ymm0, ymmword ptr [rax-0780h]
	vmovdqa ymm0, ymmword ptr [rax-07A0h]
	vmovdqa ymm0, ymmword ptr [rax-07C0h]
	vmovdqa ymm0, ymmword ptr [rax-07E0h]
	vmovdqa ymm0, ymmword ptr [rax-0800h]
	vmovdqa ymm0, ymmword ptr [rax-0820h]
	vmovdqa ymm0, ymmword ptr [rax-0840h]
	vmovdqa ymm0, ymmword ptr [rax-0860h]
	vmovdqa ymm0, ymmword ptr [rax-0880h]
	vmovdqa ymm0, ymmword ptr [rax-08A0h]
	vmovdqa ymm0, ymmword ptr [rax-08C0h]
	vmovdqa ymm0, ymmword ptr [rax-08E0h]
	vmovdqa ymm0, ymmword ptr [rax-0900h]
	vmovdqa ymm0, ymmword ptr [rax-0920h]
	vmovdqa ymm0, ymmword ptr [rax-0940h]
	vmovdqa ymm0, ymmword ptr [rax-0960h]
	vmovdqa ymm0, ymmword ptr [rax-0980h]
	vmovdqa ymm0, ymmword ptr [rax-09A0h]
	vmovdqa ymm0, ymmword ptr [rax-09C0h]
	vmovdqa ymm0, ymmword ptr [rax-09E0h]
	vmovdqa ymm0, ymmword ptr [rax-0A00h]
	vmovdqa ymm0, ymmword ptr [rax-0A20h]
	vmovdqa ymm0, ymmword ptr [rax-0A40h]
	vmovdqa ymm0, ymmword ptr [rax-0A60h]
	vmovdqa ymm0, ymmword ptr [rax-0A80h]
	vmovdqa ymm0, ymmword ptr [rax-0AA0h]
	vmovdqa ymm0, ymmword ptr [rax-0AC0h]
	vmovdqa ymm0, ymmword ptr [rax-0AE0h]
	vmovdqa ymm0, ymmword ptr [rax-0B00h]
	vmovdqa ymm0, ymmword ptr [rax-0B20h]
	vmovdqa ymm0, ymmword ptr [rax-0B40h]
	vmovdqa ymm0, ymmword ptr [rax-0B60h]
	vmovdqa ymm0, ymmword ptr [rax-0B80h]
	vmovdqa ymm0, ymmword ptr [rax-0BA0h]
	vmovdqa ymm0, ymmword ptr [rax-0BC0h]
	vmovdqa ymm0, ymmword ptr [rax-0BE0h]
	vmovdqa ymm0, ymmword ptr [rax-0C00h]
	vmovdqa ymm0, ymmword ptr [rax-0C20h]
	vmovdqa ymm0, ymmword ptr [rax-0C40h]
	vmovdqa ymm0, ymmword ptr [rax-0C60h]
	vmovdqa ymm0, ymmword ptr [rax-0C80h]
	vmovdqa ymm0, ymmword ptr [rax-0CA0h]
	vmovdqa ymm0, ymmword ptr [rax-0CC0h]
	vmovdqa ymm0, ymmword ptr [rax-0CE0h]
	vmovdqa ymm0, ymmword ptr [rax-0D00h]
	vmovdqa ymm0, ymmword ptr [rax-0D20h]
	vmovdqa ymm0, ymmword ptr [rax-0D40h]
	vmovdqa ymm0, ymmword ptr [rax-0D60h]
	vmovdqa ymm0, ymmword ptr [rax-0D80h]
	vmovdqa ymm0, ymmword ptr [rax-0DA0h]
	vmovdqa ymm0, ymmword ptr [rax-0DC0h]
	vmovdqa ymm0, ymmword ptr [rax-0DE0h]
	vmovdqa ymm0, ymmword ptr [rax-0E00h]
	vmovdqa ymm0, ymmword ptr [rax-0E20h]
	vmovdqa ymm0, ymmword ptr [rax-0E40h]
	vmovdqa ymm0, ymmword ptr [rax-0E60h]
	vmovdqa ymm0, ymmword ptr [rax-0E80h]
	vmovdqa ymm0, ymmword ptr [rax-0EA0h]
	vmovdqa ymm0, ymmword ptr [rax-0EC0h]
	vmovdqa ymm0, ymmword ptr [rax-0EE0h]
	vmovdqa ymm0, ymmword ptr [rax-0F00h]
	vmovdqa ymm0, ymmword ptr [rax-0F20h]
	vmovdqa ymm0, ymmword ptr [rax-0F40h]
	vmovdqa ymm0, ymmword ptr [rax-0F60h]
	vmovdqa ymm0, ymmword ptr [rax-0F80h]
	vmovdqa ymm0, ymmword ptr [rax-0FA0h]
	vmovdqa ymm0, ymmword ptr [rax-0FC0h]
	vmovdqa ymm0, ymmword ptr [rax-0FE0h]

	sub rax,1000h							; End of one unrolled loop iteration. Decrement pointer by 128 words of size 32 bytes, which is 4096 bytes.
	
	cmp rax,rdx		; have we reached the first word yet?
	jbe done		; if current word address is <= first word address, jump to done
	jmp myloop		; continue loop

done:
	xor eax,eax		; return 0
	ret

win_asm_revSequentialRead_Word256 endp
end
