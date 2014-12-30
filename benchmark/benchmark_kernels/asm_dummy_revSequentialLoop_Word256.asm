; (C) 2014 Microsoft Corporation

.code
asm_dummy_revSequentialLoop_Word256 proc

; Arguments:
; rcx is address of the last 256-bit word in the array
; rdx is address of the first 256-bit word in the array

; rax holds current 256-bit word address
; ymm0 holds result from reading the memory 256-bit wide

	mov rax,rcx		; initialize current word address to start of the array
	cmp rax,rdx		; have we reached the first word yet?
	jbe done		; if current word address is <= first word address, jump to done

myloop:
	; Loop is empty so that we can benchmark only loop overhead!

	sub rax,1000h							; End of one unrolled loop iteration. Decrement pointer by 128 words of size 32 bytes, which is 4096 bytes.
	
	cmp rax,rdx		; have we reached the first word yet?
	jbe done		; if current word address is <= first word address, jump to done
	jmp myloop		; continue loop

done:
	xor eax,eax		; return 0
	ret

asm_dummy_revSequentialLoop_Word256 endp
end
