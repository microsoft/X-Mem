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
asm_dummy_forwSequentialLoop_Word256 proc

; Arguments:
; rcx is address of the first 256-bit word in the array
; rdx is address of the last 256-bit word in the array

; rax holds current 256-bit word address
; ymm0 holds result from reading the memory 256-bit wide

	mov rax,rcx		; initialize current word address to start of the array
	cmp rax,rdx		; have we reached the last word yet?
	jae done		; if current word address is >= last word address, jump to done

myloop:
	; Loop is empty so that we can benchmark only loop overhead!

	add rax,1000h							; End of one unrolled loop iteration. Increment pointer by 128 words of size 32 bytes, which is 4096 bytes.
	
	cmp rax,rdx		; have we reached the last word yet?
	jae done		; if current word address is >= last word address, jump to done
	jmp myloop		; continue loop

done:
	xor eax,eax		; return 0
	ret

asm_dummy_forwSequentialLoop_Word256 endp
end
