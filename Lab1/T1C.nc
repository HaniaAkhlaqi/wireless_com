/* A Simple Accelerometer Example
 *
 * Values only in the x-axis are detected in the following example.
 * For your Lab1, use extend the code for Y and Z axes.
 * Finally, interface them with a button so that the sensing starts onlt after the press of a button.
 *
 */
 
#include "UserButton.h"
#include "printfZ1.h"


module T1C @safe()
{
  	uses interface Leds;
  	uses interface Boot;

  	/* We use millisecond timer to check the shaking of client.*/
	uses interface Timer<TMilli> as TimerAccel;

  	/*Accelerometer Interface*/
	uses interface Read<uint16_t> as Xaxis;
	uses interface Read<uint16_t> as Yaxis;
	uses interface Read<uint16_t> as Zaxis;
	uses interface SplitControl as AccelControl;
}


implementation
{
    uint16_t error=100; //Set the error value
    uint16_t x;
    uint16_t y;
    uint16_t z;

    event void Boot.booted() 
    {
		call AccelControl.start(); //Starts accelerometer
   		call TimerAccel.startPeriodic(100); //Starts timer

    }

    event void AccelControl.startDone(error_t err)
	{
		printfz1("  +  Accelerometer Started\n");
		x = 0;
		y = 0;
		z = 0;
		printfz1_init();
	}

	event void AccelControl.stopDone(error_t err) 
	{
		printfz1("Accelerometer Stopped\n");
		call TimerAccel.startPeriodic(100);
	}

	event void TimerAccel.fired()
	{
		call Xaxis.read(); //Takes input from the x axis of the accelerometer
	}

    event void Xaxis.readDone(error_t result, uint16_t data)
	{

		printfz1("  +  X (%d) ", data);
		if (abs(x - data) > error) 
    	{
      		call Leds.led0On(); //LED correponding to the x-axis
		}
    
    	else
    	{
      		call Leds.led0Off(); //If difference is less than the error turn the LED off.
    	}
		
		x = data; //Store current sensor input to compare with the next. 
		call Yaxis.read(); 
	}


	
    event void Yaxis.readDone(error_t result, uint16_t data)
	{	
		printfz1(" + Y (%d) ", data);
		if (abs(y - data) > error)
		{
			call Leds.led1On(); // LED corresponfing to y-axis
		}
		else
		{
			call Leds.led1Off();
		}
		
		y = data;
		call Zaxis.read();

	}

    event void Zaxis.readDone(error_t result, uint16_t data)
	{	
		printfz1(" + Z (%d) ", data);
		if (abs(z - data) > error)
		{
			call Leds.led2On();
		}
		else
		{
			call Leds.led2Off();
		}
		z = data;
	}

}



