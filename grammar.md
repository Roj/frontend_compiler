For now I won't add functions. 
Therefore we have:  

## Control structures
```
Grouping -> if(Expression) Block Else Grouping
Grouping -> for(Statement; Expression; Statement) Block Grouping
Grouping -> Statement; Grouping
Statement -> Identifier = Expression 
Else -> else Block
Else -> epsilon
Block -> {Grouping}
Block -> Statement;
Block -> S
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
Expression -> Expression && Term
Expression -> Expression || Term
Expression -> Term
Term -> Term * Factor
Term -> Term / Factor
Term -> Factor
Factor -> Identifier
Factor -> Number
Factor -> (Expression)
```

## Expressions -- left recursion removal
```
Expression -> Term ExpressionPrime
ExpressionPrime -> >= Term ExpressionPrime
ExpressionPrime -> <= Term ExpressionPrime
ExpressionPrime -> > Term ExpressionPrime
ExpressionPrime -> < Term ExpressionPrime
ExpressionPrime -> + Term ExpressionPrime
ExpressionPrime -> - Term ExpressionPrime
ExpressionPrime -> && Term ExpressionPrime
ExpressionPrime -> || Term ExpressionPrime
ExpressionPrime -> epsilon
Term -> Factor TermPrime
TermPrime -> * Factor TermPrime
TermPrime -> / Factor TermPrime
TermPrime -> epsilon
Factor -> ! Factor
Factor -> IDENTIFIER
Factor -> (Expression)
```
