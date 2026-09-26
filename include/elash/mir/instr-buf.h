#pragma once

#include <elash/mir/instr.h>

typedef struct ElMirInstrBuf {
    ElMirInstr** items;
    usize len, cap;
} ElMirInstrBuf;

void el_mir_ibuf_init(ElMirInstrBuf* ibuf);
void el_mir_ibuf_destroy(ElMirInstrBuf* ibuf);

void el_mir_ibuf_copy(const ElMirInstrBuf* src, ElMirInstrBuf* dst);
void el_mir_ibuf_move(ElMirInstrBuf* src, ElMirInstrBuf* dst);

void el_mir_ibuf_resize(ElMirInstrBuf* ibuf, usize new_size);
void el_mir_ibuf_reserve(ElMirInstrBuf* ibuf, usize min_cap);
void el_mir_ibuf_reserve_exact(ElMirInstrBuf* ibuf, usize new_cap);

void el_mir_ibuf_push(ElMirInstrBuf* ibuf, ElMirInstr* instr);

void el_mir_ibuf_clear(ElMirInstrBuf* ibuf);
