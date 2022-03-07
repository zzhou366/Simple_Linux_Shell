#include "storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node *head = NULL;
struct node *current = NULL;

//display the list
void printList() {
   struct node *ptr = head;
   //start from the beginning
   while(ptr != NULL) {
      printf("%s %s\n",ptr->alias,ptr->replacement);
      fflush(stdout);
      // int length = 0;
      // // printf("1\n");
      // while(*(ptr->tkcmd + length) != NULL)
      // {
      //    printf("%s ", *(ptr->tkcmd + length));
      //    length++;
      // }
      ptr = ptr->next;
      // printf("\n");
      // fflush(stdout);
   }
}

//insert link at the first location
void insert(char* alias, char* replacement, char** tkcmd) {
   //create a link
   struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->alias = alias;
   link->replacement = replacement;
   link->tkcmd = tkcmd;
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}



//find a link with given key
struct node* find(char* alias) {

   //start from the first link
   struct node* current = head;

   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(strcmp(current->alias, alias) != 0) {
	//   printf("inside finde: %s %s\n", current->alias, current->replacement);
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
//    printf("before return--------find\n");
   return current;
}

//delete a link with given key
struct node* discard(char* alias) {

   //start from the first link
   struct node* current = head;
   struct node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(strcmp(current->alias, alias) != 0) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}

