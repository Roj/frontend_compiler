#include "../libs/lexical.h"
#include "../libs/parser.h"
#include <check.h>

void parser_test(char* input, void(*Nonterminal)(void)) {
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	char errormsg[1000];
	sprintf(errormsg, "Failed: %s", input);
	ck_assert_msg(parse_unit(lex, Nonterminal), errormsg);
}

START_TEST (parser_assignment_test) {
	char* input = "a := 3;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping), 
		"Simple assignment is not in grammar");
}
END_TEST

START_TEST (parser_assignment_nosemic_test) {
	char* input = "a := 3";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(!parse_unit(lex, Grouping), 
		"Grammar does not detect missing semicolon in statement a=3");
}
END_TEST

START_TEST (parser_assignment_simple_expr_test) {
	char* input = "a := 3+3";
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

START_TEST (parser_unaryminus_test) {
	parser_test("-3", Factor);
	parser_test("-a", Factor);
	parser_test("-a+(-3)", Expression);
}
END_TEST

START_TEST (parser_logical_ops_test) {
	char* input = "not(a and b) or c +  not k - not b";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Expression),
		"Grammar does not support boolean expressions");
}
END_TEST

START_TEST (parser_expr_is_not_statement) {
	char* input = "b=a;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(!parse_unit(lex, Statement),
		"Failed: b=a; should not be allowed as statement");
}
END_TEST

START_TEST (parser_if_oneline_test) {
	char* input = "if a>b then b:=a;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then b:=a");
}
END_TEST

START_TEST (parser_if_manyline_test) {
	char* input = "if a>b then begin b:=a; b:=a; end;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then begin b:=a; b:=a; end;");
}
END_TEST

START_TEST (parser_if_oneline_else_oneline_test) {
	char* input = "if a>b then b:=a; else a:=b;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then b:=a; else a:=b;");
}
END_TEST

START_TEST (parser_if_manyline_else_oneline_test) {
	char* input = "if a>b then begin b:=a; b:=a; end; else b:=a;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then begin b:=a; b:=a; end; else b:=a;");
}
END_TEST

START_TEST (parser_if_manyline_else_manyline_test) {
	char* input = "if a>b then begin b:=a; b:=a; end; else begin b:=a; b:=a; end;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then begin b:=a; b:=a; end; else begin b:=a; b:=a; end;");
}
END_TEST

START_TEST (parser_if_oneline_else_manyline_test) {
	char* input = "if a>b then b:=a; else begin b:=a; b:=a; end;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then b:=a; else begin b:=a; b:=a; end;");
}
END_TEST

START_TEST (parser_for_simple_oneline_to) {
	parser_test("for a := 1 to 10 do b:=3;", Grouping);
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
	tcase_add_test(tc_core, parser_unaryminus_test);
	tcase_add_test(tc_core, parser_logical_ops_test);
	tcase_add_test(tc_core, parser_expr_is_not_statement);
	tcase_add_test(tc_core, parser_if_oneline_test);
	tcase_add_test(tc_core, parser_if_oneline_else_manyline_test);
	tcase_add_test(tc_core, parser_if_oneline_else_oneline_test);
	tcase_add_test(tc_core, parser_if_manyline_test);
	tcase_add_test(tc_core, parser_if_manyline_else_oneline_test);
	tcase_add_test(tc_core, parser_if_manyline_else_manyline_test);
	tcase_add_test(tc_core, parser_for_simple_oneline_to);
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


