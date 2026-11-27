#include <elash/ast/dump/indent.h>
#include <elash/ast/dump/type.h>
#include <elash/ast/dump/expr.h>
#include <elash/ast/dump/decl.h>

#include <elash/util/assert.h>

void el_ast_dump_type(ElAstType* node, usize indent, ElWriter* out) {
    el_ast_dump_print_indent(indent, out);
    switch (node->kind) {
    case EL_AST_TYPE_NAME:
        el_writef(out, "TypeName(\""EL_SV_FMT"\")\n", EL_SV_FARG(node->as.name->name));
        return;
    case EL_AST_TYPE_REF:
        el_swrite(out, "ReferenceType:\n");
        el_ast_dump_type(node->as.ref.base, indent + 1, out);
        return;
    case EL_AST_TYPE_OPT:
        el_swrite(out, "OptionalType:\n");
        el_ast_dump_type(node->as.opt.base, indent + 1, out);
        return;
    case EL_AST_TYPE_SLICE:
        el_writef(out, "%sSliceType:\n", node->as.slice.is_raw ? "Raw" : "");
        el_ast_dump_type(node->as.slice.base, indent + 1, out);
        return;
    case EL_AST_TYPE_ARRAY:
        el_swrite(out, "ArrayType:\n");
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "base:\n");
        el_ast_dump_type(node->as.array.base, indent + 2, out);
        el_ast_dump_print_indent(indent + 1, out);
        el_swrite(out, "size:\n");
        el_ast_dump_expr(node->as.array.size, indent + 2, out);
        return;
    case EL_AST_TYPE_STRUCT:
        el_swrite(out, "StructType:\n");
        for (ElAstDecl* decl = node->as.struct_.fields; decl != NULL; decl = decl->next) {
            el_ast_dump_decl(decl, indent + 2, out);
        }
        return;
    case EL_AST_TYPE_TUPLE:
        el_swrite(out, "TupleType:\n");
        for (ElAstType* type = node->as.tuple.head; type != NULL; type = type->next) {
            el_ast_dump_type(type, indent + 1, out);
        }
        return;
    }
    EL_UNREACHABLE_ENUM_VAL(ElAstTypeKind, node->kind);
}
