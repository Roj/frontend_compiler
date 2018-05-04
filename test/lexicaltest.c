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
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_ptr_ne(lex, NULL);
	ck_assert_msg(lex->type == NUMBER, "Type is not number");
	ck_assert_int_eq(lex->data.value, 123);
}
END_TEST

START_TEST (lexeme_2number_test) {
	char* input = "39 4032";

	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
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
	bool unfinished_comment = false;
	lexeme_t* current = process_string(input, &unfinished_comment);
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
	bool unfinished_comment = false;
	process_string(input, &unfinished_comment);
	ck_assert_int_gt(get_error_count(), 0);
}
END_TEST

START_TEST (lexeme_identifier_test) {
	char* input = "hola";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(lex->type == IDENTIFIER, "Type is not identifier");
	ck_assert_str_eq(lex->data.name, "hola");
}
END_TEST

START_TEST (lexeme_2identifier_test) {
	char* input = "abc def";
	char* identifiers[2] = {"abc", "def"};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	int i = 0;
	while (lex && lex->type != EOI) {
		ck_assert_msg(lex->type == IDENTIFIER, "Type is not identifier");
		ck_assert_str_eq(lex->data.name, identifiers[i++]);
		lex = lex->next;
	}
}
END_TEST

START_TEST (lexeme_unexpected_identifier_test) {
	char* input = "541oops";
	bool unfinished_comment = false;
	process_string(input, &unfinished_comment);
	ck_assert_msg(get_error_count() > 0, "Did not get a lexical error");

}
END_TEST

void check_types(lexeme_type_t types[], lexeme_t* lex) {
	int i = 0;
	while (lex && lex->type != EOI) {
		char msg[500];
		sprintf(msg, "Type at i=%d is not correct: expected %s, got %s",
			i, lextype2str(types[i]), lex2str(lex));
		ck_assert_msg(lex->type == types[i++], msg);
		lex = lex->next;
	}
	
}

START_TEST (lexeme_keyword_test) {
	char* input = "abc if for";
	lexeme_type_t types[3] = {IDENTIFIER, KW_IF, KW_FOR};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_expression_test) {
	char* input = "123+ 192 - j/3 == 9";
	lexeme_type_t types[9] = {
		NUMBER, OP_PLUS, NUMBER, OP_MINUS, IDENTIFIER, OP_DIV, NUMBER,
		OP_EQUALS, NUMBER
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_assign_test) {
	char* input = "a = 3 == 9";
	lexeme_type_t types[5] = {
		IDENTIFIER, ASSIGN, NUMBER, OP_EQUALS, NUMBER
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_grouping_test) {
	char* input = "if (a[3]+9) {}";
	//Indentation should reflect level of nesting.
	lexeme_type_t types[] = {
		KW_IF, 
		PARENS_START,
			IDENTIFIER, 
			BRACKET_START, NUMBER, BRACKET_END,
			OP_PLUS, NUMBER, 
		PARENS_END, 
		BLOCK_START, BLOCK_END
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_comment_test) {
	char* input = "a/3 /*divides by three*/";
	lexeme_type_t types[3] = {
		IDENTIFIER,
		OP_DIV,
		NUMBER
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_unfinished_comment_test) {
	char* input = "a/3 /* divides by throops 1+2\n";
	bool unfinished_comment = false;
	process_string(input, &unfinished_comment);

	ck_assert_msg(unfinished_comment, 
		"Unfinished comment state should be true");
}
END_TEST

START_TEST (lexeme_not_test) {
	char* input = "a = !b";
	lexeme_type_t types[4] = {IDENTIFIER, ASSIGN, OP_NOT, IDENTIFIER};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_boolean_op_test) {
	char* input = "!(b&&a) || 0x0";
	lexeme_type_t types[8] = {OP_NOT, PARENS_START, IDENTIFIER, OP_AND, IDENTIFIER, PARENS_END, OP_OR, NUMBER};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
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
	tcase_add_test(tc_core, lexeme_unexpected_identifier_test);
	tcase_add_test(tc_core, lexeme_keyword_test);
	tcase_add_test(tc_core, lexeme_expression_test);
	tcase_add_test(tc_core, lexeme_grouping_test);
	tcase_add_test(tc_core, lexeme_comment_test);
	tcase_add_test(tc_core, lexeme_unfinished_comment_test);
	tcase_add_test(tc_core, lexeme_assign_test);
	tcase_add_test(tc_core, lexeme_not_test);
	tcase_add_test(tc_core, lexeme_boolean_op_test);
	suite_add_tcase(s, tc_core);
	return s;
}


int main() {
	Suite* s = lexical_suite();
	SRunner* sr = srunner_create(s);
	srunner_set_log (sr, "test.log");
	srunner_run_all(sr, CK_NORMAL);
	int number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0)? EXIT_SUCCESS : EXIT_FAILURE;
}
