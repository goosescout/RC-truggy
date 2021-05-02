#ifndef PAGE_EXAMPLE_H
#define PAGE_EXAMPLE_H

const char PAGE_example[] PROGMEM = R"=====(
<html>
    <head>
        <meta charset="UTF-8">
        <title>Управление</title>
    </head>

    <script>
        let socket = new WebSocket("ws://192.168.4.1/ws");
        document.addEventListener("DOMContentLoaded", startup);

        function update() {
            var can_send_power = false;
            var can_send_servo = false;
            power = parseInt(document.getElementById("power").value);
            servo = parseInt(document.getElementById("servo").value);
            if (isNaN(power) || power < 0 || power > 255) {
                document.getElementById("power_error").innerHTML = "Неправильное значение"
                document.getElementById("power").value = 90
            } else {
                document.getElementById("power_error").innerHTML = ""
                can_send_power = true;
            };
            if (isNaN(servo) || servo < 60 || servo > 120) {
                document.getElementById("servo_error").innerHTML = "Неправильное значение"
                document.getElementById("servo").value = 90
            } else {
                document.getElementById("servo_error").innerHTML = ""
                can_send_servo = true;
            };
            if (can_send_power && can_send_servo) {
                send_message("update " + power + " " + servo);
            };
        };
       
        function send_message(message) {
            socket.send(message);
            log("sent: " + message);
        };

        function startup() {
            var elem = document.getElementById("forward");
            elem.addEventListener("touchstart", handle_forward_start, false);
            elem.addEventListener("touchend", handle_forward_end, false);

            var elem = document.getElementById("back");
            elem.addEventListener("touchstart", handle_back_start, false);
            elem.addEventListener("touchend", handle_back_end, false);

            var elem = document.getElementById("left");
            elem.addEventListener("touchstart", handle_left_start, false);
            elem.addEventListener("touchend", handle_left_end, false);

            var elem = document.getElementById("right");
            elem.addEventListener("touchstart", handle_right_start, false);
            elem.addEventListener("touchend", handle_right_end, false);
        }
 
        function handle_forward_start(event) {
            event.preventDefault();
            send_message("f_go")
            log("touch forward started")
        }

        function handle_forward_end(event) {
            event.preventDefault();
            send_message("f_stop")
            log("touch forward ended")
        }

        function handle_back_start(event) {
            event.preventDefault();
            send_message("b_go")
            log("touch back started")
        }

        function handle_back_end(event) {
            event.preventDefault();
            send_message("b_stop")
            log("touch back ended")
        }

        function handle_left_start(event) {
            event.preventDefault();
            send_message("l_go")
            log("touch left started")
        }

        function handle_left_end(event) {
            event.preventDefault();
            send_message("l_stop")
            log("touch left ended")
        }

        function handle_right_start(event) {
            event.preventDefault();
            send_message("r_go")
            log("touch right started")
        }

        function handle_right_end(event) {
            event.preventDefault();
            send_message("r_stop")
            log("touch right ended")
        }
        
        function log(msg) {
            var p = document.getElementById('log');
            p.innerHTML = msg + "\n" + p.innerHTML;
        }
    </script>

    <style>
        button {
            box-shadow: 0px 1px 0px 0px #f0f7fa;
            background: linear-gradient(to bottom, #33bdef 5%, #019ad2 100%);
            background-color: #33bdef;
            border-radius: 16px;
            border: 1px solid #057fd0;
            cursor: pointer;
            color: #ffffff;
            font-family: Arial;
            font-size: 40px;
            font-weight: bold;
            padding: 6px 24px;
            text-decoration: none;
            text-shadow: 0px -1px 0px #5b6178;
            user-select: none;
        }

        #forward {
            display: block;
        }

        #back {
            display: block;
        }

        #left {
            display: inline;
        }

        #right {
            display: inline;
        }

        #settings {
            margin-top: 110px;
        }

        #settings > * {
            display: block;
            font-family: Arial;
        }

        #settings > p {
            line-height: 0px;
        }

        #update {
            font-size: 20px;
            margin-top: 20px;
        }

        .error {
            color: red;
            position: relative;
            top: 2px;
        }
    </style>

    <body>
        <div id="buttons">
            <div style="display: inline; position: absolute;">
                <button id="forward">^</button>
                <button id="back">v</button>
            </div>
            <div style="display: inline; position: relative; left: 120px; top: 60px;">
                <button id="left">&#60;</button>
                <button id="right">></button>
            </div>
        </div>
        <div id="settings">
            <p>Максимальная мощность (0 - 255):</p>
            <input type="text" value=90 id="power" style="display: inline;">
            <span class="error" id="power_error" style="display: inline;"></span> 
            <p>Начальный угол поворота серво (60 - 120):</p>
            <input type="text" value=90 id="servo" style="display: inline;">
            <span class="error" id="servo_error" style="display: inline;"></span> 
            <button id="update" onclick="update()">Обновить</button>
        </div>
        <pre id="log" style="margin-top: 120px;"></pre>
    </body>
</html>
)=====";
#endif


void filldynamicdata(AsyncWebServerRequest *request)
{        
    String values ="";
    values += "mydynamicdata|" + (String) + "This is filled by AJAX. Millis since start: " + (String) millis() + "|div\n";   // Build a string, like this:  ID|VALUE|TYPE
    request->send ( 200, "text/plain", values);   
}

void processExample(AsyncWebServerRequest *request)
{        
    if (request->args() > 0 )  // Are there any POST/GET Fields? 
    {
       for ( uint8_t i = 0; i < request->args(); i++ ) {  // Iterate through the fields
            if (request->argName(i) == "firstname") 
            {
                 // Your processing for the transmitted form-variable 
                 String fName = request->arg(i);
            }
        }
    }
    request->send (200, "text/html", PAGE_example); 
}
