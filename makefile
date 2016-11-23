#format is target-name: target dependencies
#{-tab-}actions

# All Targets
all: myELF

# Tool invocations
myELF: myELF.o
	gcc -g -m32 -Wall -o myELF myELF.o
myELF.o: myELF.c
	gcc -m32 -g -Wall -c -o myELF.o myELF.c

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o myELF