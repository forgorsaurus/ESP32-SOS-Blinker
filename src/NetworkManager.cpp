#include "NetworkManager.h"
#include "html_pages.h"
#include "definitions.h"
#include <Update.h>

NetworkManager::NetworkManager(ConfigManager& configMgr) 
    : _configMgr(configMgr), _server(80), _apMode(false), _lastWifiCheck(0), _lastNetworkStatus(WL_IDLE_STATUS) {
}

void NetworkManager::begin() {
    _config = _configMgr.load();
    
    pinMode(PIN_BTN_CONFIG, INPUT_PULLUP);
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);
    
    setupWebServer();

    // Initial check: if no SSID configured, force AP
    if (_config.wifi_ssid.length() == 0) {
        startAP();
    } else {
        startSTA();
    }
}

void NetworkManager::update() {
    _server.handleClient();
    
    if (_apMode) {
        _dnsServer.processNextRequest();
        // AP Blink: 2s period (1s on, 1s off)
        if ((millis() % 2000) < 1000) digitalWrite(PIN_LED_STATUS, HIGH);
        else digitalWrite(PIN_LED_STATUS, LOW);
    } else {
        // STA Mode: OFF (Status LED)
        digitalWrite(PIN_LED_STATUS, LOW);
        
        // Log WiFi Status Changes (WIFI-004)
        wl_status_t currentStatus = WiFi.status();
        if (currentStatus != _lastNetworkStatus) {
            Serial.print("WiFi Status Changed: ");
            switch (currentStatus) {
                case WL_IDLE_STATUS: Serial.println("Idle"); break;
                case WL_NO_SSID_AVAIL: Serial.println("No SSID Available"); break;
                case WL_SCAN_COMPLETED: Serial.println("Scan Completed"); break;
                case WL_CONNECTED: Serial.println("Connected"); Serial.print("IP: "); Serial.println(WiFi.localIP()); break;
                case WL_CONNECT_FAILED: Serial.println("Connection Failed"); break;
                case WL_CONNECTION_LOST: Serial.println("Connection Lost"); break;
                case WL_DISCONNECTED: Serial.println("Disconnected"); break;
                default: Serial.println(currentStatus); break;
            }
            _lastNetworkStatus = currentStatus;
        }

        // Reconnect logic (WIFI-003)
        if (millis() - _lastWifiCheck > 10000) {
            _lastWifiCheck = millis();
            if (WiFi.status() != WL_CONNECTED) {
                WiFi.reconnect();
            }
        }
    }
    
    // Button Logic (Hold 5s to force AP)
    static unsigned long btnPressStart = 0;
    if (digitalRead(PIN_BTN_CONFIG) == LOW) {
        if (btnPressStart == 0) btnPressStart = millis();
        else if (millis() - btnPressStart > 5000) {
            startAP();
            btnPressStart = 0;
            // Wait for release
            while(digitalRead(PIN_BTN_CONFIG) == LOW) delay(10);
        }
    } else {
        btnPressStart = 0;
    }
}

void NetworkManager::startSTA() {
    Serial.println("WiFi STA mode");
    _apMode = false;
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(_config.device_name.c_str());
    
    if (!_config.wifi_dhcp && _config.wifi_ip.length() > 0) {
        IPAddress ip, gw, sn, dns;
        ip.fromString(_config.wifi_ip);
        gw.fromString(_config.wifi_gateway);
        sn.fromString(_config.wifi_subnet);
        dns.fromString(_config.wifi_dns);
        WiFi.config(ip, gw, sn, dns);
    }
    
    WiFi.begin(_config.wifi_ssid.c_str(), _config.wifi_pass.c_str());
    _server.begin();
}

void NetworkManager::startAP() {
    Serial.println("WiFi AP mode");
    _apMode = true;
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    
    // FSD: AP SHALL assign IP 192.168.1.1 to clients
    IPAddress apIP(192, 168, 1, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    
    WiFi.softAP(_config.ap_ssid.c_str(), _config.ap_pass.c_str());
    
    _dnsServer.start(53, "*", WiFi.softAPIP());
    _server.begin();
}

void NetworkManager::setupWebServer() {
    _server.on("/", HTTP_GET, std::bind(&NetworkManager::handleRoot, this));
    _server.on("/save", HTTP_POST, std::bind(&NetworkManager::handleSave, this));
    _server.on("/generate_204", std::bind(&NetworkManager::handleNotFound, this));
    _server.on("/hotspot-detect.html", std::bind(&NetworkManager::handleNotFound, this));
    _server.onNotFound(std::bind(&NetworkManager::handleNotFound, this));
    
    // OTA
    _server.on("/update", HTTP_POST, 
        [this]() {
            bool success = !Update.hasError();
            if (success) {
                Serial.println("OTA update success");
                // Success: Blink 100ms * 3 times
                for (int i = 0; i < 3; i++) {
                    digitalWrite(PIN_LED_STATUS, HIGH); delay(100);
                    digitalWrite(PIN_LED_STATUS, LOW); delay(100);
                }
                _server.send(200, "text/plain", "Update Success! Rebooting...");
                delay(1000);
                ESP.restart();
            } else {
                Serial.println("OTA update failed");
                // Fail: Blink 100ms * 5 times
                for (int i = 0; i < 5; i++) {
                    digitalWrite(PIN_LED_STATUS, HIGH); delay(100);
                    digitalWrite(PIN_LED_STATUS, LOW); delay(100);
                }
                _server.send(500, "text/plain", "Update Failed");
            }
        },
        [this]() {
            HTTPUpload& upload = _server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.println("OTA update started");
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
                
                // Progress logging (optional but good for OTA-005)
                static int lastProgress = -1;
                int progress = (Update.progress() * 100) / Update.size();
                if (progress != lastProgress && progress % 10 == 0) {
                    Serial.printf("Progress: %d%%\n", progress);
                    lastProgress = progress;
                }

                // OTA Update Blink: Every 250ms (FSD)
                static unsigned long lastBlink = 0;
                if (millis() - lastBlink > 125) { // 125ms on, 125ms off -> 250ms period (4Hz)
                    lastBlink = millis();
                    digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    Serial.printf("OTA update finished: %u bytes\n", upload.totalSize);
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );
}

void NetworkManager::handleRoot() {
    String html = FPSTR(PAGE_HEADER);
    
    // WiFi Scan
    html += "<div class='card'><h2>WiFi Configuration</h2><div class='scan-results'>";
    if (_apMode) {
        int n = WiFi.scanNetworks();
        if (n == 0) {
            html += "<div class='scan-item'>No networks found</div>";
        } else if (n < 0) {
            html += "<div class='scan-item'>Scan failed</div>";
        } else {
            for (int i = 0; i < n; ++i) {
                String rssi = String(WiFi.RSSI(i));
                // Simple visual indicator for signal strength could be added here
                html += "<div class='scan-item' onclick=\"selectNetwork('" + WiFi.SSID(i) + "')\">";
                html += "<strong>" + WiFi.SSID(i) + "</strong> (" + rssi + " dBm)</div>";
            }
        }
    } else {
        html += "<div class='scan-item'>Scanning disabled in Station Mode.<br>Switch to AP mode to scan.</div>";
        if (WiFi.status() == WL_CONNECTED) {
             html += "<div class='scan-item'><strong>Current: " + _config.wifi_ssid + "</strong> (" + String(WiFi.RSSI()) + " dBm)</div>";
        }
    }
    html += "</div></div>";
    
    // WiFi Config Form
    html += "<div class='card'><form action='/save' method='POST'>";
    html += "<label>SSID:</label><input type='text' id='ssid' name='ssid' value='" + _config.wifi_ssid + "'>";
    html += "<label>Password:</label><input type='password' id='pass' name='pass' value='" + _config.wifi_pass + "'>";
    
    // DHCP Toggle
    String checked = _config.wifi_dhcp ? "checked" : "";
    String manualStyle = _config.wifi_dhcp ? "display:none" : "display:block";
    
    html += "<label><input type='checkbox' name='dhcp' " + checked + " onchange='toggleIP(this.checked)'> Use DHCP</label>";
    
    // Manual IP Fields
    html += "<div id='manual_ip' style='" + manualStyle + "'>";
    html += "<label>IP Address:</label><input type='text' name='ip' value='" + _config.wifi_ip + "'>";
    html += "<label>Gateway:</label><input type='text' name='gateway' value='" + _config.wifi_gateway + "'>";
    html += "<label>Subnet Mask:</label><input type='text' name='subnet' value='" + _config.wifi_subnet + "'>";
    html += "<label>DNS Server:</label><input type='text' name='dns' value='" + _config.wifi_dns + "'>";
    html += "</div>";
    
    html += "<button type='submit'>Save & Connect</button></form></div>";
    
    // Firmware Update
    uint32_t freeSpace = ESP.getFreeSketchSpace();
    String freeSpaceStr = String(freeSpace / 1024.0 / 1024.0, 2) + " MB";
    
    html += "<div class='card'><h2>Firmware Update</h2>";
    html += "<p><strong>Current Version:</strong> 1.0.0</p>"; // Hardcoded per FSD
    html += "<p><strong>Build Date:</strong> " + String(__DATE__) + " " + String(__TIME__) + "</p>";
    html += "<p><strong>Free Space:</strong> " + freeSpaceStr + "</p>";
    
    html += "<input type='file' id='update_file' name='update'>";
    html += "<button onclick='uploadFile()'>Upload Firmware</button>";
    
    html += "<div id='progress-container'><div id='progress-bar'><div id='progress-fill'>0%</div></div></div>";
    html += "<p style='font-size:0.8em; color:#999;'>⚠ Do not power off during update</p>";
    html += "</div>";
    
    html += FPSTR(PAGE_FOOTER);
    _server.send(200, "text/html", html);
}

void NetworkManager::handleSave() {
    if (_server.hasArg("ssid")) _config.wifi_ssid = _server.arg("ssid");
    if (_server.hasArg("pass")) _config.wifi_pass = _server.arg("pass");
    
    _config.wifi_dhcp = _server.hasArg("dhcp");
    
    if (_server.hasArg("ip")) _config.wifi_ip = _server.arg("ip");
    if (_server.hasArg("gateway")) _config.wifi_gateway = _server.arg("gateway");
    if (_server.hasArg("subnet")) _config.wifi_subnet = _server.arg("subnet");
    if (_server.hasArg("dns")) _config.wifi_dns = _server.arg("dns");
    
    _configMgr.save(_config);
    _server.send(200, "text/html", "Saved. Restarting...");
    delay(500);
    ESP.restart();
}

void NetworkManager::handleNotFound() {
    if (_apMode && _server.hostHeader() != WiFi.softAPIP().toString()) {
        _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        _server.send(302, "text/plain", "");
    } else {
        _server.send(404, "text/plain", "Not Found");
    }
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isAPMode() {
    return _apMode;
}
