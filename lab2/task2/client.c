#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"

#define ACCM_READ_INTERVAL    CLOCK_SECOND

/*---------------------------------------------------------------------------*/
/* Declare our "main" process, the client process*/
PROCESS(client_process, "Clicker client");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process);
/*---------------------------------------------------------------------------*/
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
	int16_t x_1, x_2 = 0;
	PROCESS_BEGIN();

	/* Activate the button sensor. */
	SENSORS_ACTIVATE(button_sensor);

	/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    accm_init();

	/* Initialize NullNet */
	nullnet_buf = (uint8_t *)&payload;
	nullnet_len = sizeof(payload); //bandwidth?
	nullnet_set_input_callback(recv);


	/* Loop forever. */
	while (1) {
		/* Wait until an event occurs. If the event has
		 * occured, ev will hold the type of event, and
		 * data will have additional information for the
		 * event. In the case of a sensors_event, data will
		 * point to the sensor that caused the event.
		 * Here we wait until the button was pressed. */
		//PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		x_1 = accm_read_axis(X_AXIS);
		leds_off(LEDS_ALL);

		/* Copy the string "hej" into the packet buffer. */
		memcpy(nullnet_buf, &payload, sizeof(payload));
    	nullnet_len = sizeof(payload);


		if (abs(x_2 - x_1) > threshold) {
			nullnet_len = 2;
			NETSTACK_NETWORK.output(NULL);
		} else if (ev == sensors_event && data == &button_sensor){
			nullnet_len = 1;
			NETSTACK_NETWORK.output(NULL);
		}else{
			nullnet_len = 0;
			NETSTACK_NETWORK.output(NULL);
		}
		x_2 = x_1;	
		printf("x2 value  %d\n",x_2);

		etimer_set(&et, ACCM_READ_INTERVAL);
      	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}
