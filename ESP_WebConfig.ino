/* 
ESP_WebConfig 

Copyright (c) 2015 John Lassen. All rights reserved.
This is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Latest version: 1.1.3  - 2015-07-20
Changed the loading of the Javascript and CCS Files, so that they will successively loaded and that only one request goes to the ESP.

-----------------------------------------------------------------------------------------------
History

Version: 1.1.2  - 2015-07-17
Added URLDECODE for some input-fields (SSID, PASSWORD...)

Version  1.1.1 - 2015-07-12
First initial version to the public
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>

#include <Servo.h>

#include <Ticker.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include "helpers.h"
#include "global.h"
#include "websocket.h"
/*
Include the HTML, STYLE and Script "Pages"
*/
#include "Page_Root.h"
#include "Page_Admin.h"
#include "Page_Script.js.h"
#include "Page_Style.css.h"
#include "Page_Jquery.min.js.h"
#include "Page_NTPsettings.h"
#include "Page_Information.h"
#include "Page_General.h"
#include "PAGE_NetworkConfiguration.h"
#include "example.h"

#define ACCESS_POINT_NAME "Car Remote Control"
#define ACCESS_POINT_PASSWORD "12345678"
#define AdminTimeOut 180 // Defines the Time in Seconds, when the Admin-Mode will be disabled

uint8_t motorChan = 5;

void motor_control() {
    if (going && power < max_power) {
        if (power == 0){
            power = 30;
        } else {
            power += power_step;  
        }

        if (power >= max_power) {
            power = max_power;
        }
        ledcWrite(motorChan, power);
        delay(3);

        Serial.println(power);
    }
    else if (!going && power > 0) {
        power -= power_step;

        if (power <= 0) {
            power = 0;
            digitalWrite(IN_A_PIN, HIGH);
            digitalWrite(IN_B_PIN, HIGH);
        }
        ledcWrite(motorChan, power);
        delay(2);

        Serial.println(power);
    }
}


void setup(void)
{
    EEPROM.begin(512);
    Serial.begin(115200);
    delay(500);
    Serial.println("Starting ES8266");
    if (!ReadConfig())
    {
        // DEFAULT CONFIG
        config.ssid = "MYSSID";
        config.password = "MYPASSWORD";
        config.dhcp = true;
        config.IP[0] = 192;
        config.IP[1] = 168;
        config.IP[2] = 1;
        config.IP[3] = 100;
        config.Netmask[0] = 255;
        config.Netmask[1] = 255;
        config.Netmask[2] = 255;
        config.Netmask[3] = 0;
        config.Gateway[0] = 192;
        config.Gateway[1] = 168;
        config.Gateway[2] = 1;
        config.Gateway[3] = 1;
        config.ntpServerName = "0.de.pool.ntp.org";
        config.Update_Time_Via_NTP_Every = 0;
        config.timezone = -10;
        config.daylight = true;
        config.DeviceName = "Not Named";
        config.AutoTurnOff = false;
        config.AutoTurnOn = false;
        config.TurnOffHour = 0;
        config.TurnOffMinute = 0;
        config.TurnOnHour = 0;
        config.TurnOnMinute = 0;
        WriteConfig();
        Serial.println("General config applied");
    }
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ACCESS_POINT_NAME, ACCESS_POINT_PASSWORD);

    pinMode(IN_A_PIN, OUTPUT);
    pinMode(EN_A_PIN, INPUT);
    pinMode(PWM_PIN, OUTPUT);
    pinMode(EN_B_PIN, INPUT);
    pinMode(IN_B_PIN, OUTPUT);

    ledcAttachPin(PWM_PIN, motorChan);
    ledcSetup(motorChan, 5000, 8);
    //digitalWrite(EN_A_PIN, HIGH);
    //digitalWrite(EN_B_PIN, HIGH);
    digitalWrite(IN_A_PIN, HIGH);
    digitalWrite(IN_B_PIN, HIGH);
    
    //pinMode(27, OUTPUT);
    servomotor.attach(27);
    servomotor.write(servo_zero);

    //ledcWrite(1, 50);

    server.on("/", processExample);
    server.on("/admin/filldynamicdata", filldynamicdata);

    server.on("/favicon.ico", [](AsyncWebServerRequest *request) { Serial.println("favicon.ico"); request->send ( 200, "text/html", "" ); });

    server.on("/admin.html", [](AsyncWebServerRequest *request) { Serial.println("admin.html"); request->send ( 200, "text/html", PAGE_AdminMainPage ); });
    server.on("/config.html", send_network_configuration_html);
    server.on("/info.html", [](AsyncWebServerRequest *request) { Serial.println("info.html"); request->send ( 200, "text/html", PAGE_Information ); });
    server.on("/ntp.html", send_NTP_configuration_html);
    server.on("/general.html", send_general_html);

    server.on("/style.css", [](AsyncWebServerRequest *request) { Serial.println("style.css"); request->send ( 200, "text/css", PAGE_Style_css ); });
    server.on("/microajax.js", [](AsyncWebServerRequest *request) { Serial.println("microajax.js"); request->send ( 200, "text/plain", PAGE_microajax_js ); });
    server.on("/admin/values", send_network_configuration_values_html);
    server.on("/admin/connectionstate", send_connection_state_values_html);
    server.on("/admin/infovalues", send_information_values_html);
    server.on("/admin/ntpvalues", send_NTP_configuration_values_html);
    server.on("/admin/generalvalues", send_general_configuration_values_html);
    server.on("/admin/devicename", send_devicename_value_html);

    // server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){ Serial.println("jquery"); request->send(200, "text/javascript", PAGE_Jquery);});

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.onNotFound([](AsyncWebServerRequest *request) { Serial.println("Page Not Found"); request->send ( 400, "text/html", "Page not Found" ); });
    server.begin();

    Serial.println("HTTP server started");
    //tkSecond.attach(1, Second_Tick);
    //UDPNTPClient.begin(2390); // Port for NTP receive

}

void loop(void)
{
    if (Refresh)
    {
        Refresh = false;
    }

    motor_control();
    if (turning) {
        if (servo_dest < servo_pos) {
            servo_pos -= 1;
            servomotor.write(servo_pos);

            if (power >= 150) {
                delay(20);
            } else {
                delay(10);
            }

        } else if (servo_dest > servo_pos) {
            servo_pos += 1;
            servomotor.write(servo_pos);
            
            if (power >= 150) {
                delay(20);
            } else {
                delay(10);
            }

        } else {
            turning = false;
        }
        Serial.println(servo_pos);
    }

    Serial.println("DIAG_A, DIAG_B:");
    Serial.println(digitalRead(EN_A_PIN));
    Serial.println(digitalRead(EN_B_PIN));
    
}
