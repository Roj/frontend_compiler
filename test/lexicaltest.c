#include "../libs/lexical.h"
#include <check.h>

//Tests that assert whether the structures work
//as expected.
START_TEST (lexeme_string_test) {
	lexeme_t lex;
	lex.type = IDENTIFIER;
	lex.data.name = "variablename";

	ck_assert_str_eq(lex.data.name, "variablename");
}
END_TEST

START_TEST (lexeme_number_test) {
	char* input = "123";
	lexeme_t* lex = process_string(input);
	ck_assert_ptr_ne(lex, NULL);
	ck_assert_msg(lex->type == NUMBER, "Type is not number");
	ck_assert_int_eq(lex->data.value, 123);
}
END_TEST

START_TEST (lexeme_2number_test) {
	char* input = "39 4032";

	lexeme_t* lex = process_string(input);
	ck_assert_ptr_ne(lex, NULL);
	ck_assert_msg(lex->type == NUMBER, "Type is not number");
	ck_assert_int_eq(lex->data.value, 39);

	lex = lex->next;
	ck_assert_ptr_ne(lex, NULL);
	ck_assert_msg(lex->type == NUMBER, "Type is not number");
	ck_assert_int_eq(lex->data.value, 4032);

}

END_TEST
START_TEST (lexeme_4number_octal_hex_test) {
	char* input = "0312 123 0XABcDeF 0 0X1dead2";
	int numbers[5] = {0312, 123, 0xABcDeF, 0, 0X1dead2};
	lexeme_t* current = process_string(input);
	int i = 0;
	while (current && current->type != EOI) {
		ck_assert_int_eq(current->data.value, numbers[i++]);
		current = current->next;
	}
	//Check if we got enough tokens.
	ck_assert_int_eq(i, 5);
}
END_TEST

START_TEST (lexeme_hex_syntax_error_test) {
	char* input = "0xDEADMAU5";
	process_string(input);
	ck_assert_int_gt(get_error_count(), 0);
}
END_TEST

START_TEST (lexeme_identifier_test) {
	char* input = "hola";
	lexeme_t* lex = process_string(input);
	ck_assert_msg(lex->type == IDENTIFIER, "Type is not identifier");
	ck_assert_str_eq(lex->data.name, "hola");
}
END_TEST

START_TEST (lexeme_2identifier_test) {
	char* input = "abc def";
	char* identifiers[2] = {"abc", "def"};
	lexeme_t* lex = process_string(input);
	int i = 0;
	while (lex && lex->type != EOI) {
		ck_assert_msg(lex->type == IDENTIFIER, "Type is not identifier");
		ck_assert_str_eq(lex->data.name, identifiers[i++]);
		lex = lex->next;
	}
}
END_TEST

START_TEST (lexeme_keyword_test) {
	char* input = "abc if for";
	lexeme_type_t types[3] = {IDENTIFIER, KW_IF, KW_FOR};
	lexeme_t* lex = process_string(input);
	int i = 0;
	while (lex && lex->type != EOI) {
		ck_assert_msg(lex->type == types[i++], "Type is not correct");
		lex = lex->next;
	}
}
END_TEST


Suite* lexical_suite(void) {
	Suite* s = suite_create("Lexical analyzer");
	TCase* tc_core = tcase_create("Core");
	tcase_add_test(tc_core, lexeme_string_test);
	tcase_add_test(tc_core, lexeme_number_test);
	tcase_add_test(tc_core, lexeme_2number_test);
	tcase_add_test(tc_core, lexeme_4number_octal_hex_test);
	tcase_add_test(tc_core, lexeme_hex_syntax_error_test);
	tcase_add_test(tc_core, lexeme_identifier_test);
	tcase_add_test(tc_core, lexeme_2identifier_test);
	tcase_add_test(tc_core, lexeme_keyword_test);
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
