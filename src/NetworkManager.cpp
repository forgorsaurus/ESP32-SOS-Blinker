#include "NetworkManager.h"
#include "html_pages.h"
#include "definitions.h"
#include <Update.h>

NetworkManager::NetworkManager(ConfigManager& configMgr) 
    : _configMgr(configMgr), _server(80), _apMode(false), _lastWifiCheck(0) {
}

void NetworkManager::begin() {
    _config = _configMgr.load();
    
    pinMode(PIN_BTN_CONFIG, INPUT_PULLUP);
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);
    
    // Initial check: if no SSID configured, force AP
    if (_config.wifi_ssid.length() == 0) {
        startAP();
    } else {
        startSTA();
    }
    
    setupWebServer();
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
        
        // Reconnect logic
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
    _apMode = true;
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
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
            _server.send(200, "text/plain", (Update.hasError()) ? "Update Failed" : "Update Success! Rebooting...");
            delay(1000);
            ESP.restart();
        },
        [this]() {
            HTTPUpload& upload = _server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    // Error
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    // Error
                }
                // Fast blink for activity
                digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    // Success
                } else {
                    // Error
                }
            }
        }
    );
}

void NetworkManager::handleRoot() {
    String html = FPSTR(PAGE_HEADER);
    
    html += "<div class='card'><h2>Available Networks</h2><div class='scan-results'>";
    int n = WiFi.scanNetworks();
    if (n == 0) {
        html += "<div class='scan-item'>No networks found</div>";
    } else if (n < 0) {
        html += "<div class='scan-item'>Scan failed</div>";
    } else {
        for (int i = 0; i < n; ++i) {
            html += "<div class='scan-item' onclick=\"selectNetwork('" + WiFi.SSID(i) + "')\">";
            html += "<strong>" + WiFi.SSID(i) + "</strong> (" + String(WiFi.RSSI(i)) + " dBm)</div>";
        }
    }
    html += "</div></div>";
    
    html += "<div class='card'><h2>WiFi Config</h2><form action='/save' method='POST'>";
    html += "<label>SSID:</label><input type='text' id='ssid' name='ssid' value='" + _config.wifi_ssid + "'>";
    html += "<label>Password:</label><input type='password' id='pass' name='pass' value='" + _config.wifi_pass + "'>";
    html += "<label><input type='checkbox' name='dhcp' " + String(_config.wifi_dhcp ? "checked" : "") + "> Use DHCP</label>";
    html += "<label>IP:</label><input type='text' name='ip' value='" + _config.wifi_ip + "'>";
    html += "<label>Gateway:</label><input type='text' name='gateway' value='" + _config.wifi_gateway + "'>";
    html += "<label>Subnet:</label><input type='text' name='subnet' value='" + _config.wifi_subnet + "'>";
    html += "<label>DNS:</label><input type='text' name='dns' value='" + _config.wifi_dns + "'>";
    html += "<button type='submit'>Save & Connect</button></form></div>";
    
    html += "<div class='card'><h2>Update</h2><form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='update'><button type='submit'>Upload</button></form></div>";
    
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
