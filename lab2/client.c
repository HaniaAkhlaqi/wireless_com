#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"

#define ACCM_READ_INTERVAL    CLOCK_SECOND * 5
#define EVENT_INTERVAL		CLOCK_SECOND * 10

/*---------------------------------------------------------------------------*/
/* Declare our "main" process, the client process*/
PROCESS(client_process, "Clicker client");
PROCESS(event_timing, "LED handling process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process, &event_timing);
/*---------------------------------------------------------------------------*/
//timer for accelerometer read interval
static struct etimer acc_timer;
static struct etimer event_timer;
uint16_t threshold = 100;
static bool button_triggered = 0;
static bool acc_triggered = 0;
/*---------------------------------------------------------------------------*/

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest) {
}

/* Our main process. */
PROCESS_THREAD(client_process, ev, data) {
	static char payload[] = "hej";
	int16_t x = 0;
	PROCESS_BEGIN();

	/* Activate the button sensor. */
	SENSORS_ACTIVATE(button_sensor);

	/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    accm_init();

	/* Initialize NullNet */
	nullnet_buf = (uint8_t *)&payload;
	nullnet_len = sizeof(payload); 
	nullnet_set_input_callback(recv);


	/* Loop forever. */
	while (1) {
		/* Wait until an event occurs. If the event has
		 * occured, ev will hold the type of event, and
		 * data will have additional information for the
		 * event. In the case of a sensors_event, data will
		 * point to the sensor that caused the event.
		 * Here we wait until the button was pressed. */
		PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event && data == &button_sensor) || (etimer_expired(&acc_timer)));
		process_poll(&event_timing);
		
		x = accm_read_axis(X_AXIS);

		/* Copy the string "hej" into the packet buffer. */
		memcpy(nullnet_buf, &payload, sizeof(payload));

		if ((x > threshold) || (-1 * x > threshold)) {
			acc_triggered = 1;	
        } else {
			acc_triggered = 0;
		}

		if(ev == sensors_event && data == &button_sensor){
			button_triggered = 1;
		} else {
			button_triggered = 0;
		}	

		if (acc_triggered && button_triggered) {
			if (ev == PROCESS_EVENT_TIMER){
				nullnet_len = 3;
				leds_toggle(LEDS_RED);
				leds_toggle(LEDS_GREEN);
				NETSTACK_NETWORK.output(NULL);
				printf("acc and btn Sent\n");
			}
		} else if(butten_triggered) {
			nullnet_len = 2;
			leds_toggle(LEDS_GREEN);
			NETSTACK_NETWORK.output(NULL);
			printf("btn Sent\n");
		} else{
			nullnet_len = 1;
			leds_toggle(LEDS_RED);
			NETSTACK_NETWORK.output(NULL);
			printf("acc Sent\n");
		}
	
		etimer_set(&acc_timer, ACCM_READ_INTERVAL);
      	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&acc_timer));
	}

	PROCESS_END();
}

PROCESS_THREAD(event_timing, ev, data) {
  PROCESS_BEGIN();

  while (1){
      /* Set the LED off timer for 10 seconds */
    etimer_set(&event_timer, EVENT_INTERVAL);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
	post_process(&client_process, PROCESS_EVENT_TIMER, NULL);
 
    }
    PROCESS_END();
}
