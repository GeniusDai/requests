//
//  hashtable.c
//  requests
//
//  Created by hao on 2019/4/29.
//  Copyright © 2019 hao. All rights reserved.
//

#ifndef hashtable_h
#include "hashtable.h"
#endif

struct node **init_table(void) {
    struct node **a = malloc(TABLE_SIZE * sizeof(void *));
    bzero(a, TABLE_SIZE * sizeof(void *));
    return a;
}

void free_table(struct node **a) {
    int i;
    struct node *pre, *head;
    for (i = 0; i < TABLE_SIZE; i++) {
        head = a[i];
        while (head != NULL) {
            pre = head;
            head = head->next;
            if (pre != a[i]) {
                free(pre);
            }
        }
    }
    free(a);
    return;
}

void print_table (struct node **a) {
    printf("HASH TABLE:\n");
    int i;
    struct node *head;
    for (i = 0; i < TABLE_SIZE; i++) {
        head = a[i];
        while (head != NULL) {
            printf("%s: %s\n", head->key, head->value);
            head = head->next;
        }
    }
    printf("\n");
    return;
}

int hash_func(keytype key) {
    return (5 * strlen(key) + key[0]) % TABLE_SIZE;
}

void set(entry table, keytype key, valuetype value) {
    int hash_code = hash_func(key);
    struct node *head = table[hash_code];
    struct node *pre = NULL;
    
    while (head != NULL && head->key != key) {
        pre = head;
        head = head->next;
    }
    
    if (head == NULL) {
        head = malloc(sizeof(struct node));
        bzero(head, sizeof(struct node));
        head->key = key;
        head->value = value;
        head->next = NULL;
        if (table[hash_code] == NULL) {
            table[hash_code] = head;
        } else {
            if (pre == NULL) {
                printf("hash table error\n");
                exit(-1);
            }
            pre->next = head;
        }
    } else if (head->key == key){
        head->value = value;
    }
    return;
}

struct node *pop(entry table, keytype key) {
    int hash_code = hash_func(key);
    struct node *head = table[hash_code];
    struct node *pre = NULL;
    
    while (head != NULL && head->key != key) {
        pre = head;
        head = head->next;
    }
    if (head == NULL) {
        return NULL;
    } else if (pre == NULL){
        table[hash_code] = table[hash_code]->next;
        return head;
    } else {
        pre->next = pre->next->next;
        return head;
    }
}

struct node *get(entry table, keytype key) {
    int hash_code = hash_func(key);
    struct node *head = table[hash_code];
    
    while (head != NULL && head->key != key) {
        head = head->next;
    }
    return head;
}
