#ifndef TEST
#include <espressif/esp_common.h>
#include <lwip/api.h> // vtask
#else
#include "test/mock.cpp"
#endif

#include <string.h>

#define TIMER_SIZE 10

// look at http://www.cplusplus.com/forum/general/136410/

struct Timer {
    void (*callback)();
    unsigned long time = 0;
    int id = -1;
};
Timer timer[TIMER_SIZE];

int get_free_timer()
{
    int pos = 0;
    unsigned long current_time = sdk_system_get_time();
    for(; pos < TIMER_SIZE; pos++) {
        if (timer[pos].time < current_time) {
            return pos;
        }
    }
    return -1;
}

int get_timer_by_id(int id)
{
    int pos = 0;
    for(; pos < TIMER_SIZE; pos++) {
        if (timer[pos].id == id) {
            return pos;
        }
    }
    return -1;
}

int get_timer_pos(int id)
{
    int pos = -1;
    if (id > 0) {
        pos = get_timer_by_id(id);
    }
    if (pos == -1) {
        pos = get_free_timer();
    }
    return pos;
}

int add_timer(void (*callback)(), int seconds, int id)
{
    int pos = get_timer_pos(id);
    if (pos > -1) {
        timer[pos].id = id;
        timer[pos].time = sdk_system_get_time() + (seconds * 1000000);
        timer[pos].callback = callback;
    }
    printf("timer added at pos %d with id %d in %d sec\n", pos, id, seconds);
    // printf("add timer\n");
    return pos;
}

void execute_timer()
{
    int pos = 0;
    unsigned long current_time = sdk_system_get_time();
    for(; pos < TIMER_SIZE; pos++) {
        if (timer[pos].time > 0 && timer[pos].time < current_time) {
            (timer[pos].callback)();
            timer[pos].time = 0;
        }
    }
}

void timer_task(void *pvParameters)
{
    while(1) {
        vTaskDelay(100);
        execute_timer();
    }
}
