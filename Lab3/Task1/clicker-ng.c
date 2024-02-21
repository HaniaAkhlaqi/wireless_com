
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

struct event {
  clock_time_t time;
  linkaddr_t addr;
};
#define MAX_NUMBER_OF_EVENTS 3
struct event event_history[MAX_NUMBER_OF_EVENTS];


void handle_event(const linkaddr_t *src) {/* Updates the event history and checks if an alarm should be triggered. This function would be called when a broadcast packet is received, or when the button on
the local node is clicked.*/
    static int event_count = 0;
    // Update event history
    if (event_count < MAX_NUMBER_OF_EVENTS) {
        event_history[event_count].addr = *src;
        event_history[event_count].time = clock_time();
        event_count++;
    }

    // Check if alarm should be triggered
    // This is just a placeholder condition, replace with your own logic
    int alarm_triggered = false;
    if (event_count >= MAX_NUMBER_OF_EVENTS) {
        alarm_triggered = true;
        if (alarm_triggered) {
            leds_toggle(LEDS_YELLOW);
        }
    }
}


void print_event_history(const struct event *event_history) {
    printf("Event History:\n");
    int i= 0;
    for (i = 0; i < MAX_NUMBER_OF_EVENTS; i++) {
        printf("Event %d: Source = %d, Time = %lu\n", i, event_history[i].addr.u8[0], event_history[i].time);
    }
}
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
  printf("Received: %s - from %d\n", (char*) data, src->u8[0]);
  leds_toggle(LEDS_GREEN);
  handle_event(src);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(clicker_ng_process, ev, data)
{
  static char payload[] = "hej";

  PROCESS_BEGIN();

  
  /* Initialize NullNet */
   nullnet_buf = (uint8_t *)&payload;
   nullnet_len = sizeof(payload);
   nullnet_set_input_callback(recv);
  
  /* Activate the button sensor. */
  SENSORS_ACTIVATE(button_sensor);

  while(1) {
  
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);

		
    memcpy(nullnet_buf, &payload, sizeof(payload));
    nullnet_len = sizeof(payload);

    /* Send the content of the packet buffer using the
     * broadcast handle. */
     NETSTACK_NETWORK.output(NULL);

    print_event_history(event_history);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
