#ifndef STORAGE
#define STORAGE
struct node {
    char* alias;
    char* replacement;
    char** tkcmd;
   struct node *next;
};
struct node *head;
struct node *current;

void printList();
void insert(char* alias, char* replacement, char** tkcmd);
struct node* find(char* alias);
struct node* discard(char* alias);

#endif

