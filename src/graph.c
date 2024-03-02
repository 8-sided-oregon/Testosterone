#include "graph.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

node_t *create_node(void) {
    node_t *n = xmalloc(sizeof(node_t));
    memset(n, 0, sizeof(node_t));
    return n;
}

void destroy_node(node_t *n) {
    n->is_deleting = true;
    node_list_t *p = NULL;
    for (node_list_t *i = n->connecting_nodes; i != NULL; i = i->next) {
        if (p != NULL)
            free(p);
        p = i;
    }
    free(p);
    if (n->text != NULL)
        free(n->text);
    free(n);
}

void destroy_node_rec(node_t *n) {
    if (n->is_deleting)
        return;
    n->is_deleting = true;
    for (node_list_t *i = n->connecting_nodes; i != NULL; i = i->next)
        destroy_node_rec(i->node);
    destroy_node(n);
}

void link_nodes(node_t *n1, node_t *n2) {
    if (n1->connecting_nodes == NULL) {
        n1->connecting_nodes = xmalloc(sizeof(node_list_t));
        n1->connecting_nodes->next = NULL;
        n1->connecting_nodes->node = n2;
    }
    else {
        node_list_t *i = n1->connecting_nodes;
        for (; i->next != NULL; i = i->next) {}
        i->next = xmalloc(sizeof(node_list_t));
        i->next->node = n2;
        i->next->next = NULL;
    }
}

void unlink_nodes(node_t *n1, node_t *n2) {
    node_list_t *next;
    if (n1->connecting_nodes == NULL)
        return;
    if (n1->connecting_nodes->node == n2) {
        next = n1->connecting_nodes->next;
        free(n1->connecting_nodes);
        n1->connecting_nodes = next;
    }
    if (n1->connecting_nodes == NULL)
        return;
    node_list_t *prev = n1->connecting_nodes;
    for (node_list_t *cur = prev->next; cur != NULL; cur = next) {
        next = cur->next;
        if (cur->node == n2) {
            free(cur);
            prev->next = next;
        }
        else {
            prev = cur;
        }
    }
}

void write_node_text(node_t *n, char *text) {
    size_t len = strlen(text);
    n->text = xmalloc(len);
    memcpy(n->text, text, len);
}
