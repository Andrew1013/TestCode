#ifndef __AVCT_LIST_H__
#define __AVCT_LIST_H__
struct list_head {
	struct list_head *next;
};

#define AVCT_INIT_LIST_HEAD(name) { do {(name)->next = NULL;}while(0); }

#define avct_list_entry(ptr, type, member) \
	(type *)(&((type *)ptr)->member )

static inline void avct_list_add_tail(struct list_head *new, struct list_head *head)
{
    struct list_head *tmp = head;
    while (tmp->next)
    {
        tmp = tmp->next;
    }
    tmp->next = new;
    new->next = NULL;
}

static inline void avct_list_add(struct list_head *new, struct list_head *head)
{
	new->next = head->next;
	head->next = new;
}

static inline void avct_list_del(struct list_head *entry, struct list_head *head)
{
    struct list_head *tmp = head;
    if (entry == tmp)
    {
        tmp->next = entry->next;
    }
    else
    {
        while (tmp->next != entry)
        {
            tmp = tmp->next;
        }
        tmp->next = entry->next;
    }
}

static inline int avct_list_get_number(struct list_head *head)
{
    struct list_head *tmp = head;
    int counter =0;
    while (tmp->next)
    {
        tmp = tmp->next;
        counter++;
    }
    return counter;
}

#define avct_list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = (pos)?pos->next:NULL; pos ; \
		pos = n, n = (pos)?pos->next:NULL)

#define avct_list_for_each(pos, n, head) avct_list_for_each_safe(pos, n, head)


#endif
