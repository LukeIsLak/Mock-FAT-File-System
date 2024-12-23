.PHONY all:
all:
	gcc -Wall -D PART1 main.c -o diskinfo -lm
	gcc -Wall -D PART2 main.c -o disklist -lm
	gcc -Wall -D PART3 main.c -o diskget -lm
	gcc -Wall -D PART4 main.c -o diskput -lm

.PHONY clean:
clean:
	-rm diskinfo disklist diskget diskput