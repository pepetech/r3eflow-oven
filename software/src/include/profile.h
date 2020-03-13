#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <stdlib.h>
#include <em_device.h>

typedef enum
{
  PROFILE_HEAD,
  PROFILE_TAIL
} profile_direction_t;

typedef struct profile_node
{
  struct profile_node* prev;
  float target;
  float ramp;
  uint32_t minTime;
  uint32_t maxTime;
  uint32_t maxRampTime;
  struct profile_node* next;
} profile_node_t;

typedef struct
{
  profile_node_t *head;
  profile_node_t *tail;
  uint16_t len;
} profile_t;

typedef struct
{
  profile_node_t *next;
  profile_direction_t direction;
} profile_iterator_t;

profile_node_t* profile_node_new(float target, uint32_t time);

profile_t* profile_new();

profile_node_t* profile_rpush(profile_t* self, profile_node_t* node);

profile_node_t* profile_lpush(profile_t* self, profile_node_t* node);

profile_node_t* profile_at(profile_t* self, int16_t index);

profile_node_t* profile_rpop(profile_t* self);

profile_node_t* profile_lpop(profile_t* self);

void profile_remove(profile_t* self, profile_node_t* node);

void profile_destroy(profile_t* self);

profile_iterator_t* profile_iterator_new(profile_t* profile, profile_direction_t direction);

profile_iterator_t* profile_iterator_new_from_node(profile_node_t* node, profile_direction_t direction);

profile_node_t* profile_iterator_next(profile_iterator_t* self);

void profile_iterator_destroy(profile_iterator_t* self);

#endif // __PROFILE_H__