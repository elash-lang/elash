#include <elash/ast/dump/type.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/expr.h>

void el_ast_dump_type(ElAstTypeNode* node, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TYPE_NAME:
        fprintf(out, "TypeName(\""EL_SV_FMT"\")\n", EL_SV_FARG(node->name->name));
        break;
    case EL_AST_TYPE_PTR:
        fprintf(out, "PointerType:\n");
        el_ast_dump_type(node->base, indent + 1, out);
        break;
    case EL_AST_TYPE_ARRAY:
        fprintf(out, "ArrayType:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "base:\n");
        el_ast_dump_type(node->array.base, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "size:\n");
        el_ast_dump_expr(node->array.size, indent + 2, out);
        break;
    }
}
