#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#define LED_INT_ONTIME   CLOCK_SECOND * 10

/*---------------------------------------------------------------------------*/
/* Declare our "main" process, the basestation_process */
PROCESS(basestation_process, "Clicker basestation");
/* Declare our "main" process, the led_process */
PROCESS(led_process, "LED handling process");
/* The basestation & led process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&basestation_process, &led_process);
/*---------------------------------------------------------------------------*/
/* Holds the number of packets received. */
static int count = 0;
/*Timer for led off process*/
static struct etimer ledETimer;
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
    count++;
    /* 0bxxxxx allows us to write binary values */
    /* for example, 0b10 is 2 */
    leds_off(LEDS_ALL);
    leds_on(count & 0b1111);
  	
    process_poll(&led_process);
}

/* Our main process. */
PROCESS_THREAD(basestation_process, ev, data) {
	PROCESS_BEGIN();

	/* Initialize NullNet */
	nullnet_set_input_callback(recv);

	PROCESS_END();
}

PROCESS_THREAD(led_process, ev, data) {
  PROCESS_BEGIN();

  while (1) {
    //PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

      /* Set the LED off timer for 10 seconds */
    etimer_set(&ledETimer, LED_INT_ONTIME);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
    leds_off(LEDS_ALL); //Turn off all LEDs after 10 seconds without alarms */
 
    }
    PROCESS_END();
}

















