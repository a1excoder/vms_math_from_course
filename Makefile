build:
	gcc main.c -L./ -lmathvms -o test
	del libmathvms.a

vms:
	gcc -c math_vms.c -o math_vms.o
	ar rcs libmathvms.a math_vms.o
	del math_vms.o