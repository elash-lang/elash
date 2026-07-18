#include <elash/ast/dump/type.h>
#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/expr.h>

#include <elash/util/assert.h>

void el_ast_dump_type(ElAstType* node, usize indent, FILE* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TYPE_NAME:
        fprintf(out, "TypeName(\""EL_SV_FMT"\")\n", EL_SV_FARG(node->name->name));
        return;
    case EL_AST_TYPE_REF:
        fprintf(out, "ReferenceType:\n");
        el_ast_dump_type(node->base, indent + 1, out);
        return;
    case EL_AST_TYPE_RAW_SLICE:
        fprintf(out, "RawSliceType:\n");
        el_ast_dump_type(node->base, indent + 1, out);
        return;
    case EL_AST_TYPE_SLICE:
        fprintf(out, "SliceType:\n");
        el_ast_dump_type(node->base, indent + 1, out);
        return;
    case EL_AST_TYPE_ARRAY:
        fprintf(out, "ArrayType:\n");
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "base:\n");
        el_ast_dump_type(node->array.base, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        fprintf(out, "size:\n");
        el_ast_dump_expr(node->array.size, indent + 2, out);
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, node->kind);
}
