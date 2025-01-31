#ifndef SCREEN_H
#define SCREEN_H

#include "Arduino.h"

extern String wifi_ssid;
extern bool wifi_status;
extern String wifi_info;
extern String thing_name;
extern bool aws_status;
extern String plc_ip;
extern String plc_port;
extern bool ethernet_status;
extern String modbus_id;
extern String firmware_version;
extern String firmware_name;
extern String firmware_date;
extern bool showParameters;

void screen_setup();
void screen_loop();
void showMessage(String message, String value);
void showError(String message, String value);

#endif
