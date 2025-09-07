#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;
//判断是否成环
void Fast_and_slow_point(Node *head)
{
    Node *quick = head;
    Node *slow = head;

    while(quick != NULL && quick->next != NULL)
    {
        quick = quick->next->next;
        slow = slow->next;
        if(quick == slow)
        {
            printf("=========yesyes\n");
            return;
        }
    }
    printf("========nononono\n");
    return;
}

Node *create_empty_head()
{
    Node *head = (Node *)malloc(sizeof(Node));
    head->next = NULL;
    return head;
}

//头插法
Node *push_front(Node *head, int value)
{
    if(head == NULL)
    {
        return head;
    }

    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->next = head->next;
    head->next = newNode;
    newNode->data = value;

    printf("22222222222222__%d\n", newNode->data);

    return head;
}

//尾插法
Node *push_back(Node *head, int value)
{
    if(head == NULL)
    {
        return head;
    }

    Node *p = head;

    while (p->next != NULL)
    {
        p = p->next;
    }
    
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->next = NULL;
    p->next = newNode;
    newNode->data = value;

    return head;
}

//头删法
Node *pop_front(Node *head)
{
    if(head == NULL)
    {
        return head;
    }

    Node *p = head->next;
    head->next = p->next;
    free(p);
}

//尾删法
Node *pop_back(Node *head)
{
    if(head == NULL)
    {
        return head;
    }

    Node *p = head;
    Node *q = NULL;
    while(p->next->next != NULL)
    {
        p = p->next;
    }
    q = p->next;
    p->next = NULL;
    free(q);
    return head;
}

void show_linklist(Node *head)
{
    if(head == NULL)
    {
        printf("55555555\n");
        return;
    }

    Node *p = head;
    while (p->next != NULL)
    {
        printf("-----p->value == %d\n", p->next->data);
        p = p->next;
    }
}

//链表排序
Node *linklist_sort(Node *head)
{
    if(head == NULL)
    {
        return NULL;
    }
    Node *p = head;
    Node *q = head->next->next;
    while (p->next !=)
    {
        /* code */
    }
    

    return head;
}

int main()
{
    Node *head = create_empty_head();
    printf("1111111111111111\n");
    push_front(head, 10);
    push_front(head, 20);
    push_front(head, 30);

    push_back(head, 40);
    push_back(head, 50);
    push_back(head, 60);

    show_linklist(head);

    Fast_and_slow_point(head);
}