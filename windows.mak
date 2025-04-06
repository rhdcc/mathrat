# run with 'nmake /Fwindows.mak'
all: mrat.exe

./src/tests/tests.c:
	if not exist .\src\tests\bin\ mkdir .\src\tests\bin\ ;
	clang -o ./src/tests/bin/make_tests.exe -std=c11 -Wall -pedantic -D_CRT_SECURE_NO_WARNINGS ./src/tests/make_tests.c
	.\src\tests\bin\make_tests.exe
	rmdir /Q/S .\src\tests\bin\

mrat.exe: ./src/tests/tests.c
	if not exist .\bin\ mkdir .\bin\ ;
	clang -o ./bin/mrat.exe -std=c11 -Wall -pedantic -I./src/ ./src/main.c ./src/integer.c ./src/temp_buffer.c
	del /Q .\src\tests\tests.c

clean:
	if exist .\bin\ rmdir /Q/S .\bin\ ;
	if exist .\src\tests\bin\ rmdir /Q/S .\src\tests\bin\ ;

