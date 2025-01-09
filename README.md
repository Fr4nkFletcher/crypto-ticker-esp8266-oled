# ESP8266 OLED Crypto Ticker

This project displays cryptocurrency prices on this [module](https://a.co/d/024ynaRp). The display shows current prices for BTC, ETH, and DOGE, along with 24-hour percentage changes.

<p align="center">
  <img src="https://github.com/Fr4nkFletcher/crypto-ticker-esp8266-oled/blob/main/images/ticker.gif" alt="Demo 1">
</p>

## Features

- Displays current prices for BTC, ETH, and DOGE.
- Shows 24-hour percentage changes for each cryptocurrency.
- Connects to WiFi to fetch the latest prices.
- Touch button to cycle through different frames.
- Flips the display vertically for correct orientation.

## Hardware Requirements
Build yourself with these materials:
- ESP8266 microcontroller
- SSD1306 OLED display (0.96 inch, 128x64)
- Touch button

## Libraries Used

- `ESP8266WiFi`
- `ESP8266HTTPClient`
- `WiFiClientSecure`
- `Wire`
- `SSD1306Wire`
- `OLEDDisplayUi`
- `ArduinoJson`

## Wiring

- OLED SDA -> GPIO 14
- OLED SCL -> GPIO 12
- Touch button -> GPIO 0

## Setup

1. Clone the repository.
2. Open the project in the Arduino IDE.
3. Select: Tools > Board > ESP8266 > NodeMCU 1.0 (ESP-12E Module)
4. Install the required libraries via the Library Manager.
5. Update the `SSID` and `password` variables with your WiFi credentials.
6. Upload the code to your ESP8266.
