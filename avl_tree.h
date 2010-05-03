#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

typedef struct avl_node_t avl_node;

avl_node *avl_node_new(int key, void *value);
void avl_node_destroy(avl_node *n);
avl_node *avl_search(avl_node *node, int key);
avl_node *avl_insert(avl_node *root, int key, void *value);
avl_node *avl_delete(avl_node *root, int key);
void *avl_value(avl_node *n);

#endif  // __AVL_TREE_H__
