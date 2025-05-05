#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define EVENT_QUEUE_CAPACITY 64
#define MAX_EVENT_TYPES 5
#define MAX_POLL_FUNCTIONS 64

typedef struct EmptyEvent {
    uint64_t timestamp;
} EmptyEvent;

typedef struct MouseClickEvent {
    uint64_t timestamp;
    int x;
    int y;
    int button;
} MouseClickEvent;

typedef struct MouseMoveEvent {
    uint64_t timestamp;
    int dx;
    int dy;
} MouseMoveEvent;

typedef struct MouseScrollEvent {
    uint64_t timestamp;
    int value;
} MouseScrollEvent;

typedef struct KeyboardEvent {
    uint64_t timestamp;
    int keycode;
    int value; // 0 release, 1 press, 2 repeat
} KeyboardEvent;

typedef enum EventType {
    EMPTY_EVENT,
    MOUSE_CLICK_EVENT,
    MOUSE_MOVE_EVENT,
    MOUSE_SCROLL_EVENT,
    KEYBOARD_EVENT,
} EventType;

typedef struct Event {
    EventType type;
    union {
        EmptyEvent empty;
        MouseClickEvent click;
        MouseMoveEvent move;
        MouseScrollEvent scroll;
        KeyboardEvent keyboard;
    } data;
} Event;

typedef struct EventQueue {
    Event *events;
    int capacity;
    int count;
    int head;
    int tail;
} EventQueue;

typedef void (*EventHandler)(Event *event);
typedef void (*PollFunction)(EventQueue *event_queue);

static EventHandler event_handlers[MAX_EVENT_TYPES];
static PollFunction poll_functions[MAX_POLL_FUNCTIONS];
static int poll_function_count = 0;
static pthread_mutex_t poll_mutex = PTHREAD_MUTEX_INITIALIZER;

bool init_event_queue(EventQueue *event_queue, int capacity);
void destroy_event_queue(EventQueue *event_queue);
void push_event(EventQueue *event_queue, Event *event);
bool pop_event(EventQueue *event_queue, Event *event);
void print_event_queue(EventQueue *event_queue);
void register_event_handler(EventType type, EventHandler handler);
void process_event_queue(EventQueue *event_queue);
void register_poll_function(PollFunction func);
void *poll_loop(void *arg);
void start_polling();
void stop_polling();
void cleanup_polling();

#endif // __EVENTS_H__