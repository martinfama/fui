#include "events.h"
#include "debugui.h"
#include <libevdev-1.0/libevdev/libevdev.h>
#include <unistd.h>

bool init_event_queue(EventQueue *event_queue, int capacity) {
    event_queue->events = (Event *)malloc(capacity * sizeof(Event));
    if (event_queue->events == NULL) {
        return false;
    }
    event_queue->capacity = capacity;
    event_queue->count = 0;
    event_queue->head = 0;
    event_queue->tail = 0;
    return true;
}

void destroy_event_queue(EventQueue *event_queue) {
    if (event_queue->events != NULL) {
        free(event_queue->events);
        event_queue->events = NULL;
    }
    event_queue->capacity = 0;
    event_queue->count = 0;
    event_queue->head = 0;
    event_queue->tail = 0;
}

void push_event(EventQueue *event_queue, Event *event) {
    if (event_queue->count < event_queue->capacity) {
        event_queue->events[event_queue->tail] = *event;
        event_queue->tail = (event_queue->tail + 1) % event_queue->capacity;
        event_queue->count++;
    } else {
        event_queue->head = (event_queue->head + 1) % event_queue->capacity;
        event_queue->events[event_queue->tail] = *event;
        event_queue->tail = (event_queue->tail + 1) % event_queue->capacity;
    }
}

bool pop_event(EventQueue *event_queue, Event *event) {
    if (event_queue->count > 0) {
        *event = event_queue->events[event_queue->head];
        event_queue->head = (event_queue->head + 1) % event_queue->capacity;
        event_queue->count--;
        return true;
    }
    return false;
}

void print_event_queue(EventQueue *event_queue) {
    for (int i = 0; i < event_queue->count; i++) {
        Event event = event_queue->events[(event_queue->head + i) % event_queue->capacity];
        switch (event.type) {
        case EMPTY_EVENT:
            pdebugui("Empty Event: Timestamp: %lld\n", event.data.empty.timestamp);
            break;
        case MOUSE_CLICK_EVENT:
            pdebugui("Mouse Click Event: Timestamp: %lld, X: %d, Y: %d, Button: %s\n", event.data.click.timestamp,
                     event.data.click.x, event.data.click.y,
                     libevdev_event_code_get_name(EV_KEY, event.data.click.button));
            break;
        case MOUSE_MOVE_EVENT:
            pdebugui("Mouse Move Event: Timestamp: %lld, DX: %d, DY: %d\n", event.data.move.timestamp,
                     event.data.move.dx, event.data.move.dy);
            break;
        case MOUSE_SCROLL_EVENT:
            pdebugui("Mouse Scroll Event: Timestamp: %lld, Value: %d\n", event.data.scroll.timestamp,
                     event.data.scroll.value);
            break;
        case KEYBOARD_EVENT:
            pdebugui("Keyboard Event: Timestamp: %lld, Keycode: %s, Value: %d\n", event.data.keyboard.timestamp,
                     libevdev_event_code_get_name(EV_KEY, event.data.keyboard.keycode), event.data.keyboard.value);
            break;
        default:
            pdebugui("Unknown Event Type: %d\n", event.type);
            break;
        }
    }
}

void register_event_handler(EventType type, EventHandler handler) { event_handlers[type] = handler; }

void process_event_queue(EventQueue *event_queue) {
    Event event;
    while (pop_event(event_queue, &event)) {
        if (event_handlers[event.type] != NULL) {
            event_handlers[event.type](&event);
        } else {
            pdebugui("No handler for event type %d\n", event.type);
        }
    }
}

void register_poll_function(PollFunction func) {
    if (poll_function_count < MAX_POLL_FUNCTIONS)
        poll_functions[poll_function_count++] = func;
}

static bool running_poll_thread = true;
static pthread_t poll_thread;
void *poll_loop(void *arg) {
    EventQueue *event_queue = (EventQueue *)arg;
    while (running_poll_thread) {
        pthread_mutex_lock(&poll_mutex);
        for (int i = 0; i < poll_function_count; i++) {
            poll_functions[i](event_queue);
        }
        pthread_mutex_unlock(&poll_mutex);
        usleep(10);
    }
    return NULL;
}

void start_polling(EventQueue *event_queue) {
    running_poll_thread = true;
    pthread_create(&poll_thread, NULL, poll_loop, event_queue);
}

void stop_polling() {
    running_poll_thread = false;
    pthread_join(poll_thread, NULL);
}

void cleanup_polling() {
    stop_polling();
    pthread_mutex_destroy(&poll_mutex);
}