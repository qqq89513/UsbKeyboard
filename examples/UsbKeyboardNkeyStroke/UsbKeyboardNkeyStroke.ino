//work with Arduino IDE 1.0.6

/*
	Bugs we know:not working with serial port(D0, D1) working simuntaniously. 
	Remove the usb port for programing(ex.mini usb on nano) after programmed, then replug the HID usb port.
	Circuit in folder /UsbKeyboard/examples/UsbKeyboardNkeyStroke/circuit/
*/

/*
	已知問題:無法與串列通訊腳(D0, D1)同時作業
	程式燒錄完成後移除燒錄的usb port, 然後拔掉當作鍵盤的USB再插回去
	電路圖位置:/UsbKeyboard/examples/UsbKeyboardNkeyStroke/circuit/
*/

#include "UsbKeyboard.h"

#define BUTTON_PINstart 6  //from D6
#define BUTTON_PINend   12 //to D12, are all pins for buttons
#define PIN_LED      13		 //toggle led state each button click

byte keyMap[7]={KEY_A, KEY_B, KEY_C, KEY_D, MOD_CONTROL_LEFT, MOD_SHIFT_LEFT, MOD_ALT_LEFT};
boolean pressedMap[7];


void setup()
{ 
  for(byte i=BUTTON_PINstart; i<=BUTTON_PINend; i++)pinMode(i, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
}


void loop(){
  UsbKeyboard.update();//update should be done more often then once/50ms

  for(byte i=BUTTON_PINstart; i<=BUTTON_PINend; i++)//check each pin state
  {
    if ( ( !digitalRead(i) ) && ( !pressedMap[i-BUTTON_PINstart] ) )//button down
    {
      UsbKeyboard.keyStroke(keyMap[i-BUTTON_PINstart]);
      pressedMap[i-BUTTON_PINstart]=1;//debounce
      delayMicroseconds(700);//debounce
    }
    else if(digitalRead(i) && pressedMap[i-BUTTON_PINstart])//button up
    {
      pressedMap[i-BUTTON_PINstart]=0;//debounce
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));//toggle led each click
      delayMicroseconds(700);//debounce
    }
  }
}
