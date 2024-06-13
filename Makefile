Shell.exe:
	gcc -g shell.c -o Shell.exe

clean:
	rm Shell.exe

all: clean Shell.exe