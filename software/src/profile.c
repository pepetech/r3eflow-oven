#include "profile.h"

profile_node_t* profile_node_new(float target, uint32_t time)
{
    profile_node_t* self;
    if(!(self = malloc(sizeof(profile_node_t)))) return NULL;
    self->prev = NULL;
    self->next = NULL;
    self->target = target;
    self->minTime = time;
    return self;
}

profile_t* profile_new()
{
    profile_t *self;
    if(!(self = malloc(sizeof(profile_t)))) return NULL;
    self->head = NULL;
    self->tail = NULL;
    self->len = 0;
    return self;
}

void profile_destroy(profile_t* self)
{
    uint16_t len = self->len;
    profile_node_t *next;
    profile_node_t *curr = self->head;

    while(len--)
    {
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(self);
}

profile_node_t* profile_rpush(profile_t* self, profile_node_t* node)
{
    if(!node) return NULL;

    if(self->len)
    {
        node->prev = self->tail;
        node->next = NULL;
        self->tail->next = node;
        self->tail = node;
    }
    else
    {
        self->head = self->tail = node;
        node->prev = node->next = NULL;
    }

    ++self->len;
    return node;
}

profile_node_t* profile_rpop(profile_t* self)
{
    if(!self->len) return NULL;

    profile_node_t* node = self->tail;

    if(--self->len)
    {
        (self->tail = node->prev)->next = NULL;
    }
    else
    {
        self->tail = self->head = NULL;
    }

    node->next = node->prev = NULL;
    return node;
}

profile_node_t* profile_lpop(profile_t* self)
{
    if(!self->len) return NULL;

    profile_node_t* node = self->head;

    if(--self->len)
    {
        (self->head = node->next)->prev = NULL;
    }
    else
    {
        self->head = self->tail = NULL;
    }

    node->next = node->prev = NULL;
    return node;
}

profile_node_t* profile_lpush(profile_t* self, profile_node_t* node)
{
    if(!node) return NULL;

    if(self->len)
    {
        node->next = self->head;
        node->prev = NULL;
        self->head->prev = node;
        self->head = node;
    }
    else
    {
        self->head = self->tail = node;
        node->prev = node->next = NULL;
    }

    ++self->len;
    return node;
}


profile_node_t* profile_at(profile_t* self, int16_t index)
{
    profile_direction_t direction = PROFILE_HEAD;

    if(index < 0)
    {
        direction = PROFILE_TAIL;
        index = ~index;
    }

    if((unsigned)index < self->len)
    {
        profile_iterator_t *it = profile_iterator_new(self, direction);
        profile_node_t* node = profile_iterator_next(it);
        while(index--) node = profile_iterator_next(it);
        profile_iterator_destroy(it);
        return node;
    }

    return NULL;
}

void profile_remove(profile_t* self, profile_node_t* node)
{
    node->prev ? (node->prev->next = node->next) : (self->head = node->next);

    node->next ? (node->next->prev = node->prev) : (self->tail = node->prev);

    free(node);

    --self->len;
}

profile_iterator_t* profile_iterator_new(profile_t* list, profile_direction_t direction)
{
  profile_node_t* node = (direction == PROFILE_HEAD) ? list->head : list->tail;
  return profile_iterator_new_from_node(node, direction);
}

profile_iterator_t* profile_iterator_new_from_node(profile_node_t* node, profile_direction_t direction)
{
    profile_iterator_t *self;
    if(!(self = malloc(sizeof(profile_iterator_t)))) return NULL;
    self->next = node;
    self->direction = direction;
    return self;
}

profile_node_t* profile_iterator_next(profile_iterator_t* self)
{
    profile_node_t *curr = self->next;
    if(curr)
    {
        self->next = (self->direction == PROFILE_HEAD) ? curr->next : curr->prev;
    }
    return curr;
}


void profile_iterator_destroy(profile_iterator_t* self)
{
    free(self);
    self = NULL;
}