#include <stdio.h>
#include <stdlib.h>

#include "avl_tree.h"

/*
#define LOG(...) \
   printf(__VA_ARGS__); \
   printf("\n");
*/
#ifndef MAX
# define MAX(a, b) ( ( (a) >= (b) ) ? (a) : (b) )
#endif

struct avl_node_t {
    avl_node *left, *right, *parent;
    unsigned char left_height, right_height;
    int key;
    void *value;
};

avl_node *avl_node_new(int key, void *value)
{
    avl_node *n = malloc(sizeof(*n));
    n->left = n->right = n->parent = NULL;
    n->left_height = n->right_height = 0;
    n->key = key;
    n->value = value;
    return n;
}

void avl_node_destroy(avl_node *n)
{
    if(n->left)
        avl_node_destroy(n->left);
    if(n->right)
        avl_node_destroy(n->right);

    free(n);
}

avl_node *avl_search(avl_node *node, int key)
{
    if(node->key == key)
        return node;
    else if(key > node->key)
        return avl_search(node->right, key);
    else if(key < node->key)
        return avl_search(node->left, key);
    else
        return NULL;
}

static avl_node *rot_left(avl_node *n)
{
//    LOG("rotating left");

    n->parent->right_height = n->left_height;
    n->left_height = MAX(n->parent->right_height, n->parent->left_height) + 1;

    avl_node *p = n->parent;
    p->right = n->left;
    n->left = p;
    n->parent = p->parent;
    p->parent = n;
    if(p->right)
        p->right->parent = p;
    return n;
}

static avl_node *rot_right(avl_node *n)
{
//    LOG("rotating right");

    n->parent->left_height = n->right_height;
    n->right_height = MAX(n->parent->right_height, n->parent->left_height) + 1;

    avl_node *p = n->parent;
    p->left = n->right;
    n->right = p;
    n->parent = p->parent;
    p->parent = n;
    if(p->left)
        p->left->parent = p;
    return n;
}

static void addLevels(avl_node *p, avl_node *n)
{
    /* move up through tree and add to heights */
    if(p) {
        if(p->right == n)
            p->right_height++;
        else
            p->left_height++;

        addLevels(p->parent, p);
    }
}

static void remLevels(avl_node *p, avl_node *n)
{
    /* move up through tree and remove from heights */
    if(p) {
        if(p->right == n)
            p->right_height--;
        else
            p->left_height--;

        remLevels(p->parent, p);
    }
}


static avl_node *rebalance(avl_node *c, char done)
{
    /* zoom up to the root */
    if(done) {
        if(c->parent)
            return rebalance(c->parent, 1);
        else
            return c;
    }

    avl_node *b = c->parent;
    if(!b)
        return c;
    avl_node *a = b->parent;
    if(!a)
        return b;
    avl_node *f = a->parent;

    int balance = a->left_height - a->right_height;
//    LOG("balance factor = %d, left = %d, right = %d", balance, a->left_height, a->right_height);

    if(balance > 1 || balance < -1) {
//        LOG("rebalancing node %x", (int)a);

        avl_node *next;

        /* four cases */
        if(a->right == b && b->right == c) {
            rot_left(b);
            next = b;
        } else if(a->right == b && b->left == c) {
            avl_node *new = rot_right(c);
            rot_left(new);
            next = c;
        } else if(a->left == b && b->left == c) {
            rot_right(b);
            next = b;
        } else if(a->left == b && b->right == c) {
            avl_node *new = rot_left(c);
            rot_right(new);
            next = c;
        }

        if(! f)
            return next;
        else {
            /* fix up f's child pointer */
            if(f->right == a)
                f->right = next;
            else
                f->left = next;

            /* fix up levels above */
            remLevels(f, next);

            return rebalance(next, 1);
        }
    }

    return rebalance(b, 0);
}

static avl_node *insert_node(avl_node *root, avl_node *n)
{
    char extra_level = 1;

//    LOG("entering insert");

    if(n->key >= root->key) {
        if(root->right) {
//            LOG("following right subtree");
            return insert_node(root->right, n);
        } else {
            if(root->left)
                extra_level = 0;

//            LOG("inserting into right subtree");

            root->right = n;
            n->parent = root;
        }
    } else {
        if(root->left) {
//            LOG("following left subtree");
            return insert_node(root->left, n);
        } else {
            if(root->right)
                extra_level = 0;

//            LOG("inserting into left subtree");

            root->left = n;
            n->parent = root;
        }
    }

    if(extra_level)
        addLevels(n->parent, n);

    return rebalance(n, 0);
}

avl_node *avl_insert(avl_node *root, int key, void *value)
{
    avl_node *n = avl_node_new(key, value);

    /* this is the first element in the tree */
    if(!root)
        return n;

    return insert_node(root, n);
}

static avl_node *find_min(avl_node *n)
{
    if(n && n->left)
        return find_min(n->left);
    else
        return n;
}

static avl_node *find_max(avl_node *n)
{
    if(n && n->right)
        return find_max(n->right);
    else
        return n;
}

static void set_side_null(avl_node *p, avl_node *c)
{
    /* looks at both left and right children of p, whichever side is c
     * is set to NULL  */

    if(p->right == c)
        p->right = NULL;
    else
        p->left = NULL;
}

static void set_side_to_grandchild(avl_node *p, avl_node *c)
{
    /* looks at both left and right children of p, whichever side is c
     * is set to c's grandchild (implies c only has one subtree)  */

    if(p->right == c)
        p->right = c->right ? c->right : c->left;
    else
        p->left = c->right ? c->right : c->left;
}


static avl_node *delete_right(avl_node *d)
{
    /* replace with min of right subtree */
    avl_node *r = find_min(d->right);
    
    avl_node *to_reblance_from = r->parent;
    
    d->key = r->key;
    d->value = r->value;
    
    /* delete node */
    set_side_to_grandchild(to_reblance_from, r);

    return rebalance(to_reblance_from, 0);
}


static avl_node *delete_left(avl_node *d)
{
    /* replace with max of left subtree */
    avl_node *r = find_max(d->left);
    
    avl_node *to_reblance_from = r->parent;
    
    d->key = r->key;
    d->value = r->value;
    
    /* delete node */
    set_side_to_grandchild(to_reblance_from, r);

    return rebalance(to_reblance_from, 0);
}


avl_node *avl_delete(avl_node *root, int key)
{
    /* find node */
    avl_node *to_delete = avl_search(root, key);

    /* simple case if end node */
    if(to_delete->right_height == 0 && to_delete->left_height == 0) {
        if(! to_delete->parent)
            return NULL;
        else {
            set_side_null(to_delete->parent, to_delete);
            return rebalance(to_delete->parent, 0);
        }
    } else if(to_delete->right_height > 0 && to_delete->left_height > 0) {
        /* pick largest subtree */
        if(to_delete->right_height > to_delete->left_height)
            return delete_right(to_delete);
        else
            return delete_left(to_delete);
    } else {
        /* otherwise just pick the valid side */
        if(to_delete->right_height > 0)
            return delete_right(to_delete);
        else
            return delete_left(to_delete);
    }
}

void *avl_value(avl_node *n)
{
    return n->value;
}


static int next_pow_2(int v)
{
    int test = 1;

    while(test < v)
        test *= 2;

    return test;
}

static avl_node *print_list[512];
static int list_idx = 0;
static int list_size = 0;
static int counter = 0;
static int counter_end = 1;
static void avl_dbg_print_tree()
{
    if(list_idx == list_size)
        return;

    avl_node *n = print_list[list_idx++];
    if(n)
        printf("%d:%d:%d\t", n->key, n->left_height, n->right_height);
    else
        printf("NIL ");

    counter++;

    //LOG("counter = %d, idx = %d, next = %d",counter, list_idx, next_pow_2(list_idx));

    if(counter == counter_end) {
        printf("\n");
        counter_end = next_pow_2(counter+1);
        counter = 0;
    }

    if(n) {
        print_list[list_size++] = n->left;
        print_list[list_size++] = n->right;
    }

    avl_dbg_print_tree();

    //int i;
    //for(i=0; i<depth*3; i++)
    //    printf(" ");


}

int main()
{
    avl_node *tree = avl_insert(NULL, 5, NULL);
    tree = avl_insert(tree, 2, NULL);
    tree = avl_insert(tree, 3, NULL);
    tree = avl_insert(tree, 7, NULL);
    tree = avl_insert(tree, 6, NULL);
    tree = avl_insert(tree, 8, NULL);
    tree = avl_insert(tree, 9, NULL);
    tree = avl_insert(tree, 1, NULL);

    tree = avl_delete(tree, 5);


    print_list[0] = tree;
    list_idx = 0;
    list_size = 1;
    avl_dbg_print_tree();

    return 0;
}
