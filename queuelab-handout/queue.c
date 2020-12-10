/*
 * Developed by R. E. Bryant, 2017
 * Extended to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    //if its NULL cant really add a new spot so linked
    ///leave the method??

    if(q == NULL)
    {
      //printf("Allocation returned NULL\n"); ///to test
      return NULL;
    }

    q->head = NULL;
    q->tail = NULL;
    q ->size = 0;

    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
  //like always what if its already empty or NULL
  if(q == NULL)
  {
    return;
  }
  // i guess brute force version is to go through each element
  //with a loop
  //so linked list is , while(head.next is NULL) delete head.next;

    list_ele_t *head = q->head;
    list_ele_t *temp = q->head;
    while (temp != NULL) {
        free(temp->value);
        head = temp;
        temp = temp->next;
        free(head);
    }
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh;
    /* What should you do if the q is NULL? */
    if(q == NULL || s == NULL) //always handle NULL exception first
    {
      return false;
    }
    //find space for the new head
    newh = malloc(sizeof(list_ele_t));
    if(newh == NULL)
    {
      //printf("No space for the head!\n"); //thats what she said
      return false;
    }

    newh->next = q->head; //the next of the new head will be the old head
    q->head = newh; // the head of the queue is now newh

    //check for the tail being NULL
    if (q->tail == NULL)
    {
      q->tail = newh;
    }

    //we created a new node but that shit empty
    newh->value = malloc(strlen(s) + 1);
    //but might not be able to find space.

    if (newh->value == NULL)
    {
      return false;
    }
    memcpy(newh->value, s, strlen(s));
    newh->value[strlen(s)] = '\0';

    q->size++;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
  //relly really similar to head
  list_ele_t *newt;
  /* What should you do if the q is NULL? */
  if(q == NULL) //always handle NULL exception first
  {
    return false;
  }
  /* Don't forget to allocate space for the string and copy it */
  /* What if either call to malloc returns NULL? */
  newt = malloc(sizeof(list_ele_t));
  //check for NUll
    if (newt == NULL)
    {
      return false;
    }
    //tail can be null too
    if (q->tail != NULL)
    {
        q->tail->next = newt;
    }
    q->tail = newt;

    if (q->head == NULL)
    {
        q->head = newt;
    }

    newt->next = NULL; //so we have to make the tail the last node
    //node exist but no value
    newt->value = malloc(strlen(s) + 1);
    //null check
    if (newt->value == NULL)
    {
      return false;
    }

    memcpy(newt->value, s, strlen(s));
    newt->value[strlen(s)] = '\0';

    q->size++;

    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a NULL terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    list_ele_t *node;

   if (q == NULL || q->size == 0)
   {
       return false;
  }
   node = q->head; //point the new node to head
   q->head = q->head->next; //point the head to the one after it essentially
   //skipping it
   if (q->head == NULL)
   {
     q->tail = NULL;
   }

   if (sp != NULL) {
       size_t value_str_len = strlen(node->value);
       if (value_str_len >= bufsize) {
           strncpy(sp, node->value, bufsize - 1);
           sp[bufsize - 1] = '\0';
       } else {
           strncpy(sp, node->value, value_str_len);
           sp[value_str_len] = '\0';
       }
   }
   free(node->value); //free val first
   free(node); // then free node
   q->size--;

   return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
  if(q == NULL)
  {
    return 0;
  }else
  {
      return q->size;
  }
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
  if(q == NULL || q -> head == NULL)
  {
    return;
  }
  //make temp variables
  list_ele_t *temp;
  list_ele_t *prev = NULL;
  list_ele_t *curr = NULL;

  //lets think of this in super java way
  curr = q->head;   //so cur is the head of the q
  q -> tail = curr; // and the tail is pointing to cur(ie the head)

  while(curr != NULL) // so until its not null
  {
    temp = curr->next; //set a temp var to the next one
    curr->next = prev; // and set THAT next one to the last one
    prev = curr; //but that last one is curr
    curr = temp; // and the new curr is the saved temp
  }

  q ->head = prev;//lastly the curr is going to be null so
  //set the head to the prev
}
