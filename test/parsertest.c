#include "../libs/lexical.h"
#include "../libs/parser.h"
#include <check.h>

START_TEST (parser_assignment_test) {
	char* input = "a = 3;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping), 
		"Simple assignment is not in grammar");
}
END_TEST

START_TEST (parser_assignment_nosemic_test) {
	char* input = "a = 3";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(!parse_unit(lex, Grouping), 
		"Grammar does not detect missing semicolon in statement a=3");
}
END_TEST

START_TEST (parser_assignment_simple_expr_test) {
	char* input = "a = 3+3";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Statement),
		"Grammar does not support assignment of expr as statement");
}
END_TEST

START_TEST (parser_compound_expr_test) {
	char* input = "3+3+(4*2)";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Expression), 
		"Grammar does not support compound expressions");
}
END_TEST

START_TEST (parser_complete_compound_expr_test) {
	char* input = "3+3+(4*2)/2-(3+2)";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Expression),
		"Grammar does not support harder comp. expressions");
}
END_TEST

START_TEST (parser_boolean_test) {
	char* input = "1>2";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Expression),
		"Grammar does not support boolean expressions");
}
END_TEST

START_TEST (parser_logical_ops_test) {
	char* input = "!(a&&b)||c + !k - !b";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Expression),
		"Grammar does not support boolean expressions");
}
END_TEST


Suite* parser_suite(void) {
	Suite* suite = suite_create("Syntactical analyzer");
	TCase* tc_core = tcase_create("Core");
	tcase_add_test(tc_core, parser_assignment_test);
	tcase_add_test(tc_core, parser_assignment_nosemic_test);
	tcase_add_test(tc_core, parser_assignment_simple_expr_test);
	tcase_add_test(tc_core, parser_compound_expr_test);
	tcase_add_test(tc_core, parser_complete_compound_expr_test);
	tcase_add_test(tc_core, parser_boolean_test);
	tcase_add_test(tc_core, parser_logical_ops_test);
	suite_add_tcase(suite, tc_core);
	return suite;
}

int main() {
	Suite* suite = parser_suite();
	SRunner* runner = srunner_create(suite);
	srunner_set_log(runner, "parser_test.log");
	srunner_run_all(runner, CK_NORMAL);
	int number_failed = srunner_ntests_failed(runner);
	srunner_free(runner);
	return (number_failed == 0)? EXIT_SUCCESS : EXIT_FAILURE;
}


