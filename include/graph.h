#ifndef __GRAPH_H_
#define __GRAPH_H_

#include <stdint.h>
#include <raylib.h>

typedef struct NodeList node_list_t;
typedef struct Node node_t;

struct NodeList {
    node_t *node;
    node_list_t *next;
};

struct Node {
    node_list_t *connecting_nodes;
    bool is_deleting;
    Color color;
    char *text;
    int x, y, w, h;
};

node_t *create_node(void);
void destroy_node(node_t *n);
void destroy_node_rec(node_t *n);
void link_nodes(node_t *n1, node_t *n2);
void unlink_nodes(node_t *n1, node_t *n2);
void write_node_text(node_t *n, char *text);

#endif
