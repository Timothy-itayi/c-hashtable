#include <stdlib.h>
#include <string.h>
#include <math.h>  // For pow()

#include "hash_table.h"

// Define constants
#define HT_PRIME_1 151
#define HT_PRIME_2 163
#define HT_INITIAL_BASE_SIZE 50

// Define the deleted item
static ht_item HT_DELETED_ITEM = {NULL, NULL};

// Safe memory allocation functions
static void* xmalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static void* xcalloc(size_t num, size_t size) {
    void* ptr = calloc(num, size);
    if (!ptr) {
        perror("calloc failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Create a new item (key-value pair)
static ht_item* ht_new_item(const char* k, const char* v) {
    ht_item* i = xmalloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

// Resize hash table to a larger size
static void ht_resize_up(ht_hash_table* ht) {
    const int new_size = next_prime(ht->base_size * 2);  // Ensure prime size
    ht_resize(ht, new_size);
}

// Resize hash table to a smaller size
static void ht_resize_down(ht_hash_table* ht) {
    const int new_size = next_prime(ht->base_size / 2);  // Ensure prime size
    ht_resize(ht, new_size);
}

// Resize the hash table with a new base size
static void ht_resize(ht_hash_table* ht, const int base_size) {
    if (base_size < HT_INITIAL_BASE_SIZE) {
        return;
    }

    ht_hash_table* new_ht = ht_new_sized(base_size);
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    // Swap items between old and new table
    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    ht_item** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    ht_free(new_ht);
}

// Create a new hash table with the given base size
static ht_hash_table* ht_new_sized(const int base_size) {
    ht_hash_table* ht = xmalloc(sizeof(ht_hash_table));
    ht->base_size = base_size;
    ht->size = next_prime(ht->base_size);
    ht->count = 0;
    ht->items = xcalloc((size_t)ht->size, sizeof(ht_item*));
    return ht;
}

// Create a new hash table with the default size
ht_hash_table* ht_new() {
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

// Delete an item (key-value pair) from memory
static void ht_del_item(ht_item* i) {
    free(i->key);
    free(i->value);
    free(i);
}

// Free memory for the entire hash table
void ht_del_hash_table(ht_hash_table* ht) {
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL) {
            ht_del_item(item);
        }
    }
    free(ht->items);
    free(ht);
}

// Hash function to calculate the hash value for a string
static int ht_hash(const char* s, const int a, const int m) {
    long hash = 0;
    const int len_s = strlen(s);
    for (int i = 0; i < len_s; i++) {
        hash += (long)pow(a, len_s - (i+1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

// Get hash value for a string with a given number of buckets and attempt
static int ht_get_hash(const char* s, const int num_buckets, const int attempt) {
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

// Insert a new key-value pair into the hash table
void ht_insert(ht_hash_table* ht, const char* key, const char* value) {
    ht_item* item = ht_new_item(key, value);
    int index = ht_get_hash(item->key, ht->size, 0);
    ht_item* cur_item = ht->items[index];
    int i = 1;

    while (cur_item != NULL) {
        if (cur_item != &HT_DELETED_ITEM) {
            if (strcmp(cur_item->key, key) == 0) {
                ht_del_item(cur_item);
                ht->items[index] = item;
                return;
            }
        }
        index = ht_get_hash(item->key, ht->size, i);
        cur_item = ht->items[index];
        i++;
    }

    ht->items[index] = item;
    ht->count++;
}

// Search for a value by its key in the hash table
char* ht_search(ht_hash_table* ht, const char* key) {
    int index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;

    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                return item->value;
            }
        }

        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }

    return NULL;
}

// Delete a key-value pair from the hash table
void ht_delete(ht_hash_table* ht, const char* key) {
    int index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;

    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                ht_del_item(item);
                ht->items[index] = &HT_DELETED_ITEM;
                ht->count--;
                return;
            }
        }

        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }
}
