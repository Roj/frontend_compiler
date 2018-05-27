#ifndef _CODEGEN_H
#define _CODEGEN_H

//Based on an AST, emit an objectfile `fileout` for a hardcoded target.
void codegen(NodeProgram* root, char* fileout);

#endif
