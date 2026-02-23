# ESP32 SOS Blinker - Functional Specification Document

## Document Information


| Field |   Value   |
| --------- | ---------- |
| Version | 1.1 |
| Status  | Draft |
| Created | 2026-02-09 |
| Updated | 2026-02-23 |

## Overview

### 1.1 Purpose

This document specifies the functional requirements for an SOS Blinker device programmed in C++ using the Arduino framework. The device uses the on-board LEDs of the Luatos Core ESP32-C3 to transmit an SOS signal in Morse code. It is also equipped with a Wi-Fi Manager to handle Wi-Fi connections without hard-coded credentials, and supports Over-The-Air (OTA) firmware updates.

### 1.2 System Context

```
            ┌─────────────────┐                   ┌────────────────────────────────────┐
            │   WiFi Router   │ ◄───────────────► │        User's Smartphone/PC        │
            │                 │  WiFi (STA Mode)  │  (Update WiFi Config & OTA Update) │
            └─────────────────┘                   └────────────────────────────────────┘
                     ▲
                     │  WiFi (Station Mode)
                     ▼
            ┌─────────────────┐                   ┌────────────────────────────────────┐
            │    ESP32 SOS    │ ◄─────────────────│        User's Smartphone/PC        │
            │   SOS Blinker   │   WiFi (AP Mode)  │    (Initial Setup & OTA Update)    │
            └────────┬────────┘                   └────────────────────────────────────┘
                     │
                     │  Light (Visual Morse Code)
                     ▼
            ┌─────────────────┐
            │    User's Eye   │
            │    (Observer)   │
            └─────────────────┘

Interface:
- Light: Luatos Core ESP32-C3's onboard LED blinking morse code pattern
- WiFi STA: Connection to home/site network and Local OTA Updates
- WiFi AP: Captive portal for initial configuration (when unconfigured)
```

### 1.3 Goals

1. Transmit the SOS Morse code signal three times using the onboard LED
2. WiFi credential can be configured via WiFi Manager
3. Support OTA firmware updates for maintenance

## 2. Hardware Requirements

### 2.1 Target Platform

| Component | Specification |
| ----------- | ------------- |
| MCU | Luatos Core ESP32-C3 |
| LED Indicator | Onboard LED D4 (GPIO 12) and D5 (GPIO 13) |
| WiFi | Built-in ESP32 WiFi |
| Flash | 4MB |
| PSRAM | none |
| Power | 5V DC via USB |

### 2.2 Network Architecture

```
┌───────────────────────────┐
│           ESP32           │
│  ┌─────────────────────┐  │
│  │    Built-in WiFi    │  │
│  │                     │  │
│  │  ┌───────────────┐  │  │
│  │  │  STA Mode     │  │  │
│  │  │  - OTA Updates│  │  │
│  │  └───────────────┘  │  │
│  │  ┌───────────────┐  │  │
│  │  │  AP Mode      │  │  │
│  │  │  - Captive    │  │  │
│  │  │    Portal     │  │  │
│  │  │  - Config UI  │  │  │
│  │  └───────────────┘  │  │
│  └──────────┬──────────┘  │
└─────────────┼─────────────┘
              │               
              ▼               
      ┌──────────────┐
      │ Wi-FI Router │
      │  (Internet)  │
      └──────────────┘
```
### 2.3 Pin Assignments (Luatos Core ESP32-C3)

#### Application Pin Assignments

| Function | GPIO | Notes |
|----------|------|-------|
| BTN_CONFIG | 9 | Config button (hold 5s for captive portal, strapping pin) |
| LED_SOS | 12 | Onboard LED D4 |
| LED_STATUS | 13 | Onboard LED D5 |

#### All GPIOs on Luatos Core ESP32-C3 Header

| GPIO | Available | Notes |
|------|-----------|-------|
| 0 | Free | UART1_TX/ADC_0/input, output, high resistance |
| 1 | Free | UART1_RX/ADC_1/input, output, high resistance |
| 2 | Free | SPI2_CK/ADC_2/input, output, high resistance |
| 3 | Free | SPI2_MOSI/ADC_3/input, output, high resistance |
| 4 | Free | I2C_SDA/ADC_4/input, output, high resistance | 
| 5 | Free | I2C_SCL/ADC_5/input, output, high resistance |
| 6 | Free | input, output, high resistance | 
| 7 | Free | SPI2_CS/input, output, high resistance | 
| 8 | Free | input, output, high resistance | 
| **9** | **Config btn** | Config button (Active Low), BOOTMODE/input |
| 10 | Free | SPI2_MISO/input, output, high resistance |
| 11 | Free | VDD_SPI/input, output, high resistance |
| **12** | **LED_SOS** | Onboard LED D4 (Active High) |
| **13** | **LED_STATUS** | Onboard LED D5 (Active High) |
| 18 | Free | USB_D-/input, output, high resistance |
| 19 | Free | USB_D+/input, output, high resistance |
| 20 | Free | UART0_RX/input, output, high resistance |
| 21 | Free | UART0_TX/input, output, high resistance |

## 3. Functional Requirements

### 3.1 SOS Signaling
- **SOS-001**: System SHALL blink LED with morse pattern (... --- ...)
- **SOS-002**: 'Dot' (.) duration SHALL be base unit (250ms)
- **SOS-003**: 'dash' (-) duration SHALL be 3 * base unit
- **SOS-004**: Gap between dots and dashes SHALL be base unit
- **SOS-005**: Gap between letters SHALL be 4 * base unit
- **SOS-006**: Gap between words SHALL be 12 * base unit

### 3.2 Network Management

#### 3.2.1 WiFi Station Mode (Internet Network)

- **WIFI-001**: System SHALL connect to configured WiFi network in STA mode
- **WIFI-002**: WiFi credentials SHALL be stored in NVS
- **WIFI-003**: System SHALL automatically reconnect on WiFi disconnect
- **WIFI-004**: System SHALL log WiFi connection status changes via serial console
- **WIFI-005**: System SHALL support WPA2/WPA3 authentication

#### 3.2.2 WiFi Access Point Mode (Configuration)

- **AP-001**: System SHALL start AP mode when no valid WiFi config exists
- **AP-002**: From STA Mode, System SHALL start AP mode when BTN_CONFIG held for 5 seconds
- **AP-003**: AP SHALL use SSID format: `SOSBLINK-ESP32-{MAC_LAST_4}`
- **AP-004**: AP SHALL use open authentication (no password) for easy initial setup
- **AP-005**: AP SHALL assign IP 192.168.1.1 to clients

### 3.3 Captive Portal

#### 3.3.1 DNS Redirect

- **CP-001**: System SHALL run DNS server in AP mode
- **CP-002**: DNS server SHALL redirect all queries to portal IP (192.168.1.1)
- **CP-003**: System SHALL respond to captive portal detection requests

#### 3.3.2 Web Interface

- **CP-004**: Portal SHALL serve responsive HTML/CSS/JS interface
- **CP-005**: Portal SHALL work without external dependencies (offline)
- **CP-006**: Portal SHALL support modern browsers (Chrome, Firefox, Safari)

#### 3.3.3 Configuration Form Fields

**WiFi Configuration:**
```
┌─────────────────────────────────────────┐
│  WiFi Configuration                     │
├─────────────────────────────────────────┤
│  Available Networks: [Scan]             │
│   ┌─────────────────────────────────┐   │
│   │ ● HomeNetwork       (-45 dBm)   │   │
│   │ ○ OfficeWiFi        (-62 dBm)   │   │
│   │ ○ GuestNetwork      (-78 dBm)   │   │
│   └─────────────────────────────────┘   │
│                                         │
│  SSID:     [____________________]       │
│  Password: [____________________] 👁    │
│                                         │
│  [ ] Use static IP                      │
│  IP:      [___.___.___.___ ]            │
│  Gateway: [___.___.___.___ ]            │
│  Subnet:  [___.___.___.___ ]            │
│  DNS:     [___.___.___.___ ]            │
│                                         │
│           [Save & Connect]              │
└─────────────────────────────────────────┘
```

### 3.4 Configuration Management

#### 3.4.1 Storage

- **CFG-001**: Configuration SHALL be stored in ESP32 NVS
- **CFG-002**: Configuration SHALL persist across reboots
- **CFG-003**: Configuration SHALL be modifiable via captive portal

#### 3.4.2 Configuration Parameters

**Device Settings:**


| Parameter     | Type   | Default      | Description                   |
| --------------- | -------- | -------------- | ------------------------------- |
| `device_name` | string | "blinker-esp32" | Device identifier at STA Mode            |

**WiFi Settings :**


| Parameter      | Type   | Default | Description               |
| ---------------- | -------- | --------- | --------------------------- |
| `wifi_ssid`    | string | ""      | WiFi network name         |
| `wifi_pass`    | string | ""      | WiFi password (encrypted) |
| `wifi_dhcp`    | bool   | true    | Use DHCP for WiFi         |
| `wifi_ip`      | string | ""      | Static IP (if DHCP off)   |
| `wifi_gateway` | string | ""      | Gateway IP                |
| `wifi_subnet`  | string | ""      | Subnet mask               |
| `wifi_dns`     | string | ""      | DNS server                |

**Access Point Settings:**


| Parameter    | Type   | Default           | Description                        |
| -------------- | -------- | ------------------- | ------------------------------------ |
| `ap_ssid`    | string | "SOSBLINK-ESP32-XXXX" | AP network name                    |
| `ap_pass`    | string | ""                | AP password (empty = open network) |
| `ap_timeout` | uint16 | 300               | AP auto-disable (seconds, 0=never) |

### 3.5 OTA (Over-The-Air) Updates

#### 3.5.1 Update Methods

- **OTA-001**: System SHALL support firmware upload via accessing the Luatos Core ESP32's IP address and uploading the firmware file via web UI

#### 3.5.2 Update Requirements

- **OTA-002**: System SHALL support rollback to previous firmware on boot failure
- **OTA-003**: System SHALL preserve configuration across updates
- **OTA-004**: System SHALL reject firmware larger than OTA partition
- **OTA-005**: System SHALL indicate update progress via web UI and serial log

#### 3.5.3 Web UI Update Page

```
┌─────────────────────────────────────────┐
│  Firmware Update                        │
├─────────────────────────────────────────┤
│                                         │
│  Current Version: 1.0.0                 │
│  Build Date: 2026-02-23                 │
│  Free Space: 1.5 MB                     │
│                                         │
│  ┌─────────────────────────────────┐    │
│  │  Select firmware file (.bin)    │    │
│  │  [Browse...]                    │    │
│  └─────────────────────────────────┘    │
│                                         │
│  ████████████░░░░░░░░░░░░ 45%           │
│  Uploading firmware...                  │
│                                         │
│  [Upload Firmware]                      │
│                                         │
│  ⚠ Do not power off during update      │
└─────────────────────────────────────────┘
```

### 3.6 Status Indication

Status LED indication is implemented using Onboard LED D5 and serial log.

| Status | LED Pattern | Serial Log |
| --- | --- | --- |
| Booting | OFF | Booting... |
| WiFi STA mode | OFF | WiFi STA mode |
| WiFi AP mode | Blinking every 2 seconds | WiFi AP mode |
| OTA update | Blinking every 250 ms | OTA update |
| OTA update failed | Blinking every 100ms for five times | OTA update failed |
| OTA update success | Blinking every 100ms for three times | OTA update success |

## 4. Non-Functional Requirements

### 4.1 Performance

| Metric               | Requirement                      |
| ---------------------- | ---------------------------------- |
| Boot time            | < 10 seconds to operational      |

### 4.2 Reliability

- **REL-001**: System SOS blinking capability SHALL keep functioning even when unconfigured or loss connection to WiFi Network

## 5. Implementation Phases

### Phase 1: SOS Blinking

- Make the LED blink in SOS pattern
- Make the pattern run three times

### Phase 2: WiFi STA/AP mode management

- WiFi STA/AP mode management
- NVS configuration storage

### Phase 3: OTA Updates

- Partition management
- Rollback support
- Progress indication

## 5. Project Structure

```
2026_ESP32Blik/
├── docs/
│   ├── esp32-blinker-fsd.md
│   └── esp32_technical_reference_manual_en.pdf
├── include/
│   ├── README
│   ├── SystemConfig.h
│   └── definitions.h
├── lib/
│   └── README
├── src/
│   ├── ConfigManager.cpp
│   ├── ConfigManager.h
│   ├── NetworkManager.cpp
│   ├── NetworkManager.h
│   ├── SOSBlinker.cpp
│   ├── SOSBlinker.h
│   ├── html_pages.h
│   └── main.cpp
├── .gitignore
└── platformio.ini
```

## 6. Dependencies

### 6.1 PlatformIO Build Configuration

The project uses PlatformIO build system with build config as follows
```
[env:airm2m_core_esp32c3]
platform = espressif32
board = airm2m_core_esp32c3
framework = arduino
monitor_speed = 115200
```

## 7. Appendices

### Reference Documents

- ESP32 Technical Reference Manual

---

## Revision History


| Version | Date       | Author | Changes                                                                 |
| --------- | ------------ | -------- | ------------------------------------------------------------------------- |
| 1.0     | 2026-02-09 | -      | Initial specification                                                   |
| 1.1     | 2026-02-23 | -      | Updated specification                                                   |
