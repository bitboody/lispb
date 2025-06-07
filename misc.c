#include "mpc.h"

int number_of_leaves(mpc_ast_t *t) {
    // Leaf node: no children
    if (t->children_num == 0) {
        // Filter out parentheses and similar syntax tokens
        if (strcmp(t->tag, "number|regex") == 0 || 
            strcmp(t->tag, "char") == 0 || 
            strstr(t->tag, "operator") != NULL) {
            return 1;
        } else {
            return 0;
        }
    }

    // Internal node: recurse
    int total = 0;
    for (int i = 0; i < t->children_num; i++) {
        total += number_of_leaves(t->children[i]);
    }
    return total;
}

int number_of_branches(mpc_ast_t *t)
{
    if (t->children_num == 0)
    {
        return 1;
    }
    if (t->children_num >= 1)
    {
        int total = 1;
        for (int i = 0; i < t->children_num; i++)
        {
            if (t->children[i]->children_num)
            {
                total = total + number_of_branches(t->children[i]);
            }
        }
        return total;
    }
    return 0;
}
