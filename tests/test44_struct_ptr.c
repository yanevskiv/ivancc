// (Test) Return: 42
// Pointers to structs and the -> operator, including a struct that points at
// its own type. A linked list walked to its end is the shortest proof that
// both work together.

struct Node {
    int          val;
    struct Node *next;
};

int sum(struct Node *head)
{
    int total = 0;

    for (struct Node *n = head; n; n = n->next) {
        total += n->val;
    }
    return total;
}

int main()
{
    struct Node a;
    struct Node b;
    struct Node c;
    struct Node *p = &a;

    a.val = 12; a.next = &b;
    b.val = 20; b.next = &c;
    c.val = 10; c.next = 0;

    if (sizeof(struct Node) != 16) return 1;
    if (p->val != 12) return 2;
    if (p->next->val != 20) return 3;

    p->val = 12;
    (*p).next->val = 20;

    return sum(&a);
}
