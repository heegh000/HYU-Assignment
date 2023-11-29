	lw 0 1 5	laod 2 to reg1 (numeric)
	lw 0 2 ten	load 10 to reg3 (symbolic)
	nor 1 2 3	reg3 = reg1 nor reg2 ( ~(2 | 10) ) 
done halt 
ten .fill 10 
two .fill 2
