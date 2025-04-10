//
// Created by Brandon Howe on 4/7/25.
//

#include <string.h>
#include "optimizer_tac.h"

void remove_phi_expressions(TACList* list)
{
    for (int i = list->count - 1; i >= 0; i--)
    {
        if (list->items[i].operation == TAC_OP_PHI)
        {
            int64_t found_count = 0;
            for (int j = i; j >= 0; j--)
            {
                if (list->items[j].lhs.symbol == list->items[i].rhs1.symbol)
                {
                    list->items[j].lhs.symbol = list->items[i].lhs.symbol;
                    found_count += 1;
                }
                if (list->items[j].lhs.symbol == list->items[i].rhs2.symbol)
                {
                    list->items[j].lhs.symbol = list->items[i].lhs.symbol;
                    found_count += 1;
                }
                if (found_count >= 2) break;
            }
            list->count -= 1;
            memmove(&list->items[i], &list->items[i + 1], (list->count - i) * sizeof(TACExpr));
            i--;
        }
    }
}

void mark_symbol_live(TACList* list, bool* live_status, TACSymbol symbol)
{
    for (int i = list->count - 1; i >= 0; i--)
    {
        bool is_live = false;
        if (symbol.type == TAC_SYMBOL_TYPE_SYMBOL)
        {
            if (list->items[i].lhs.type == TAC_SYMBOL_TYPE_SYMBOL && list->items[i].lhs.symbol == symbol.symbol)
            {
                is_live = true;
            }
        }
        else if (symbol.type == TAC_SYMBOL_TYPE_VARIABLE)
        {
            if (list->items[i].lhs.type == TAC_SYMBOL_TYPE_VARIABLE &&
                bh_str_equal(list->items[i].lhs.variable.data, symbol.variable.data) &&
                list->items[i].lhs.variable.version == symbol.variable.version)
            {
                is_live = true;
            }
        }
        if (is_live)
        {
            live_status[i] = true;
            mark_symbol_live(list, live_status, list->items[i].rhs1);
            mark_symbol_live(list, live_status, list->items[i].rhs2);
            for (int j = 0; j < list->items[i].arg_count; j++)
            {
                mark_symbol_live(list, live_status, list->items[i].args[j]);
            }
            break;
        }
    }
}

void eliminate_dead_tac(TACList* list)
{
    bh_allocator allocator = GPA;

    bool* live_status = bh_alloc(allocator, sizeof(bool) * list->count);

    for (int i = 0; i < list->count; i++)
    {
        if (list->items[i].operation >= TAC_OP_CALL) live_status[i] = true;
    }
    mark_symbol_live(list, live_status, (TACSymbol){ .type = TAC_SYMBOL_TYPE_SYMBOL, .symbol = 0 });

    // Remove unused expressions
    int64_t unused_count = 0;
    for (int i = 0; i < list->count; i++)
    {
        if (live_status[i])
        {
            list->items[i - unused_count] = list->items[i];
        }
        else
        {
            unused_count += 1;
        }
    }
    list->count -= unused_count;
}

void optimize_tac_list(TACList* list)
{
    remove_phi_expressions(list);
    // eliminate_dead_tac(list);
}