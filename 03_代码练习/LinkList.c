#include <stdio.h>

typedef struct Link
{
    int data;
    struct Link *next;
}LinkList;

LinkList* create_LinkList()
{
    LinkList* head = (LinkList*)malloc(sizeof(LinkList));
    head->data = 0;
    head->next = NULL;
}

int is_empth(LinkList* head)
{
    if (head->next == NULL)
    {
        return 1;
    }
    return 0;
}

void insert_node(LinkList* s, int value)
{
    if (is_empth(s))
    {
        printf("The list is empty. Cannot insert a node.\n");
        return;
    }
    LinkList* new_node = (LinkList*)malloc(sizeof(LinkList));
    new_node->next = s->next;
    s->next = new_node;
    new_node->data = value;
}

void LinkList_reverse(LinkList* s)
{
    if (is_empth(s))
    {
        printf("The list is empty. Cannot reverse the list.\n");
        return;
    }
    LinkList *p = s->next;
    s->next = NULL;
    while (p->next != NULL)
    {
        LinkList *tmp = p;
        p = p->next;
        tmp->next = s->next;
        s->next = tmp; 
    }
}

void LinkList_sort(LinkList *s)
{
    if (is_empth(s))
    {
        printf("The list is empty. Cannot sort the list.\n");
        return;
    }
    LinkList *p = s->next;
    s->next = NULL;
    while (p!= NULL)
    {
        LinkList *q = p;
        p = p->next;
        q->next = s->next;
        s->next = q;
    }
}

void is_loop(LinkList *s)
{
    LinkList *slow;
    LinkList *fast;

    slow = s;
    fast = s;

    while (fast != NULL && fast->next != NULL)
    {
        slow = s->next;
        fast = s->next->next;
        if (slow == fast)
        {
            printf("is loop\n");
            return;
        }
    }
    printf("not is loop!\n");
}

int main() {


    return 0;
}