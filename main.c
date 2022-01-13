#include "HashMap.h"
#include "Tree.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void print_map(HashMap* map) {
    const char* key = NULL;
    void* value = NULL;
    printf("Size=%zd\n", hmap_size(map));
    HashMapIterator it = hmap_iterator(map);
    while (hmap_next(map, &it, &key, &value)) {
        printf("Key=%s Value=%p\n", key, value);
    }
    printf("\n");
}


int main(void)
{
    HashMap* map = hmap_new();
    hmap_insert(map, "a", hmap_new());
    print_map(map);

    HashMap* child = (HashMap*)hmap_get(map, "a");
    hmap_free(child);
    hmap_remove(map, "a");
    print_map(map);

    hmap_free(map);

    printf("\n\n================\nZaczyna sie moja czesc\n");

    Tree* tree = tree_new();
    //printf("Jestem po tree_new\n");
    if(!tree){
        printf("error1\n");
    }

    char* list = tree_list(tree, "/");
    //printf("Jestem po tree_list\n");
    if(!list){
        printf("error2\n");
    }
    printf("'''%s'\n", list);

    int res = tree_create(tree, "/a/");
    //printf("Jestem po tree_create\n");
    if(res){
        printf("error3\n");
    }

    list = tree_list(tree, "/");
    //printf("Jestem po tree_list\n");
    if(!list){
        printf("error4\n");
    }
    printf("'a''%s'\n", list);

    res = tree_create(tree, "/b/");
    //printf("Jestem po tree_create\n");
    if(res){
        printf("error8\n");
    }

    list = tree_list(tree, "/");
    //printf("Jestem po tree_list\n");
    if(!list){
        printf("error9\n");
    }
    printf("'a,b''%s'\n", list);

    res = tree_create(tree, "/b/w/");
    //printf("Jestem po tree_create\n");
    if(res){
        printf("error88\n");
    }

    list = tree_list(tree, "/b/");
    //printf("Jestem po tree_list\n");
    if(!list){
        printf("error99\n");
    }
    printf("'w''%s'\n", list);

    res = tree_move(tree, "/b/w/", "/a/w/");
    //printf("Jestem po tree_create\n");
    if(res){
        printf("error888\n");
    }

    list = tree_list(tree, "/b/");
    //printf("Jestem po tree_list\n");
    if(!list){
        printf("error999\n");
    }
    printf("'''%s'\n", list);

    list = tree_list(tree, "/a/");
    //printf("Jestem po tree_list\n");
    if(!list){
        printf("error999\n");
    }
    printf("'w''%s'\n", list);

    res = tree_remove(tree, "/a/");
    //printf("Jestem po tree_remove\n");
    if(res){
        printf("error5\n");
    }

    return 0;
}