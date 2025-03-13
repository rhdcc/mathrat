# run with 'nmake /Fwindows.mak'
all: mrat.exe

mrat.exe:
	if not exist .\bin\ mkdir .\bin\ ;
	clang -o ./bin/mrat.exe -std=c11 -Wall -pedantic ./src/main.c

clean:
	rmdir /S ./bin\

