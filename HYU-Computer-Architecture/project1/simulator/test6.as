	lw 0 1 ten	load 10 to reg1
	lw 0 2 neg3 load -3 to reg2
	add 1 2 3 reg3 = reg1 + reg2
	jalr 3 4 Stroe PC+1 to reg4 and Jump 
	add 2 3 4
	add 5 6 7
	sw 1 2 3
	halt
ten .fill 10
neg3 .fill -3
