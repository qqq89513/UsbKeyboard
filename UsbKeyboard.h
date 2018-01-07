/*
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 */
#ifndef __UsbKeyboard_h__
#define __UsbKeyboard_h__

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#include "usbdrv.h"

// TODO: Work around Arduino 12 issues better.
//#include <WConstants.h>
//#undef int()

//12.19.2017 modified by QM, works well with arduino 1.0.6

typedef uint8_t byte;


#define BUFFER_SIZE 7 // Minimum of 2, 1 for all modifiers + 1 for normal keydown
											//ex:BUFFER_SIZE 7	//support 6 normal keys and all modifiers rollover at the same time

static uchar    idleRate;           // in 4 ms units 


/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and but we do allow
 * simultaneous key presses. 
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */
PROGMEM char usbHidReportDescriptor[35] = { /* USB report descriptor */
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop) 
  0x09, 0x06,                    // USAGE (Keyboard) 
  0xa1, 0x01,                    // COLLECTION (Application) 
  0x05, 0x07,                    //   USAGE_PAGE (Keyboard) 
  0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl) 
  0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI) 
  0x15, 0x00,                    //   LOGICAL_MINIMUM (0) 
  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1) 
  0x75, 0x01,                    //   REPORT_SIZE (1) 
  0x95, 0x08,                    //   REPORT_COUNT (8) 
  0x81, 0x02,                    //   INPUT (Data,Var,Abs) 
  0x95, BUFFER_SIZE-1,           //   REPORT_COUNT (simultaneous keysdown, N-key rollover) 
  0x75, 0x08,                    //   REPORT_SIZE (8) 
  0x25, 0x65,                    //   LOGICAL_MAXIMUM (101) 
  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated)) 
  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application) 
  0x81, 0x00,                    //   INPUT (Data,Ary,Abs) 
  0xc0                           // END_COLLECTION 
};



/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 * Or check UsbKeyboardUsage.txt in the librarie folder
 */
#define MOD_CONTROL_LEFT    0xE0	//	this is not the exact code we sent.
#define MOD_SHIFT_LEFT      0xE1	//	the states(pressed or not) of modify keys 
#define MOD_ALT_LEFT        0xE2	//	are stored in the first(0) index of reportBuffer, same as reportBuffer[0].
#define MOD_GUI_LEFT        0xE3	//	there are 8 bits, each one stands for different modify key.
#define MOD_CONTROL_RIGHT   0xE4	//	here is the table
#define MOD_SHIFT_RIGHT     0xE5	//	bit||	H			6			5				4				3			2			1				L
#define MOD_ALT_RIGHT       0xE6	//	key||	RGui	RAlt	RShift	RCtrl		LGui	LAlt	LShift	LCtrl
#define MOD_GUI_RIGHT       0xE7	//	for example if we want LCtrl and LShift being pressed down simultaneously, reportBuffer should be:
																	//	reportBuffer[0]=B00000011; or reportBuffer[0]=0x03;

#define KEY_A       0x04
#define KEY_B       0x05
#define KEY_C       0x06
#define KEY_D       0x07
#define KEY_E       0x08
#define KEY_F       0x09
#define KEY_G       0x0A
#define KEY_H       0x0B
#define KEY_I       0x0C
#define KEY_J       0x0D
#define KEY_K       0x0E
#define KEY_L       0x0F
#define KEY_M       0x10
#define KEY_N       0x11
#define KEY_O       0x12
#define KEY_P       0x13
#define KEY_Q       0x14
#define KEY_R       0x15
#define KEY_S       0x16
#define KEY_T       0x17
#define KEY_U       0x18
#define KEY_V       0x19
#define KEY_W       0x1A
#define KEY_X       0x1B
#define KEY_Y       0x1C
#define KEY_Z       0x1D
#define KEY_1       0x1E	//	Keyboard 1 and !
#define KEY_2       0x1F	//	Keyboard 2 and @
#define KEY_3       0x20	//	Keyboard 3 and #
#define KEY_4       0x21	//	Keyboard 4 and $
#define KEY_5       0x22	//	Keyboard 5 and %
#define KEY_6       0x23	//	Keyboard 6 and ^
#define KEY_7       0x24	//	Keyboard 7 and &
#define KEY_8       0x25	//	Keyboard 8 and *
#define KEY_9       0x26	//	Keyboard 9 and (
#define KEY_0       0x27	//	Keyboard 0 and )

#define KEY_ENTER   0x28

#define KEY_SPACE   0x2C

#define KEY_F1      0x3A
#define KEY_F2      0x3B
#define KEY_F3      0x3C
#define KEY_F4      0x3D
#define KEY_F5      0x3E
#define KEY_F6      0x3F
#define KEY_F7      0x40
#define KEY_F8      0x41
#define KEY_F9      0x42
#define KEY_F10     0x43
#define KEY_F11     0x44
#define KEY_F12     0x45

#define KEY_ARROW_UP	 	0x52
#define KEY_ARROW_DOWN  0x51
#define KEY_ARROW_LEFT  0x50
#define KEY_ARROW_RIGHT 0x4F


class UsbKeyboardDevice{
 public:
  UsbKeyboardDevice (){
    PORTD = 0; // TODO: Only for USB pins?
    DDRD |= ~USBMASK;

    cli();
    usbDeviceDisconnect();
    usbDeviceConnect();


    usbInit();
      
    sei();

    // TODO: Remove the next two lines once we fix
    //       missing first keystroke bug properly.
    memset(reportBuffer, 0, sizeof(reportBuffer));      
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }
  void update(){
    usbPoll();
  }
	void waitReady(){
    while (!usbInterruptIsReady()) {
      // Note: We wait until we can send keystroke
      //       so we know the previous keystroke was
      //       sent.
    }
  }  

	bool keyStroke(byte key){
		reportBuffer[0] |= ( key>0xDF ? 1<<(key-0xE0)	 :	0 );	//set modify key
		byte keyStrokeIndex;
		for(keyStrokeIndex=1; keyStrokeIndex<BUFFER_SIZE; keyStrokeIndex++)//find an empty index
			if(reportBuffer[keyStrokeIndex]==0)
			{	
				reportBuffer[keyStrokeIndex]=key;	//set key down in empty index of reportBuffer
				break;
			}
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));//send keys data
		while (!usbInterruptIsReady());	//wait until transfer is done
		reportBuffer[0] &= ( key>0xDF ? ~(1<<(key-0xE0)) :	0xFF );//clear modify key
		reportBuffer[keyStrokeIndex]=0;//clear key down
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));//send keys data
	}
	bool keyDown(byte key){
		reportBuffer[0] |= ( key>0xDF ? 1<<(key-0xE0)	:	0 );	//set modify key
		byte emptyIndex=0;
		for(byte i=1; i<BUFFER_SIZE; i++)
			if(reportBuffer[i]==key)goto sendKeyDown;	//if the key is already down, 
			else if(reportBuffer[i]==0) emptyIndex=i;
		reportBuffer[emptyIndex]=key;
		sendKeyDown:
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
	}
	bool keyUp(byte key){
		reportBuffer[0] &= ~( key>0xDF ? 1<<(key-0xE0)	:	0 );	//set modify key
		for(byte i=1; i<BUFFER_SIZE; i++)
			if(reportBuffer[i]==key)reportBuffer[i]=0;//if the key in index that is already set, then clear it
		usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
	}
  
	//private: TODO: Make friend?
  uchar reportBuffer[BUFFER_SIZE];    // buffer for HID reports, could be ascess as UsbKeyboard.reportBuffer[index]
};

UsbKeyboardDevice UsbKeyboard = UsbKeyboardDevice();

#ifdef __cplusplus
extern "C"{
#endif 
  // USB_PUBLIC uchar usbFunctionSetup
uchar usbFunctionSetup(uchar data[8]) 
  {
    usbRequest_t    *rq = (usbRequest_t *)((void *)data);

    usbMsgPtr = UsbKeyboard.reportBuffer; //
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){
      /* class request type */

      if(rq->bRequest == USBRQ_HID_GET_REPORT){
	/* wValue: ReportType (highbyte), ReportID (lowbyte) */

	/* we only have one report type, so don't look at wValue */
        // TODO: Ensure it's okay not to return anything here?    
	return 0;

      }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
	//            usbMsgPtr = &idleRate;
	//            return 1;
	return 0;
      }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
	idleRate = rq->wValue.bytes[1];
      }
    }else{
      /* no vendor specific requests implemented */
    }
    return 0;
  }
#ifdef __cplusplus
} // extern "C"
#endif


#endif // __UsbKeyboard_h__
