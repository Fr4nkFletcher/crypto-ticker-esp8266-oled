#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"
#include <ArduinoJson.h>

// Include custom images
#include "images.h"

const char *SSID = "****"; // Your SSID
const char *password = "****"; // Your password

const int TOUCH_PIN = 0; // GPIO0 on ESP8266, adjust if needed

bool connected = false;

#define flipDisplay true

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3C, 14, 12);

OLEDDisplayUi ui(&display);

const String cryptoCompare = "https://min-api.cryptocompare.com/data/pricemultifull?fsyms=";

const String exchange = "Coinbase";
const uint8_t numOfCoins = 3;
String coinNames[numOfCoins] = {"BTC", "ETH", "DOGE"};

struct cryptoCoin {
    String name;
    String price;
    String hr_percent_change;
    String day_percent_change;
};

void connectToWiFi() {
    WiFi.begin(SSID, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to SSID: " + WiFi.SSID());
}

class cryptoCoins {
    cryptoCoin coins[numOfCoins];

public:
    bool updating;

    cryptoCoins() {
        for (int x = 0; x < numOfCoins; x++) coins[x] = cryptoCoin{coinNames[x], "", "", ""};
        this->updating = false;
    }

    bool update() {
        this->updating = true;
        bool success = false;

        if (WiFi.status() != WL_CONNECTED) connectToWiFi();

        String site = cryptoCompare;
        for (int x = 0; x < numOfCoins; x++) site += coinNames[x] + ",";
        site += "&tsyms=USD&e=" + exchange;

        WiFiClientSecure client;
        client.setInsecure(); // This is not recommended for production, use proper certificate validation
        HTTPClient http;
        http.begin(client, site);
        int httpCode = http.GET();

        if (httpCode > 0) {
            success = true;
            String payload = http.getString();
            Serial.println("Payload received: " + payload); // Debug output
            http.end();

            DynamicJsonDocument filter(1024);
            for (int i = 0; i < numOfCoins; i++) {
                filter["DISPLAY"][coinNames[i]]["USD"]["PRICE"] = true;
                filter["DISPLAY"][coinNames[i]]["USD"]["CHANGEPCT24HOUR"] = true;
            }

            DynamicJsonDocument doc(2048); // Estimate size and adjust if needed
            DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

            if (error) {
                Serial.println("Parsing failed!");
                Serial.println(error.c_str());
            } else {
                for (int x = 0; x < numOfCoins; x++) {
                    this->coins[x].price = doc["DISPLAY"][coinNames[x]]["USD"]["PRICE"].as<String>();
                    this->coins[x].day_percent_change = doc["DISPLAY"][coinNames[x]]["USD"]["CHANGEPCT24HOUR"].as<String>();
                }
            }
        } else {
            Serial.println("Error on HTTP request");
            Serial.println(http.errorToString(httpCode).c_str());
            http.end();
        }

        this->updating = false;
        return success;
    }

    cryptoCoin* getCoin(int coinIndex) {
        return &this->coins[coinIndex];
    }
};

cryptoCoins crypto;

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, String(millis() / 1000));

    display->setTextAlignment(TEXT_ALIGN_LEFT);
    if (crypto.updating) display->drawString(0, 0, "Updating");
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_16); // Change the font size to a larger one

    // Draw the first line of text centered
    display->drawString(display->getWidth() / 2 + x, 10 + y, "ESP8266-OLED");

    // Draw the second line of text centered
    display->drawString(display->getWidth() / 2 + x, 30 + y, "Crypto Ticker");
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 10 + y, "BTC: " + crypto.getCoin(0)->price);
    display->drawString(0 + x, 30 + y, "24hr: " + crypto.getCoin(0)->day_percent_change + "%");
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 10 + y, "ETH: " + crypto.getCoin(1)->price);
    display->drawString(0 + x, 30 + y, "24hr: " + crypto.getCoin(1)->day_percent_change + "%");
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 10 + y, "DOGE: " + crypto.getCoin(2)->price);
    display->drawString(0 + x, 30 + y, "24hr: " + crypto.getCoin(2)->day_percent_change + "%");
}

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 16 + y, "Connected to SSID:");
    display->drawString(0 + x, 26 + y, WiFi.SSID());
}

FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5 };
int frameCount = 5;

uint8_t prevFrame = 0;

OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;

const int updateInterval = 12000; // 12 second update
const int sleepTimeout = 1000000; // 1000 second shutoff

volatile int lastButtonPush;

void checkButton() {
    if (digitalRead(TOUCH_PIN) == LOW) {
        delay(50); // debounce
        if (digitalRead(TOUCH_PIN) == LOW) {
            ui.nextFrame();
            lastButtonPush = millis();
        }
    }
}

void setup() {
    Serial.begin(115200); // Set the serial baud rate to 115200
    Serial.println();
    Serial.println();
    if (flipDisplay) display.flipScreenVertically();

    pinMode(TOUCH_PIN, INPUT_PULLUP);

    Wire.begin(14, 12);
    display.init();
    ui.setTargetFPS(30);
    ui.setActiveSymbol(activeSymbol);
    ui.setInactiveSymbol(inactiveSymbol);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, frameCount);
    ui.setOverlays(overlays, overlaysCount);
    ui.init();

    ui.setTimePerFrame(10000);
    ui.switchToFrame(prevFrame);

    Serial.println("Initialized display");

    connectToWiFi();
    crypto = cryptoCoins();
    crypto.update();
}

int cryptoUpdate = 0;

void loop() {
    int remainingTimeBudget = ui.update();

    if (remainingTimeBudget > 0) {
        checkButton();

        if (millis() - cryptoUpdate > updateInterval && !(millis() - lastButtonPush > updateInterval)) {
            cryptoUpdate = millis();
            crypto.update();
        }

        delay(remainingTimeBudget);
    }
}
