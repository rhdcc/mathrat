bin/mrat: ./src/tests/tests.c
	mkdir -p ./bin/
	clang -o ./bin/mrat -std=c11 -Wall -pedantic -I./src/ ./src/main.c ./src/integer.c ./src/temp_buffer.c
	rm ./src/tests/tests.c

./src/tests/tests.c:
	mkdir -p ./src/tests/bin/
	clang -o ./src/tests/bin/make_tests -std=c11 -Wall -pedantic -D_CRT_SECURE_NO_WARNINGS ./src/tests/make_tests.c
	./src/tests/bin/make_tests

clean:
	rm -rf ./bin/
	rm -rf ./src/tests/bin/
