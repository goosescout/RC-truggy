#include "global.h"


String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        uint8_t index;
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());

        for (index = 0; index < MAX_WS_CLIENT; index++) {
            if (ws_client[index].id == 0) {
                ws_client[index].id = client->id();
                ws_client[index].state = CLIENT_ACTIVE;
                Serial.printf("added #%u at index[%d]\n", client->id(), index);
                client->printf("[[b;green;]Hello Client #%u, added you at index %d]", client->id(), index);
                client->ping();
                break; // Exit for loop
            }
        }
        if (index>=MAX_WS_CLIENT) {
        Serial.printf("not added, table is full");
        client->printf("[[b;red;]Sorry client #%u, Max client limit %d reached]", client->id(), MAX_WS_CLIENT);
        client->ping();
        }

    } else if(type == WS_EVT_DISCONNECT){
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
        for (uint8_t i=0; i<MAX_WS_CLIENT ; i++) {
        if (ws_client[i].id == client->id() ) {
            ws_client[i].id = 0;
            ws_client[i].state = CLIENT_NONE;
            Serial.printf("freed[%d]\n", i);
            break; // Exit for loop
        }
        }
    } else if(type == WS_EVT_ERROR){
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG){
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    } else if(type == WS_EVT_DATA){
        //data packet
        AwsFrameInfo * info = (AwsFrameInfo*) arg;
        char * msg = NULL;
        size_t n = info->len;
        uint8_t index;
    
        // Size of buffer needed
        // String same size +1 for \0
        // Hex size*3+1 for \0 (hex displayed as "FF AA BB ...")
        n = info->opcode == WS_TEXT ? n+1 : n*3+1;
    
        msg = (char*) calloc(n, sizeof(char));
        if (msg) {
          // Grab all data
          for(size_t i=0; i < info->len; i++) {
            if (info->opcode == WS_TEXT ) {
              msg[i] = (char) data[i];
            } else {
              sprintf_P(msg+i*3, PSTR("%02x "), (uint8_t) data[i]);
            }
          }
        }
         if (info->opcode == WS_TEXT) {
          String message = String(msg);

          if (message.startsWith("f_go"))
          {
              digitalWrite(IN_A_PIN, HIGH);
              digitalWrite(IN_B_PIN, LOW);
              going = true;                
          } else if (message.startsWith("b_go"))
          {
                digitalWrite(IN_A_PIN, LOW);
                digitalWrite(IN_B_PIN, HIGH);
                going = true;
          } else if (message.startsWith("l_go"))
          {
                servo_dest = 120;
                turning = true;
          } else if (message.startsWith("r_go"))
          {
                servo_dest = 60;
                turning = true;
          } else if (message.startsWith("f_stop"))
          {
                going = false;
          } else if (message.startsWith("b_stop"))
          {
                going = false;
          } else if (message.startsWith("l_stop"))
          {
                servo_dest = servo_zero;
                turning = true;
          } else if (message.startsWith("r_stop"))
          {
                servo_dest = servo_zero;
                turning = true;
          } else if (message.startsWith("update")) {
                int power = getValue(message, ' ', 1).toInt();
                int servo = getValue(message, ' ', 2).toInt();
                max_power = power;
                servo_zero = servo;
                servomotor.write(servo_zero);
                Serial.println("updated");
                Serial.println(servo);
          };
          
         };

        // Free up allocated buffer
        if (msg) 
        free(msg);

    } // EVT_DATA
}
