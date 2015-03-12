mov SP, 65535
mov AX, 5
mov BX, 2
add AX, BX
push AX
pop BX
print BX
hlt
 