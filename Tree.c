#include <errno.h>

#include "Tree.h"
#include "path_utils.h"
#include "HashMap.h"

Tree* tree_new(){
    Tree* tree = hmap_new();
    
    tree_create(tree, "/");

    return tree;
}

void tree_free(Tree* tree){

    
}

char* tree_list(Tree* tree, const char* path){
    char result[MAX_FOLDER_NAME_LENGTH + 1];
    HashMapIterator it = hmap_iterator(tree);
    const char* key;
    void* value;
    while (hmap_next(tree, &it, &key, &value))
        strcat(key, result);

    return result;
}

int tree_create(Tree* tree, const char* path){
    if(!is_path_valid(path)){
        return EINVAL;
    }

    char first[MAX_FOLDER_NAME_LENGTH + 1];
    char tail = split_path(path, first);

    while(tail){
        tree = hmap_get(tree, first);
        path = tail;
        tail = split_path(path, first);
    }

    Tree* new = hmap_new();

    hmap_insert(tree, first, new);

    return 0;
}

int tree_remove(Tree* tree, const char* path){
    if(!is_path_valid(path)){
        return EINVAL;
    }

    char first[MAX_FOLDER_NAME_LENGTH + 1];
    char tail = split_path(path, first);

    while(tail){
        tree = hmap_get(tree, first);
        path = tail;
        tail = split_path(path, first);
    }

    hmap_remove(tree, first);

    return 0;
}

int tree_move(Tree* tree, const char* source, const char* target){

    return 1;
}