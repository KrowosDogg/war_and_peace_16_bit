;numeric system base translator program

	mov SP, 65535	; should set SP to the end of RAM manually
	input AX		; number to change system base
	input BX		; base of the target numeric system
	
	mov CX, 0		; counter of digits pushed to stack
loop:
	cmp AX, 0		; while (number != 0) {
	je exit_from_loop
	div AX, BX		; radix = number%base, number /= base
	add CX, 1		; counter_of_digits += 1
	push DX			; store digit in stack
	jmp loop		; }
exit_from_loop:
	
loop2:
	cmp CX, 0		; while (counter_of_digits != 0) {
	je exit_from_loop2
	pop DX			; remember last stored digit in stack
	print DX		; print the digit
	sub CX, 1		; counter_of_digits -= 1
	jmp loop2		; }
exit_from_loop2:
	hlt
 