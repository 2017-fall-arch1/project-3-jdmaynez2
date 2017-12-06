	.arch msp430g2553
	.p2align 1,0
	.text
	
	.data
num:	.word 0 		

	.text
buzz:	.word if		
        .word elseif 	
        .word elseiftwo   
        .word lastif      


	.global states
states:
	
if:
	call #p2sw_read
	cmp #1, r12
	jnz elseif
	mov #1, r12
	jmp end
elseif:
	call #p2sw_read
	cmp #2, r12
	jnz elseiftwo
	mov #2, r12
	jmp end
elseiftwo:
	call #p2sw_read
	cmp #3, r12
	jnz lastif
	mov #3, r12
	jmp end
lastif:
    call #p2sw_read
	cmp #4, r12
	jnz end
	mov #4, r12
	jmp end
end:
	pop r0			; return 
