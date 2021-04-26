#ifndef  __SBGC_Linux__
#define  __SBGC_Linux__

#include <inttypes.h>
#include <SBGC.h>


#define LED_PIN  13

extern SBGC_Parser sbgc_parser; 

void SBGC_Demo_setup(Stream *serial);
inline void LED_ON() {   digitalWrite(LED_PIN, HIGH); }
inline void LED_OFF() {    digitalWrite(LED_PIN, LOW); }
void blink_led(uint8_t cnt);





#endif
