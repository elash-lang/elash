#include <elash/ast/dump/type.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/expr.h>

#include <elash/util/assert.h>

void el_ast_dump_type(ElAstType* node, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TYPE_NAME:
        fprintf(out, "TypeName(\""EL_SV_FMT"\")\n", EL_SV_FARG(node->as.name->name));
        return;
    case EL_AST_TYPE_REF:
        fprintf(out, "ReferenceType:\n");
        el_ast_dump_type(node->as.ref.base, indent + 1, out);
        return;
    case EL_AST_TYPE_SLICE:
        fprintf(out, "%sSliceType:\n", node->as.slice.is_raw ? "Raw" : "");
        el_ast_dump_type(node->as.slice.base, indent + 1, out);
        return;
    case EL_AST_TYPE_ARRAY:
        fprintf(out, "ArrayType:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "base:\n");
        el_ast_dump_type(node->as.array.base, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "size:\n");
        el_ast_dump_expr(node->as.array.size, indent + 2, out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, node->kind);
}
