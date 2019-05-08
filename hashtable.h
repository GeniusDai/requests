//
//  hashtable.h
//  requests
//
//  Created by hao on 2019/4/29.
//  Copyright © 2019 hao. All rights reserved.
//

#ifndef hashtable_h
#define hashtable_h

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 29

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef char *keytype;
typedef char *valuetype;

typedef struct node **entry;

struct node {
    struct node *next;
    keytype key;
    valuetype value;
};

struct node **init_table(void);

void free_table(struct node **a);

void print_table (struct node **a);

int hash_func(keytype key);

void set(entry table, keytype key, valuetype value);

struct node *pop(entry table, keytype key);

struct node *get(entry table, keytype key);
#endif /* hashtable_h */
