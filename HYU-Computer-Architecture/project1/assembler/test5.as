	lw 0 1 one	load 1 to reg1 (symbolic)
	lw 0 2 ten	load 10 to reg2 (symbolic)
start	add 1 3 3	reg3 = reg1 + reg3
	beq 3 2 1	if reg3 == reg2 break
	beq 0 0 start
done halt
ten .fill 10 
one .fill 1
