#include "linked_list.h"

// Creates and returns a new list
list_t* list_create()
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_t* myList = (list_t*)malloc(sizeof(list_t));
    myList->head = NULL;
    myList->count = 0;

    return myList;
    //return NULL;
}

// Destroys a list
void list_destroy(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    free(list);
}

// Returns beginning of the list
list_node_t* list_begin(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return list->head;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return node->next;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return node->data;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return list->count;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t *myList = list->head;
    while(myList->next){
        if(myList->data == data)
            return myList;   
        myList = myList->next;
    }
    return NULL;
}

// Inserts a new node in the list with the given data
void list_insert(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t *myNode = (list_node_t*)malloc(sizeof(list_node_t));
    
    if(list->count == 0){
        myNode->next= NULL;
        myNode->prev = NULL;
        list->head->data = data;
        list->count++;
    }
    else{
        //list_node_t *temp = (list_node_t*)malloc(sizeof(list_node_t));
        //myNode = list->head;
        myNode->next = list->head;
        list->head->prev = myNode;
        myNode->prev = NULL;
        list->head = myNode;
        // list->head->next = myNode;
        // list->head = myNode;
        list->count++;
    }
    //list->count++;
}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t *temp = list->head;
    while(temp){
        if (temp == node){
            temp->next = node->next;
            //temp->prev = 

            free(node);
        }
    }
    //return 1;
}

// Executes a function for each element in the list
void list_foreach(list_t* list, void (*func)(void* data))
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}
