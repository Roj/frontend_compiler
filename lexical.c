#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <check.h>

typedef enum _lexeme_type_t { 
	NUMBER,
	IDENTIFIER,
	OP_PLUS,
	OP_MINUS,
	KW_IF,
	KW_FOR,
	KW_BLOCK_START,
	KW_BLOCK_END

} lexeme_type_t;

typedef struct lexeme {
	lexeme_type_t type;
	union data {
		char* name;
		double dvalue;
		int value;
	} data;
} lexeme_t;

START_TEST (lexeme_string_test) {
	ck_assert_int_eq(3,3);
}
END_TEST

Suite* lexical_suite(void) {
	Suite* s;
	TCase* tc_core;
	s = suite_create("Lexical analyzer");
	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, lexeme_string_test);
	suite_add_tcase(s, tc_core);
	return s;
}

int main() { 
	//char* input = "1 + 2";
	Suite* s = lexical_suite();
	SRunner* sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0)? EXIT_SUCCESS : EXIT_FAILURE;
}
