#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Tree.h"
#include "path_utils.h"
#include "HashMap.h"

#define EMPTY_TREE -1 //given tree pointer is null
#define ALLOCATE_ERROR -2  //unable to allocate necessary memory

struct Tree{
    HashMap* subfolders;
    pthread_mutex_t mutex;
};

Tree* tree_new(){
    Tree* tree = malloc(sizeof(Tree));
    if(!tree){
        return NULL; //unable to allocate necessary memory
    }
    memset(tree, 0, sizeof(Tree));

    tree->subfolders = hmap_new();
    if(pthread_mutex_init(tree->mutex, NULL)){
        return NULL;
    }

    return tree;
}

void tree_free(Tree* tree){
    if(!tree){
        return;
    }

    HashMapIterator it = hmap_iterator(tree->subfolders);

    const char* key;
    void* value;
    while(hmap_next(tree->subfolders, &it, &key, &value)){
        tree_free(value);
    }

    hmap_free(tree->subfolders);
    pthread_mutex_destroy(tree->mutex);
    free(tree);
    tree = NULL; 
}

char* tree_list(Tree* tree, const char* path){
    if(!tree ){
        return EMPTY_TREE;
    }
    if(!is_path_valid(path)){
        return NULL;
    }

    char component[MAX_FOLDER_NAME_LENGTH + 1];
    const char* subpath = path;
    while((subpath = split_path(subpath, component))){
        pthread_mutex_t mutex = tree->mutex;
        pthread_mutex_lock(mutex);
        tree = hmap_get(tree->subfolders, component);
        pthread_mutex_unlock(mutex);
        if(!tree){
            return NULL;
        }
    }
    if(!tree->subfolders){
        return NULL;
    }

    pthread_mutex_lock(tree->mutex);
    char* result = make_map_contents_string(tree->subfolders);
    pthread_mutex_unlock(tree->mutex);

    return result;
}

int tree_create(Tree* tree, const char* path){
    if(!tree){
        return EMPTY_TREE;
    }
    if(!is_path_valid(path)){
        return EINVAL;
    }

    char* name = malloc(MAX_FOLDER_NAME_LENGTH);
    if(!name){
        return ALLOCATE_ERROR;
    }

    path = make_path_to_parent(path, name);
    if(!path){
        return EEXIST;
    }

    char component[MAX_FOLDER_NAME_LENGTH + 1];
    const char* subpath = path;
    while((subpath = split_path(subpath, component))){
        pthread_mutex_t mutex = tree->mutex;
        pthread_mutex_lock(mutex);
        tree = hmap_get(tree->subfolders, component);
        pthread_mutex_unlock(mutex);
        if(!tree){
            return ENOENT;
        }
    }  

    Tree* new = malloc(sizeof(Tree));
    if(!new){
        return ALLOCATE_ERROR;
    }
    memset(new, 0, sizeof(Tree));

    new->subfolders = hmap_new();
    pthread_mutex_init(new->mutex, NULL);

    pthread_mutex_t mutex = tree->mutex;
    pthread_mutex_lock(mutex);
    if(hmap_get(tree->subfolders, name)){
        pthread_mutex_unlock(mutex);
        return EEXIST;
    }
    hmap_insert(tree->subfolders, name, new);
    pthread_mutex_unlock(mutex);

    return 0;
}

int tree_remove(Tree* tree, const char* path){
    if(!tree){
        return EMPTY_TREE;
    }
    if(!is_path_valid(path)){
        return EINVAL;
    }

    char* name = malloc(MAX_FOLDER_NAME_LENGTH);
    if(!name){
        return ALLOCATE_ERROR;
    }

    path = make_path_to_parent(path, name);
    if(!path){
        return EBUSY;
    }

    char component[MAX_FOLDER_NAME_LENGTH + 1];
    const char* subpath = path;
    while((subpath = split_path(subpath, component))){
        pthread_mutex_t mutex = tree->mutex;
        pthread_mutex_lock(mutex);
        tree = hmap_get(tree->subfolders, component);
        pthread_mutex_unlock(mutex);
        if(!tree){
            return ENOENT;
        }
    }

    pthread_mutex_t mutex = tree->mutex;
    pthread_mutex_lock(mutex);
    Tree* toDelete = hmap_get(tree->subfolders, name);
    pthread_mutex_unlock(mutex);
    if(!toDelete){
        return ENOENT;
    }

    pthread_mutex_t mutex = toDelete->mutex;
    pthread_mutex_lock(mutex);
    if(hmap_size(toDelete->subfolders) > 0){
        pthread_mutex_unlock(mutex);
        return ENOTEMPTY;
    }
    pthread_mutex_unlock(mutex);

    tree_free(toDelete);
    pthread_mutex_t mutex = tree->mutex;
    pthread_mutex_lock(mutex);
    hmap_remove(tree->subfolders, name);
    pthread_mutex_unlock(mutex);

    return 0;
}

int tree_move(Tree* tree, const char* source, const char* target){
    if(!tree){
        return EMPTY_TREE;
    }
    if(!is_path_valid(source) || !is_path_valid(target)){
        return EINVAL;
    }

    Tree* tree2 = tree;

    char* name1 = malloc(MAX_FOLDER_NAME_LENGTH);
    if(!name1){
        return ALLOCATE_ERROR;
    }

    source = make_path_to_parent(source, name1);
    if(!source){
        return EBUSY;
    }
    char component[MAX_FOLDER_NAME_LENGTH + 1];
    const char* subpath = source;
    while((subpath = split_path(subpath, component))){
        pthread_mutex_t mutex = tree->mutex;
        pthread_mutex_lock(mutex);
        tree = hmap_get(tree->subfolders, component);
        pthread_mutex_unlock(mutex);
        if(!tree){
            return ENOENT;
        }
    }

    pthread_mutex_t mutex = tree->mutex;
    pthread_mutex_lock(mutex);
    Tree* tree1 = hmap_get(tree->subfolders, name1);
    if(!tree1){
        pthread_mutex_unlock(mutex);
        return ENOENT;
    }
    hmap_remove(tree->subfolders, name1);
    pthread_mutex_unlock(mutex);

    char* name2 = malloc(MAX_FOLDER_NAME_LENGTH);
    if(!name2){
        return ALLOCATE_ERROR;
    }

    target = make_path_to_parent(target, name2);
    if(target){
        char component[MAX_FOLDER_NAME_LENGTH + 1];
        const char* subpath = target;
        while((subpath = split_path(subpath, component))){
            pthread_mutex_t mutex = tree2->mutex;
            pthread_mutex_lock(mutex);
            tree2 = hmap_get(tree2->subfolders, component);
            pthread_mutex_unlock(mutex);
            if(!tree2){
                return ENOENT;
            }
        }
    }

    pthread_mutex_t mutex = tree2->mutex;
    pthread_mutex_lock(mutex);
    if(hmap_get(tree2->subfolders, name2)){
        pthread_mutex_unlock(mutex);
        return EEXIST;
    }

    hmap_insert(tree2->subfolders, name2, tree1);
    pthread_mutex_unlock(mutex);

    return 0;
}