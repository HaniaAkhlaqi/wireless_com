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

/*---------------------------------------------------------------------------*/
/* Declare our "main" process, the client process*/
PROCESS(client_process, "Clicker client");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process);
/*---------------------------------------------------------------------------*/
//timer for accelerometer read interval
static struct etimer et;
uint16_t threshold = 100;

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
	int result;
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
		PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event && data == &button_sensor) || (etimer_expired(&et)));

		x = accm_read_axis(X_AXIS);

		/* Copy the string "hej" into the packet buffer. */
		memcpy(nullnet_buf, &payload, sizeof(payload));
    	nullnet_len = sizeof(payload);

		result = process_post(&client_process, ev == sensors_event, NULL);

		if ((x > threshold) || (-1 * x > threshold)) {
			NETSTACK_NETWORK.output(NULL);
			leds_toggle(LEDS_RED);
			printf("acc Sent\n");
		}

		if(result == PROCESS_ERR_OK) {
			NETSTACK_NETWORK.output(NULL);
			printf("btn Sent\n");
			leds_toggle(LEDS_GREEN);
		} else {
			printf("Error\n");
		}
		
		etimer_set(&et, ACCM_READ_INTERVAL);
      	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
