#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

/*---------------------------------------------------------------------------*/
PROCESS(clicker_ng_process, "Clicker NG Process");
AUTOSTART_PROCESSES(&clicker_ng_process);
/*---------------------------------------------------------------------------*/

#define MAX_NUMBER_OF_EVENTS 3

// Define a structure to hold event information along with node ID
struct event {
    clock_time_t time;
    linkaddr_t addr; // Storing the full linkaddr rather than pointer
};

// Array to hold event history
struct event event_history[MAX_NUMBER_OF_EVENTS];

// Variable to track the number of unique nodes that have triggered events
static uint8_t unique_node_count = 0;

// Function to handle event and check for alarm triggering
void handle_event(const linkaddr_t *src) {

    static int event_count = 0;
    if (event_count < MAX_NUMBER_OF_EVENTS) {
        // Check if the source node is already recorded in event history
        int exists = 0;
        int i;
        for (i = 0; i < event_count; i++) {
            if (linkaddr_cmp(src, &event_history[i].addr)) {
                exists = 1;
                break;
            }
        }

        // If the source node is not found in history, record the event
        if (!exists) {
            memcpy(&event_history[event_count].addr, src, sizeof(linkaddr_t));
            event_history[event_count].time = clock_time();
            event_count++;
            unique_node_count++;
            printf("New event registered from Node %d\n", src->u8[0]);
        }
    }
        // Check if alarm should be triggered
    if (unique_node_count >= MAX_NUMBER_OF_EVENTS) {
      clock_time_t event_duration = (clock_time_t)(event_history[MAX_NUMBER_OF_EVENTS - 1].time - event_history[0].time);
      printf("Event duration: %lu\n", event_duration);
      if (event_duration < (clock_time_t)30 * CLOCK_SECOND) {
        leds_toggle(LEDS_YELLOW);
        leds_toggle(LEDS_BLUE);
        printf("ALARM triggered!\n");
      }
      static int i = 0;
      for(i = 0; i < MAX_NUMBER_OF_EVENTS-1; i++) {
        event_history[i] = event_history[i+1];
        printf("removed old history\n");
      } 
    }

    // //Clear event history between alarms, constraint for alarm delays is 30 seconds
    // clock_time_t alarm_interval = (clock_time_t)(event_history[MAX_NUMBER_OF_EVENTS - 1].time - clock_time());
    //   if (alarm_interval > (clock_time_t)30 * CLOCK_SECOND) {
    //     unique_node_count = 0;
    //     event_count = 0;
    // }
    
    // // Clear event history if maximum number of events is reached
    // if (event_count >= MAX_NUMBER_OF_EVENTS) {
    //     event_count = 0;
    //     unique_node_count = 0;
    // }

 
}

static void recv(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) {
    printf("Received: %s - from %d\n", (char *)data, src->u8[0]);
    leds_toggle(LEDS_GREEN);
    handle_event(src);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(clicker_ng_process, ev, data) {
    static char payload[] = "hej";

    PROCESS_BEGIN();

    /* Initialize NullNet */
    nullnet_buf = (uint8_t *)&payload;
    nullnet_len = sizeof(payload);
    nullnet_set_input_callback(recv);

    /* Activate the button sensor. */
    SENSORS_ACTIVATE(button_sensor);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

        memcpy(nullnet_buf, &payload, sizeof(payload));
        nullnet_len = sizeof(payload);

        /* Send the content of the packet buffer using the broadcast handle. */
        NETSTACK_NETWORK.output(NULL);
    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
