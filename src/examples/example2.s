label1: 
a:	mov SP, 65535
b:	mov AX, 5
c:	mov BX, 2
d:	add AX, BX
e:	jmp label2
label2:
	push AX
	pop BX
	print BX
	hlt
 