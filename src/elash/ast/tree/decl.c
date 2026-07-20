#include <elash/ast/tree/decl.h>

void el_ast_append_decl(ElAstDecl** head, ElAstDecl** tail, ElAstDecl* decl) {
    if (*head == NULL) {
        *head = decl;
        *tail = decl;
    } else {
        (*tail)->next = decl;
        *tail = decl;
    }
}
