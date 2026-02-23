#pragma once
#include <Arduino.h>

const char PAGE_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 SOS Blinker</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; margin: 0; padding: 20px; background-color: #f4f4f4; color: #333; }
    .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h1, h2 { text-align: center; color: #2c3e50; }
    label { font-weight: bold; display: block; margin-top: 10px; }
    input[type=text], input[type=password], select { width: 100%; padding: 10px; margin: 5px 0 15px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
    input[type=checkbox] { margin-right: 5px; }
    button { width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }
    button:hover { background-color: #45a049; }
    .scan-results { border: 1px solid #ddd; max-height: 200px; overflow-y: auto; margin-bottom: 15px; }
    .scan-item { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; }
    .scan-item:hover { background-color: #f0f0f0; }
    .footer { margin-top: 20px; text-align: center; font-size: 0.8em; color: #777; }
  </style>
  <script>
    function selectNetwork(ssid) {
      document.getElementById('ssid').value = ssid;
      document.getElementById('pass').focus();
    }
  </script>
</head>
<body>
<div class="container">
  <h1>SOS Blinker Configuration</h1>
)rawliteral";

const char PAGE_FOOTER[] PROGMEM = R"rawliteral(
  <div class="footer">
    ESP32 SOS Blinker v1.0
  </div>
</div>
</body>
</html>
)rawliteral";
