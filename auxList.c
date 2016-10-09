#include "header.h"

struct node
{
	int start; /* Consider this the node start. Corresponds with MSS chunk in cBuffer 0-64000 */
			
	int nextB; /* Next chunk index, optional */
	int pack;  /* Packet No. */
	int bytes; /* Num of bytes read in from client */
	int seq;   /* Sequence number */
	int ack;   /* acknowledgement flag (0 = no, 1 = yes) */
	float time;/* Time */
	
	struct node *next;
};

/* Inserts a new node to the linked list that holds information about a MSS chunck of cBuffer */
/* Start should correspond with the start index of the array element */
void insertNode(struct node *ptr, int start, int nextB, int pack, int bytes, int seq, float time)
{
        while(ptr->next!=NULL)
        {
                ptr = ptr -> next;
        }
        /* Allocate memory for the new node and put start in it.*/
        ptr->next = (struct node *)malloc(sizeof(struct node));
        ptr = ptr->next;

	/* Fill contents */
	ptr->start = start;
	ptr->nextB = nextB;
	ptr->pack = pack;
	ptr->bytes = bytes;
	ptr->seq = seq;
	ptr->time = time;
	
	/* point to end */
        ptr->next = NULL;
}

/* Deletes node with a specific start index */
void deleteNode(struct node *ptr, int start)
{
        while(ptr->next!=NULL && (ptr->next)->start != start)
        {
                ptr = ptr -> next;
        }
        if(ptr->next==NULL)
        {
                printf("Element %d is not present in the list\n",start);
                return;
        }

        struct node *temp;
        temp = ptr -> next;
        
        ptr->next = temp->next;
       
        free(temp);
       
        return;
}

/* Finds and returns node with specific start value */
/* Does not remove node */
struct node *findNode(struct node *ptr, int start)
{
        ptr =  ptr -> next;

        while(ptr!=NULL)
        {
                if(ptr->start == start)
                {
                        return ptr;
                }
                ptr = ptr -> next;
        }
        return NULL;
}

/* Prints list start indexes for debugging purposes */
void printList(struct node *ptr)
{
        if(ptr==NULL)
        {
                return;
        }
        printf("%d ",ptr->start);
        printList(ptr->next);
}

/*int main()
{
        /* start always points to the first node of the linked list.
           temp is used to point to the last node of the linked list.*/
        /*struct node *start,*temp;
        start = (struct node *)malloc(sizeof(struct node)); 
        temp = start;
        temp -> next = NULL;
        /* Here in this code, we take the first node as a dummy node.
           The first node does not contain data, but it used because to avoid handling special cases
           in insert and delete functions.
         */
        /*printf("1. Insert\n");
        printf("2. Delete\n");
        printf("3. Print\n");
        printf("4. Find\n");
        while(1)
        {
                int query;
                scanf("%d",&query);
                if(query==1)
                {
                        int data;
                        scanf("%d",&data);
                        insertNode(start, 1, 2, 3, 4, 5, 6);
                }
                else if(query==2)
                {
                        int data;
                        scanf("%d",&data);
                        deleteNode(start, 3);
                }
                else if(query==3)
                {
                        printf("The list is ");
                        printList(start->next);
                        printf("\n");
                }
                else if(query==4)
                {
                        int data;
                        scanf("%d",&data);
                        struct node *status = findNode(start,4);
                        if(status)
                        {
                                printf("Element Found\n");
                        }
                        else
                        {
                                printf("Element Not Found\n");

                        }
                }
        }


}*/
