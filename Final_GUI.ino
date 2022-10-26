#include <genieArduino.h> //library for GUI
#include <Wire.h> //library for DAC
#include<Adafruit_MCP4725.h> //library for DAC

#define DAC_resolution (9) //set value to 9,8,7,6,5 to adjust resolution of DAC

int prime_button=0; // variable to represent the state of the prime button 
int flow_power_button=0; // variable to represent the state of the flow power button
int puls_flow_button=0; // variable to represent the state of the pulsatile flow button
int page=0; // variable to represent which form is active on the screen

float control_voltage=0; //varible for controlling ADC voltage
float pump_control_flow; //the flow rate corresponding to the specified control voltage

long previousMillis_pump = 0;  //will store last time speed was changed
const long interval_pump = 750;         // period at which to change speed in ms, (500=60 BPM, 600=50 BPM, 750 = 40 BPM)
unsigned long currentMillis_pump; //store current time for changing speed of pump
int pumpstate = 0; //store whether pump is high (1) or low (0)

Genie genie;
Adafruit_MCP4725 dac;

#define RESETLINE 4



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);   // Serial0 @ 9600 baud
  dac.begin(0x60); //DAC I2C address for when ADDR is connect to GND
  genie.Begin(Serial);    // Use Serial0 for talking to the Genie library and to the 4D systems display. Port Serial0 cannot be used therefore for any other purposes such as talking to the serial monitor

  dac.setVoltage((0*4095)/5,false); //set voltage of DAC to 0V to stop pump

  genie.AttachEventHandler(myGenieEventHandler); // Attach the user function Event Handler for processing events

  // If NOT using a 4D Arduino Adaptor, digitalWrites must be reversed as Display Reset is Active Low, and
  // the 4D Arduino Adaptors invert this signal so must be Active High.
  pinMode(RESETLINE, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(RESETLINE, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(RESETLINE, 0);  // unReset the Display via D4

  // Let the display start up after the reset (This is important)
  // Increase to 4500 or 5000 if you have sync problems as your project gets larger. Can depent on microSD init speed.
  delay (3500);

  pinMode(LED_BUILTIN, OUTPUT); //set built in LED as an output
}



void loop() {
  // put your main code here, to run repeatedly:


  genie.DoEvents();   // This calls the library each loop to process the queued responses from the display

    //PRIMING~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    while (page==1) //When the prime screen (form1) is active
    {
        genie.DoEvents();   //Process the queued responses from the display

        if (prime_button == 1)
          {
            digitalWrite(LED_BUILTIN, HIGH); //turn on built in LED when prime button is pushed
            pump_control(2); //call pump control function and set voltage of DAC to 2V to operate pump at constant speed while prime button is pressed
          } 
      
        if (prime_button == 0)
          {
            digitalWrite(LED_BUILTIN, LOW); //turn off built in LED when prime button is released
            pump_control(0); //call pump control function and set voltage of DAC to 0V to turn pump off after prime button is released
          }

        genie.DoEvents();   // Process the queued responses from the display
        if (page != 1) {break;} // exit this while loop if the prime screen (form1) is no longer active
    }
      
    //CONTROL PANEL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    while (page == 2) //When the control panel (form2) is active
    {
        genie.DoEvents();   // Process the queued responses from the display

        
        
        while (flow_power_button == 0) // While the flow power button is off
          {
            digitalWrite(LED_BUILTIN, LOW); //turn off built in LED when flow power button is off
            pump_control(0); //call pump control function and set voltage of DAC to 0v to turn pump off when flow power button is off
            
            genie.DoEvents();   // This calls the library each loop to process the queued responses from the display
            if (flow_power_button != 0) {break;} // exit this while loop if the flow power button is no longer off
            if (page != 2) {break;} //exit this while loop if the control panel (form2) is no longer active
          }

          

        while (flow_power_button == 1) //While the flow power button is on
          {
            while(puls_flow_button == 0) //While the pulsatile flow button is off
              {
                digitalWrite(LED_BUILTIN, HIGH); //turn on the built in LED when the flow power button is on and the pulsatile flow button is off
                pump_control(1.65); //call pump control function and set voltage of DAC to 1.65V to operate pump at constant rate of 3.1 LPM
                
                genie.DoEvents();   // Process the queued responses from the display
                if (puls_flow_button != 0) {break;} // exit this while loop if the pulsatile flow button is no longer off
                if (flow_power_button != 1) {break;} // exit this while loop if the pulsatile flow button is no longer on
                if (page != 2) {break;} //exit this while loop if the control panel (form2) is no longer active
                
              }
              while(puls_flow_button == 1) //While the pulsatile flow button is on
              {            
                  unsigned long currentMillis_pump = millis();
                  if(currentMillis_pump - previousMillis_pump >= interval_pump) //if specific time interval has passed 
                  {
                    previousMillis_pump = currentMillis_pump; //save the last time the pump speed was changed
                
                    if (pumpstate == 0) //if the pump state was low previously
                    {
                      pumpstate = 1; //set the pump state to high
                      control_voltage = 5; //set the pump speed high
                      digitalWrite(LED_BUILTIN, HIGH); //flash on the built in LED
                    } 
                    else //if the pump state was not off previously
                    {
                      pumpstate = 0; //set the pump state to low
                      control_voltage = 0.3; //set the pump speed to low
                      digitalWrite(LED_BUILTIN, LOW); //flash off the built in LED
                    }
                    
                    pump_control(control_voltage); //operate pump at specified control voltage
                  }
                  pump_control(control_voltage); //operate pump at specified control voltage
                  
                  genie.DoEvents();   // Process the queued responses from the display
                  if (puls_flow_button != 1) {break;} // exit this while loop if the pulsatile flow button is no longer on
                  if (flow_power_button != 1) {break;} // exit this while loop if the pulsatile flow button is no longer on
                  if (page != 2) {break;} //exit this while loop if the control panel (form2) is no longer active
              }
              
              genie.DoEvents();   // Process the queued responses from the display
              if (flow_power_button != 1) {break;} // exit this while loop if the pulsatile flow button is no longer on
          }

          

        genie.DoEvents();   // Process the queued responses from the display
        if (page != 2) {break;} // exit this while if the control panel (form2) is no longer active
    }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}




void myGenieEventHandler(void)
{
  genieFrame Event;
  genie.DequeueEvent(&Event); // Remove the next queued event from the buffer, and process it below

  if (Event.reportObject.cmd == GENIE_REPORT_EVENT) //if the command recieved is from a reported event
  {
    
    if (Event.reportObject.object == GENIE_OBJ_4DBUTTON) // if the reported message was from a 4D button widget
    {
      if (Event.reportObject.index == 1) // if the reported message was from 4Dbutton1 (prime button)
      {
        prime_button = genie.GetEventData(&Event); //recieve event data from 4Dbutton1 and assign it to the prime_button variable
      }
      if(Event.reportObject.index == 2) // if the reported message was from 4Dbutton2 (flow power button)
      {
        flow_power_button = genie.GetEventData(&Event); //recieve event data from 4Dbutton2 and assign it to the flow_power_button variable
      }
      if(Event.reportObject.index == 3) //if the reported message was from 4Dbutton3 (pulsatile flow button)
      {
        puls_flow_button = genie.GetEventData(&Event); //recieve evend data from 4Dbutton3 and assign it to the puls_flow_button variable
      }
    }

    if (Event.reportObject.object == GENIE_OBJ_FORM) //if the reported message was from a form
    {
      if (Event.reportObject.index == 0) // if the reported message was from Form0 (Title screen)
      {
        page = 0;
      }
      if (Event.reportObject.index == 1) // if the reported message was from Form1 (Prime screen)
      {
        page = 1;
      }
      if (Event.reportObject.index == 2) // if the reported message was from Form2 (Control panel)
      {
        page = 2;
      }
      if (Event.reportObject.index == 3) // if the reported message was from Form3 (Flow visualiser screen)
      {
        page = 3;
      }
    }
  }
} 


float pump_control (float v) //function for controlling pump based on desired voltage (v), where 0<v<5
{
  if(v>=0 && v<=5)
  {
  dac.setVoltage((v*4095)/5,false); //set voltage of DAC based on v, to control pump speed
  }
  else
  {
    dac.setVoltage(0,false); //if v is less than zero or greater than 5, turn the pump off
  }
}
