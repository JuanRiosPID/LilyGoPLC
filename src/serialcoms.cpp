#include "serialcoms.h"
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "screen.h"

#define BAUD_RATE 38400
#define TX_1  2
#define RX_1  3

HardwareSerial port(1);
StaticJsonDocument<200> doc;


void serialcoms_setup()
{
    port.begin(BAUD_RATE,SERIAL_8N1,RX_1,TX_1);
}

void serialcoms_loop()
{
    if (port.available() > 0){
        delay(30);
        String frame = "";
        uint8_t buff = port.available();
        for (uint8_t i=0; i<buff; i++)frame += char(port.read());
        Serial.println();
        Serial.print("frame received from PLC: ");
        Serial.println(frame);
        if (frame.charAt(0) == '{' && frame.charAt(buff -1) == '}'){
            deserializeJson(doc,frame);
            if (doc.containsKey("type")){
                uint8_t type = doc["type"];
                Serial.printf("Type: %d\n",type);
                String msg = doc["msg"];
                String val = doc["val"];
                switch(type){
                    case 1:{
                        showMessage(msg,val);
                        showParameters = false;
                        break;
                    }
                    case 2:{
                        showError(String(msg),String(val));
                        showParameters = false;
                        break;
                    }
                    case 3:{
                        String ssid = doc["wss"];
                        uint8_t wstatus = doc["wst"];
                        String info = doc["win"];
                        wifi_ssid = ssid;
                        if (wstatus == 1)wifi_status = true;
                        else wifi_status = false;
                        wifi_info = info;
                        showParameters = true;
                        break;
                    }
                    case 4:{
                        String atn = doc["atn"];
                        uint8_t astatus = doc["ast"];
                        thing_name = atn;
                        if (astatus == 1)aws_status = true;
                        else aws_status = false;
                        showParameters = true;
                        break;
                    }
                    case 5:{
                        String pip = doc["pip"];
                        String ppo = doc["ppo"];
                        uint8_t pstatus = doc["pst"];
                        String mid = doc["mid"];
                        plc_ip = pip;
                        plc_port = ppo;
                        if (pstatus == 1)ethernet_status = true;
                        else ethernet_status = false;
                        modbus_id = mid;
                        showParameters = true;
                        break;
                    }
                    case 6:{
                        String fv = doc["fv"];
                        String fn = doc["fn"];
                        String fd = doc["fd"];
                        firmware_name = fn;
                        firmware_version = fv;
                        firmware_date = fd;
                        showParameters = true;
                        break;
                    }
                }
            }
        }
    }
}

/*
        Jsons de envio de esp32 ethernet

        JSON para envio de mensajes 
        {
            "type": 1, ---> 1: Envio de mensaje, 2: Envio de error, 3-7: Envio de parametros
            "msg" : "Connecting to wifi",
            "val" : "Infinitum"
        }

        JSON para envio de errores 
        {
            "type": 2, ---> 1: Envio de mensaje, 2: Envio de error, 3-7: Envio de parametros
            "msg" : "OTA firmware not found!",
            "val" : "eth_0999.bin"
        }

        JSON para envio de parametros del wifi (se abrevian los nombres para ocupar menos espacio)
        {
            "type": 3, ---> WIFI INFO
            "wss" : "Infinitum", //wifi ssid
            "wst" : 0, //wifi status --> 1 connected, 0 disconnected
            "win" : "Reconnnecting" // info del wifi
        }

        JSON para envio de parametros de aws (se abrevian los nombres para ocupar menos espacio)
        {
            "type": 4, ---> 1: AWS INFO
            "atn" : "thingName", //aws thing name
            "ast" : 0, //aws status --> 1 connected, 0 disconnected
        }

        JSON para envio de parametros del plc (se abrevian los nombres para ocupar menos espacio)
        {
            "type": 5, ---> WIFI INFO
            "pip" : "192.168.1.7", //plc ip
            "ppo" : "502", //plc port
            "pst" : 1, //plc status --> 1 connected, 0 disconnected
            "mid" : "01" // modbus id
        }

        JSON para envio de parametros del firmware(se abrevian los nombres para ocupar menos espacio)
        {
            "type": 6, ---> WIFI INFO
            "fv" : "1.0.0", //firmware version
            "fn" : "ethernetPLC_0999.bin", //firmware name
            "fd" : "xxx", //firmware date, tal como llega desde server formato ISO
            "mid" : "01" // modbus id
        }


*/