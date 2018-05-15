For now I won't add functions. 
Therefore we have:  

## Control structures
```
Program -> PROGRAM IDENTIFIER; ConstantDeclarations TypeDeclarations FuncProcDeclarations BEGIN Grouping END.

ConstantDeclarations -> const ConstDecl
ConstantDeclarations -> epsilon
ConstDecl -> IDENTIFIER = Expression; ConstDeclPrime
ConstDeclPrime -> IDENTIFIER = Expression;
ConstDeclPrime -> epsilon
TypeDeclarations -> VAR TypeDecl TypeDeclarations
TypeDeclarations -> epsilon
TypeDecl -> Variables: Type; TypeDeclPrime
TypeDeclPrime -> Variables: Type;
TypeDeclPrime -> epsilon
Variables -> IDENTIFIER VariablesPrime
VariablesPrime -> , IDENTIFIER VariablesPrime
VariablesPrime -> epsilon
Type -> INTEGER
Type -> ARRAY [ Expression .. Expression ] OF INTEGER

FuncProcDeclarations -> KW_PROCEDURE ProcDecl FuncProcDeclarations
FuncProcDeclarations -> KW_FUNCTION FuncDecl FuncProcDeclarations
FuncProcDeclarations -> epsilon
ProcDecl -> IDENTIFIER ParamsList; ForwardOrCode 
FuncDecl -> IDENTIFIER ParamsList : Type; ForwardOrCode
ForwardOrCode -> KW_FORWARD;
ForwardOrCode -> TypeDeclarations Block;
ParamsList -> (Variables : Type RestParams)
ParamsList -> epsilon
RestParams -> ; Variables : Type RestParams
RestParams -> epsilon

Grouping -> if Expression then Block Else Grouping
Grouping -> for Identifier := Expression ForDirection Expression do Block Grouping
Grouping -> while Expression do Block Grouping
Grouping -> Statement GroupingPrime
Grouping -> epsilon
GroupingPrime -> ; Grouping
GroupingPrime -> epsilon //the last statement can have no ;
ForDirection -> TO
ForDirection -> DOWNTO
Statement -> KW_EXIT
Statement -> IDENTIFIER IdentifierStatement
IdentifierStatement -> := Expression
IdentifierStatement -> [ Expression ] := Expression
IdentifierStatement -> (Arguments)
Else -> ELSE Block
Else -> epsilon
Block -> begin Grouping end;
Block -> Statement;
```

## Expressions
```
Expression -> Expression MOD Term
Expression -> Expression + Term
Expression -> Expression - Term
Expression -> Expression >= Term
Expression -> Expression <= Term
Expression -> Expression > Term
Expression -> Expression < Term
Expression -> Expression <> Term
Expression -> Expression = Term
Expression -> ! Expression
Expression -> Expression AND Term
Expression -> Expression OR Term
Expression -> Term
Term -> Term * Factor
Term -> Term / Factor
Term -> Term INTDIV Factor
Term -> Factor
Factor -> IDENTIIFER FuncCallOrArrayIndex
Factor -> NUMBER
Factor -> (Expression)
Factor -> -Factor
FuncCallOrArrayIndex -> FuncCall
FuncCallOrArrayIndex -> [ Expression ]
FuncCallOrArrayIndex -> epsilon
FuncCall -> (Arguments)
FuncCall -> epsilon
Arguments-> ExprOrLiteral RestArgs
RestArgs -> , ExprOrLiteral RestArgs
Arguments-> epsilon
RestArgs -> epsilon
ExprOrLiteral -> Expression
ExprOrLiteral -> LITERAL
```

## Expressions -- LL1 conversion
```
Expression -> Term ExpressionPrime
ExpressionPrime -> MOD Term ExpressionPrime
ExpressionPrime -> >= Term ExpressionPrime
ExpressionPrime -> <= Term ExpressionPrime
ExpressionPrime -> > Term ExpressionPrime
ExpressionPrime -> < Term ExpressionPrime
ExpressionPrime -> <> Term ExpressionPrime
ExpressionPrime -> = Term ExpressionPrime
ExpressionPrime -> + Term ExpressionPrime
ExpressionPrime -> - Term ExpressionPrime
ExpressionPrime -> and Term ExpressionPrime
ExpressionPrime -> or Term ExpressionPrime
ExpressionPrime -> epsilon
Term -> Factor TermPrime
TermPrime -> * Factor TermPrime
TermPrime -> / Factor TermPrime
TermPrime -> INTDIV Factor TermPrime
TermPrime -> epsilon
Factor -> ! Factor
Factor -> IDENTIFIER FuncCall
Factor -> NUMBER
Factor -> (Expression)
FuncCall -> (Arguments)
FuncCall -> epsilon
Arguments-> ExprOrLiteral RestArgs
RestArgs -> , ExprOrLiteral RestArgs
Arguments-> epsilon
RestArgs -> epsilon
ExprOrLiteral -> Expression
ExprOrLiteral -> LITERAL
```
