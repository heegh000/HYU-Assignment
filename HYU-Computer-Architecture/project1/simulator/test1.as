	lw 0 1 8	laod 5 to reg1 (numeric)
	lw 0 2 neg3	load -3 to reg2 (symbolic)
	lw 0 3 ten	load 10 to reg3 (symbolic)
start	add 1 2 1	reg1 = reg1 + reg2 (5 - 3) 
	add 1 3 4	reg4 = reg1 + reg3 (2 + 10)
done halt 
ten .fill 10 
neg3 .fill -3
five .fill 5
