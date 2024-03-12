.PHONY: clean

out: calc case_all
# Your code here.
case_add: 
	./casegen add 100 > case_add
case_sub: 
	./casegen sub 100 > case_sub
case_mul: 
	./casegen mul 100 > case_mul
case_div: 
	./casegen div 100 > case_div
case_all:
	make case_add
	make case_sub
	make case_mul
	make case_div
	cat < case_add < case_sub < case_mul < case_div > case_all
calc: calc.c
	gcc -o calc calc.c
casegen: casegen
	gcc -o casegen casegen.c
clean:
	rm -f out calc casegen case_* *.o
