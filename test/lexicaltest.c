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
	char* input = "&0312 123 $ABcDeF 0 $1dead2";
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
	char* input = "hola_123";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(lex->type == IDENTIFIER, "Type is not identifier");
	ck_assert_str_eq(lex->data.name, "hola_123");
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

START_TEST (lexeme_literal_test) {
	char* input = "'4?if=kabcde_!'";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(lex->type == LITERAL, "Type is not LITERAL");
	ck_assert_str_eq(lex->data.name, "4?if=kabcde_!");
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
	char* input = "abc if for to downto do begin end program var integer while of function procedure forward mod div array const";
	lexeme_type_t types[20] = {IDENTIFIER, KW_IF, KW_FOR,
		KW_TO, KW_DOWNTO, KW_DO, BLOCK_START, BLOCK_END, 
		KW_PROGRAM, KW_VAR, TYPE_INTEGER,
		KW_WHILE,KW_OF, KW_FUNCTION, KW_PROCEDURE, KW_FORWARD,
		OP_MOD, OP_INTDIV, KW_ARRAY, KW_CONST};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_expression_test) {
	char* input = "123+ 192 - j/3 = 9";
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
	char* input = "a := 3 = 9";
	lexeme_type_t types[5] = {
		IDENTIFIER, ASSIGN, NUMBER, OP_EQUALS, NUMBER
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_type_assign_test) {
	char* input = "a: integer; a := 3";
	lexeme_type_t types[7] = {
		IDENTIFIER, ASSIGN_TYPE, TYPE_INTEGER, STM_END,
		IDENTIFIER, ASSIGN, NUMBER
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_grouping_test) {
	char* input = "if a[3]+9 then begin end";
	//Indentation should reflect level of nesting.
	lexeme_type_t types[] = {
		KW_IF, 
			IDENTIFIER, 
			BRACKET_START, NUMBER, BRACKET_END,
			OP_PLUS, NUMBER, 
		KW_THEN,
		BLOCK_START, BLOCK_END
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_comment_test) {
	char* input = "a/3 {divides by three}";
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
	char* input = "a/3 { divides by throops 1+2\n";
	bool unfinished_comment = false;
	process_string(input, &unfinished_comment);

	ck_assert_msg(unfinished_comment, 
		"Unfinished comment state should be true");
}
END_TEST

START_TEST (lexeme_not_test) {
	char* input = "a := not b";
	lexeme_type_t types[4] = {IDENTIFIER, ASSIGN, OP_NOT, IDENTIFIER};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_boolean_op_test) {
	char* input = "not (b and a) or $0";
	lexeme_type_t types[8] = {OP_NOT, PARENS_START, IDENTIFIER, OP_AND,
		IDENTIFIER, PARENS_END, OP_OR, NUMBER};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_neq_test) {
	char* input = "(a<3 <> b>4) = (a<=4 <> 5)";
	lexeme_type_t types[17] = {
		PARENS_START, 
			IDENTIFIER, OP_LT, NUMBER, 
			OP_NEQUALS, 
			IDENTIFIER, OP_GT, NUMBER,
		PARENS_END,
		OP_EQUALS,
		PARENS_START,
			IDENTIFIER, OP_LTE, NUMBER, 
			OP_NEQUALS, 
			NUMBER,
		PARENS_END
	};
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);

	check_types(types, lex);
}
END_TEST

START_TEST (lexeme_program_structure_test) {
	char* input = "program abc; begin end.";
	lexeme_type_t types[6] = {
		KW_PROGRAM, IDENTIFIER, STM_END,
		BLOCK_START, BLOCK_END, PROGRAM_END
	};
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
	tcase_add_test(tc_core, lexeme_literal_test);
	tcase_add_test(tc_core, lexeme_keyword_test);
	tcase_add_test(tc_core, lexeme_expression_test);
	tcase_add_test(tc_core, lexeme_grouping_test);
	tcase_add_test(tc_core, lexeme_comment_test);
	tcase_add_test(tc_core, lexeme_unfinished_comment_test);
	tcase_add_test(tc_core, lexeme_assign_test);
	tcase_add_test(tc_core, lexeme_type_assign_test);
	tcase_add_test(tc_core, lexeme_not_test);
	tcase_add_test(tc_core, lexeme_boolean_op_test);
	tcase_add_test(tc_core, lexeme_neq_test);
	tcase_add_test(tc_core, lexeme_program_structure_test);
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
