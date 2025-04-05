#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "integer.h"
#include "temp_buffer.h"

int test_counter = 0;

void test1(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "832987473298230000043281476942104327189472819432646328";
	const char *str2 = "335623382879320043276483204324329843243204328727671132";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test2(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "999999";
	const char *str2 = "1";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test2a(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "99999";
	const char *str2 = "1";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

void test3(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "1000";
	const char *str2 = "1";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test4(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "1444";
	const char *str2 = "775";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test4a(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "775";
	const char *str2 = "1444";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test5(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "99999";
	const char *str2 = "100000";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test5a(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str1 = "100011";
	const char *str2 = "100011";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s - %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer difference = integer_subtract(&A, &B);
	integer_debug_print(&difference);
}

void test6(void) { // NOTE: Leaks memory...
	tmp_free();
	const char *str2 = "3283882";
	const char *str1 = "-998211";
	printf("\n ---- TEST #%d ---- \n", ++test_counter);
	printf("Evaluate: %s + %s = ??\n", str1, str2);
	Integer A = integer_from_str((char *)str1, strlen(str1));
	Integer B = integer_from_str((char *)str2, strlen(str2));
	integer_debug_print(&A);
	integer_debug_print(&B);
	Integer sum = integer_add(&A, &B);
	integer_debug_print(&sum);
}

int main(void) {
	test1();
	test2();
	test2a();
	test3();
	test4();
	test4a();
	test5();
	test5a();
	test6();
	return 0;
}
