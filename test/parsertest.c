#include "../libs/lexical.h"
#include "../libs/parser.h"
#include <check.h>

#define parser_test(i, f) parser_test_((i),(Nonterminal) (f))
#define parser_neg_test(i, f) parser_neg_test_((i), (Nonterminal)(f))

void parser_test_(char* input, Nonterminal nt) {
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	char errormsg[1000];
	sprintf(errormsg, "Failed: %s", input);
	ck_assert_msg(parse_unit(lex, nt), errormsg);
}

void parser_neg_test_(char* input, Nonterminal nt) {
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	char errormsg[1000];
	sprintf(errormsg, "Should not pass: %s", input);
	ck_assert_msg(! parse_unit(lex, nt), errormsg);
}

START_TEST (parser_assignment_test) {
	char* input = "a := 3;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping), 
		"Simple assignment is not in grammar");
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
	char* input = "(3+3+(4*2) mod 4) div 3";
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

START_TEST (parser_expr_funccall) {
	parser_test("a-b(3)", Expression);
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

START_TEST (parser_statement_funccall) {
	parser_test("writeln(32);", Grouping);
}
END_TEST

START_TEST (parser_statement_funccall_empty) {
	parser_test("writeln();", Grouping);
}
END_TEST

START_TEST (parser_statement_funccall_manyargs) {
	parser_test("writeln(a,b,c);", Grouping);
}
END_TEST

START_TEST (parser_statement_funccall_manyargs_expr) {
	parser_test("writeln(a,-b,c+1);", Grouping);
}
END_TEST

START_TEST (parser_statement_funccall_manyargs_literal) {
	parser_test("writeln(a,-b,'abc?ak_if then else');", Grouping);
}
END_TEST

START_TEST (parser_statement_array_assignment) {
	parser_test("a[3]:=b;", Grouping);
}
END_TEST

START_TEST (parser_statement_exit) {
	parser_test("exit;", Grouping);
}
END_TEST

START_TEST (parser_block_nosemionlast_test) {
	parser_test("begin a:=b; a:=b end", Block);
}
END_TEST

START_TEST (parser_block_missing_semi_test) {
	parser_neg_test("begin a:=b a:=b end;", Block);
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
	char* input = "if a>b then b:=a else a:=b;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then b:=a else a:=b;");
}
END_TEST

START_TEST (parser_if_manyline_else_oneline_test) {
	char* input = "if a>b then begin b:=a; b:=a; end else b:=a;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then begin b:=a; b:=a; end else b:=a;");
}
END_TEST

START_TEST (parser_if_manyline_else_manyline_test) {
	char* input = "if a>b then begin b:=a; b:=a; end else begin b:=a; b:=a; end;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then begin b:=a; b:=a; end else begin b:=a; b:=a; end;");
}
END_TEST

START_TEST (parser_if_oneline_else_manyline_test) {
	char* input = "if a>b then b:=a else begin b:=a; b:=a; end;";
	bool unfinished_comment = false;
	lexeme_t* lex = process_string(input, &unfinished_comment);
	ck_assert_msg(parse_unit(lex, Grouping),
		"Failed: if a>b then b:=a else begin b:=a; b:=a; end;");
}
END_TEST

START_TEST (parser_for_simple_oneline_to) {
	parser_test("for a := 1 to 10 do b:=3;", Grouping);
}
END_TEST

START_TEST (parser_for_simple_oneline_downto) {
	parser_test("for a := 1 downto -10 do b:=3;", Grouping);
}
END_TEST

START_TEST (parser_for_expressions_oneline) {
	parser_test("for a := (1+c)*3 to 15*c do b:=3;", Grouping);
}
END_TEST

START_TEST (parser_for_expressions_multiline) {
	parser_test("for a := (1+c)*3 to 15*c do begin b:=3; b:=3; end;", Grouping);
}
END_TEST

START_TEST (parser_while_simple_oneline) {
	parser_test("while a>b do a:=a+1;", Grouping);
}
END_TEST

START_TEST (parser_while_expression_oneline) {
	parser_test("while (a>b) and (a<MAXINT) do a:=a+1;", Grouping);
}
END_TEST

START_TEST (parser_while_simple_multiline) {
	parser_test("while a>b do begin a:=a+1; a:=a+1; end;", Grouping);
}
END_TEST

START_TEST (parser_constant_declaration_simple) {
	parser_test("const a = 10;", ConstantDeclarations);
}
END_TEST

START_TEST (parser_constant_declaration_expr) {
	parser_test("const a = $10+5*(3*15-9*&10);", ConstantDeclarations);
}
END_TEST

START_TEST (parser_constant_declaration_many) {
	parser_test("const a = 3; b=4;", ConstantDeclarations);
}
END_TEST

START_TEST (parser_constant_declaration_invalid) {
	parser_neg_test("const ;", ConstantDeclarations);
	parser_neg_test("const a=3;b=4", ConstantDeclarations);
}
END_TEST

START_TEST (parser_var_declaration_simple) {
	parser_test("var a: integer;", TypeDeclarations);
}
END_TEST

START_TEST (parser_var_declaration_multiple) {
	parser_test("var a: integer; c:integer;", TypeDeclarations);
}
END_TEST

START_TEST (parser_var_declaration_multiple_comma) {
	parser_test("var a,b: integer; c:integer;", TypeDeclarations);
}
END_TEST

START_TEST (parser_var_declaration_array) {
	parser_test("var a: array[0..20] of integer;", TypeDeclarations);
}
END_TEST

START_TEST (parser_var_declaration_array_minus) {
	parser_test("var a: array[-20..20] of integer;", TypeDeclarations);
}
END_TEST

START_TEST (parser_func_decl_noargs_novars) {
	parser_test("function abc : integer; begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_func_decl_onearg_novars) {
	parser_test("function abc(a: integer) : integer; begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_func_decl_manyarg_novars) {
	parser_test("function abc(a,b: integer; c:integer) : integer; begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_func_decl_noargs_vars) {
	parser_test("function abc:integer; var a,b: integer;begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_proc_decl_noargs_novars) {
	parser_test("procedure abc; begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_proc_decl_onearg_novars) {
	parser_test("procedure abc(a: integer); begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_proc_decl_manyarg_novars) {
	parser_test("procedure abc(a,b: integer; c:integer); begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_proc_decl_noargs_vars) {
	parser_test("procedure abc; var a,b: integer;begin a:=b; end;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_func_decl_forward) {
	parser_test("function abc:integer; forward;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_proc_decl_forward) {
	parser_test("procedure abc; forward;", 
		FuncProcDeclarations);
}
END_TEST

START_TEST (parser_program_empty) {
	parser_test("program abc; begin end.", Program);
}
END_TEST

START_TEST (parser_program_empty_type) {
	parser_test("program abc; var a:integer; begin end.", Program);
}
END_TEST

START_TEST (parser_program_empty_const) {
	parser_test("program abc; const a = $10; begin end.", Program);
}
END_TEST

START_TEST (parser_program_empty_const_type) {
	parser_test("program abc; const a=10; var b:integer; begin end.", Program);
}
END_TEST

Suite* parser_suite(void) {
	Suite* suite = suite_create("Syntactical analyzer");
	TCase* tc_core = tcase_create("Core");
	tcase_add_test(tc_core, parser_assignment_test);
	tcase_add_test(tc_core, parser_assignment_simple_expr_test);
	tcase_add_test(tc_core, parser_compound_expr_test);
	tcase_add_test(tc_core, parser_complete_compound_expr_test);
	tcase_add_test(tc_core, parser_boolean_test);
	tcase_add_test(tc_core, parser_unaryminus_test);
	tcase_add_test(tc_core, parser_logical_ops_test);
	tcase_add_test(tc_core, parser_expr_funccall);
	tcase_add_test(tc_core, parser_expr_is_not_statement);
	tcase_add_test(tc_core, parser_statement_funccall);
	tcase_add_test(tc_core, parser_statement_funccall_empty);
	tcase_add_test(tc_core, parser_statement_funccall_manyargs);
	tcase_add_test(tc_core, parser_statement_funccall_manyargs_expr);
	tcase_add_test(tc_core, parser_statement_funccall_manyargs_literal);
	tcase_add_test(tc_core, parser_statement_array_assignment);
	tcase_add_test(tc_core, parser_statement_exit);
	tcase_add_test(tc_core, parser_block_nosemionlast_test);
	tcase_add_test(tc_core, parser_block_missing_semi_test);
	tcase_add_test(tc_core, parser_if_oneline_test);
	tcase_add_test(tc_core, parser_if_oneline_else_manyline_test);
	tcase_add_test(tc_core, parser_if_oneline_else_oneline_test);
	tcase_add_test(tc_core, parser_if_manyline_test);
	tcase_add_test(tc_core, parser_if_manyline_else_oneline_test);
	tcase_add_test(tc_core, parser_if_manyline_else_manyline_test);
	tcase_add_test(tc_core, parser_for_simple_oneline_to);
	tcase_add_test(tc_core, parser_for_simple_oneline_downto);
	tcase_add_test(tc_core, parser_for_expressions_oneline);
	tcase_add_test(tc_core, parser_for_expressions_multiline);
	tcase_add_test(tc_core, parser_while_simple_oneline);
	tcase_add_test(tc_core, parser_while_expression_oneline);
	tcase_add_test(tc_core, parser_while_simple_multiline);
	tcase_add_test(tc_core, parser_constant_declaration_simple);
	tcase_add_test(tc_core, parser_constant_declaration_expr);
	tcase_add_test(tc_core, parser_constant_declaration_many);
	tcase_add_test(tc_core, parser_constant_declaration_invalid);
	tcase_add_test(tc_core, parser_var_declaration_simple);
	tcase_add_test(tc_core, parser_var_declaration_multiple);
	tcase_add_test(tc_core, parser_var_declaration_multiple_comma);
	tcase_add_test(tc_core, parser_var_declaration_array);
	tcase_add_test(tc_core, parser_var_declaration_array_minus);
	tcase_add_test(tc_core, parser_func_decl_noargs_novars);
	tcase_add_test(tc_core, parser_func_decl_onearg_novars);
	tcase_add_test(tc_core, parser_func_decl_manyarg_novars);
	tcase_add_test(tc_core, parser_func_decl_noargs_vars);
	tcase_add_test(tc_core, parser_func_decl_forward);
	tcase_add_test(tc_core, parser_proc_decl_noargs_novars);
	tcase_add_test(tc_core, parser_proc_decl_onearg_novars);
	tcase_add_test(tc_core, parser_proc_decl_manyarg_novars);
	tcase_add_test(tc_core, parser_proc_decl_noargs_vars);
	tcase_add_test(tc_core, parser_proc_decl_forward);
	tcase_add_test(tc_core, parser_program_empty);
	tcase_add_test(tc_core, parser_program_empty_type);
	tcase_add_test(tc_core, parser_program_empty_const);
	tcase_add_test(tc_core, parser_program_empty_const_type);
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


