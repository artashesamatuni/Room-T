#ifndef _EMONESP_emon_H
#define _EMONESP_emon_H

#include <Arduino.h>

// -------------------------------------------------------------------
// Commutication with EmonCMS
// -------------------------------------------------------------------

extern boolean emon_connected;
extern unsigned long packets_sent;
extern unsigned long packets_success;

// -------------------------------------------------------------------
// Publish values to EmonCMS
//
// data: a comma seperated list of name:value pairs to send
// -------------------------------------------------------------------
void emon_publish(String data);

#endif // _EMONESP_emon_H


