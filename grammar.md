For now I won't add functions. 
Therefore we have:  

## Control structures
```
Program -> PROGRAM IDENTIFIER; ConstantDeclarations TypeDeclarations BEGIN Grouping END.

ConstantDeclarations -> const ConstDecl
ConstantDeclarations -> epsilon
ConstDecl -> IDENTIFIER = Expression; ConstDeclPrime
ConstDeclPrime -> IDENTIFIER = Expression;
ConstDeclPrime -> epsilon

TypeDeclarations -> VAR TypeDecl;
TypeDeclarations -> epsilon
TypeDecl -> IDENTIFIER: Type TypeDeclPrime
TypeDeclPrime -> ; IDENTIFIER: Type
TypeDeclPrime -> epsilon
Type -> INTEGER
Type -> ARRAY [ NUMBER .. NUMBER ] OF INTEGER

Grouping -> if Expression then Block Else Grouping
Grouping -> for Identifier := Expression ForDirection Expression do Block Grouping
Grouping -> while Expression do Block Grouping
Grouping -> Statement; Grouping
ForDirection -> TO
ForDirection -> DOWNTO
Statement -> IDENTIFIER IdentifierStatement
IdentifierStatement -> := Expression
IdentifierStatement -> [ Expression ] := Expression
IdentifierStatement -> (Arguments)
Else -> ELSE Block
Else -> epsilon
Block -> begin Grouping end;
Block -> Statement;
//Block -> Statement
Grouping -> epsilon
```

## Expressions
```
Expression -> Expression + Term
Expression -> Expression - Term
Expression -> Expression >= Term
Expression -> Expression <= Term
Expression -> Expression > Term
Expression -> Expression < Term
Expression -> ! Expression
Expression -> Expression AND Term
Expression -> Expression OR Term
Expression -> Term
Term -> Term * Factor
Term -> Term / Factor
Term -> Factor
Factor -> IDENTIIFER FuncCall
Factor -> NUMBER
Factor -> (Expression)
Factor -> -Factor
FuncCall -> (Arguments)
FuncCall -> epsilon
Arguments-> Expression, Arguments
Arguments-> Expression
Arguments-> epsilon
```

## Expressions -- LL1 conversion
```
Expression -> Term ExpressionPrime
ExpressionPrime -> >= Term ExpressionPrime
ExpressionPrime -> <= Term ExpressionPrime
ExpressionPrime -> > Term ExpressionPrime
ExpressionPrime -> < Term ExpressionPrime
ExpressionPrime -> + Term ExpressionPrime
ExpressionPrime -> - Term ExpressionPrime
ExpressionPrime -> and Term ExpressionPrime
ExpressionPrime -> or Term ExpressionPrime
ExpressionPrime -> epsilon
Term -> Factor TermPrime
TermPrime -> * Factor TermPrime
TermPrime -> / Factor TermPrime
TermPrime -> epsilon
Factor -> ! Factor
Factor -> IDENTIFIER FuncCall
Factor -> NUMBER
Factor -> (Expression)
FuncCall -> (Arguments)
FuncCall -> epsilon
Arguments-> Expression RestArgs
RestArgs -> , Expression RestArgs
Arguments-> epsilon
RestArgs -> epsilon
```
