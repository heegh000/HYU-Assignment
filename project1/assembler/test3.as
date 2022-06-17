	lw 0 1 7	laod 2 to reg1 (numeric)
	lw 0 2 ten	load 10 to reg3 (symbolic)
	add 1 2 3	reg3 = reg1 + reg2 (2+10)
    sw 0 3 ten	store reg3 to mem6 (symbolic)
    sw 0 3 7	store reg3 to mem7 (numeric)
done halt 
ten .fill 10 
two .fill 2
