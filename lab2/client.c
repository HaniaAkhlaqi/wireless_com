#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/adxl345.h"
#include <stdlib.h>

#define LED_INT_ONTIME        CLOCK_SECOND/2
#define ACCM_READ_INTERVAL    CLOCK_SECOND/10
#define BTN_READ_INTERVAL    CLOCK_SECOND/5

/* Declare our "main" process, the client process*/
PROCESS(client_process, "accelerometer process");
/* The client process should be started automatically when
 * the node has booted. */
AUTOSTART_PROCESSES(&client_process);

/*define timer*/
static struct etimer et;

/*define threshold for shaking*/
uint16_t error=100;
static bool flg_btn = 0;
static bool flg_accm = 0;
static char payload[3] ={0};

/* Callback function for received packets.
 *
 * Whenever this node receives a packet for its broadcast handle,
 * this function will be called.
 *
 * As the client does not need to receive, the function does not do anything
 */
static void recv(const void *data, uint16_t len,const linkaddr_t *src, const linkaddr_t *dest) {}




/* Our main process. */
PROCESS_THREAD(client_process, ev, data) {
	
	int16_t x0,x1 =0;
	PROCESS_BEGIN();
	
	/* Initialize NullNet */
	nullnet_buf = (uint8_t *)&payload;
	nullnet_set_input_callback(recv);
	
	/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    	accm_init();

	/* Loop forever. */
	while (1) {
		/* Wait until an event occurs. If the event has
		 * occured, ev will hold the type of event, and
		 * data will have additional information for the
		 * event. In the case of a sensors_event, data will
		 * point to the sensor that caused the event.
		 * Here we wait until the button was pressed. */
		//PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
		x0 = accm_read_axis(X_AXIS);
		leds_off(LEDS_ALL << 4);
		if (abs(x1 - x0) > error) 
		{
			flg_accm=1;
			if (flg_btn==0)
			{			
    	                   nullnet_len = 1;
			   leds_toggle(LEDS_RED);
			   /*send the packet*/
	                   NETSTACK_NETWORK.output(NULL);
			}	
		}
		else
		{
		  flg_accm=0;
		  
		}

		if (button_sensor.value(0))
		{
		  flg_btn=0;
		  leds_off(LEDS_GREEN);	
		}
		else
		{		
		  flg_btn=1;
		  leds_on(LEDS_GREEN);
		  if (flg_accm==1)
		  {	  
    	                  nullnet_len = 3;
			  leds_on(LEDS_ALL << 4);
			  /*send the packet*/
			  NETSTACK_NETWORK.output(NULL);			  
		  }
		  else if (flg_accm==0)
		  {
    	                  nullnet_len = 2;
			  /*send the packet*/
			  NETSTACK_NETWORK.output(NULL); 	          
		  }
		  
		}

		x1=x0;
		etimer_set(&et, ACCM_READ_INTERVAL);
      		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	PROCESS_END();
}



