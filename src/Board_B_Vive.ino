#include <Arduino.h>
#include <Wire.h>
#include "vive510.h"

// ============ UART settings (communicate with mainboard) ============
#define UART_TX_PIN 6    
#define UART_RX_PIN 5    
#define UART_BAUD 115200

HardwareSerial MainSerial(1);

// ============ Top Hat I2C setting ============
#define TOPHAT_SDA_PIN 8
#define TOPHAT_SCL_PIN 9
#define TOPHAT_I2C_ADDR 0x28
#define TOPHAT_I2C_FREQ 40000
#define TOPHAT_UPDATE_MS 500      // 2Hz update

// Top Hat state
uint8_t tophatHealth = 255;       // current blood (255 = full, 0 = death)
uint8_t wifiPacketCount = 0;      // 从主板接收的 WiFi 包计数
bool tophatConnected = false;
unsigned long lastTopHatUpdate = 0;

// ============ Vive pin ============
#define VIVE_A_PIN 38    
#define VIVE_B_PIN 39    

Vive510 viveA(VIVE_A_PIN);
Vive510 viveB(VIVE_B_PIN);

// ============ Vive data ============
int viveAX = 0, viveAY = 0;
int viveBX = 0, viveBY = 0;
bool viveAValid = false, viveBValid = false;
int viveAStatus = 0, viveBStatus = 0;

// ============ Data validity range determination parameters (site boundary)============
#define VIVE_X_MIN 2870
#define VIVE_X_MAX 5250
#define VIVE_Y_MIN 1290
#define VIVE_Y_MAX 7000

// ============ time parameters ============
#define SEND_INTERVAL_MS 20     
#define SYNC_INTERVAL_MS 500    

unsigned long lastSendTime = 0;
unsigned long lastSyncTimeA = 0;
unsigned long lastSyncTimeB = 0;

// ============ UART Receiving buffer ============
char uartRxBuffer[64];
int uartRxIndex = 0;

// ============ median filter ============
uint32_t med3filt(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t middle;
    if ((a <= b) && (a <= c)) middle = (b <= c) ? b : c;
    else if ((b <= a) && (b <= c)) middle = (a <= c) ? a : c;
    else middle = (a <= b) ? a : b;
    return middle;
}

// Filtering historical values
uint16_t oldAX1 = 0, oldAX2 = 0, oldAY1 = 0, oldAY2 = 0;
uint16_t oldBX1 = 0, oldBX2 = 0, oldBY1 = 0, oldBY2 = 0;

// ============ Check whether the Vive coordinates are within the site boundaries ============
bool isValidCoord(int x, int y) {
    return (x > VIVE_X_MIN && x < VIVE_X_MAX &&
            y > VIVE_Y_MIN && y < VIVE_Y_MAX);
}

// ============================================================
//           Top Hat I2C communication function
// ============================================================

void initTopHat() {
    Wire.begin(TOPHAT_SDA_PIN, TOPHAT_SCL_PIN, TOPHAT_I2C_FREQ);
    lastTopHatUpdate = millis();
    
    Serial.printf("[TopHat] I2C init: SDA=%d, SCL=%d, Freq=%dHz, Addr=0x%02X\n",
                  TOPHAT_SDA_PIN, TOPHAT_SCL_PIN, TOPHAT_I2C_FREQ, TOPHAT_I2C_ADDR);
    
    // connection test
    Wire.beginTransmission(TOPHAT_I2C_ADDR);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        tophatConnected = true;
        Serial.println("[TopHat] ✓ Connected");
    } else {
        tophatConnected = false;
        Serial.printf("[TopHat] ✗ Connection failed (err=%d)\n", error);
    }
}

void topHatSendPacketCount(uint8_t count) {
    Wire.beginTransmission(TOPHAT_I2C_ADDR);
    Wire.write(count);
    uint8_t result = Wire.endTransmission();
    
    if (result == 0) {
        tophatConnected = true;
        Serial.printf("[TopHat] Sent: %d packets\n", count);
    } else {
        tophatConnected = false;
        Serial.printf("[TopHat] Send error: %d\n", result);
    }
}

uint8_t topHatReadHealth() {
    uint8_t health = tophatHealth;
    
    uint8_t bytesReceived = Wire.requestFrom((uint8_t)TOPHAT_I2C_ADDR, (uint8_t)1);
    
    if (bytesReceived > 0 && Wire.available()) {
        health = Wire.read();
        tophatConnected = true;
        Serial.printf("[TopHat] Health: %d\n", health);
    } else {
        tophatConnected = false;
        Serial.println("[TopHat] No response");
    }
    
    return health;
}

void topHatUpdate() {
    unsigned long now = millis();
    
    if (now - lastTopHatUpdate >= TOPHAT_UPDATE_MS) {
        lastTopHatUpdate = now;
        
        // 1. Send the WiFi packet count to Top Hat
        topHatSendPacketCount(wifiPacketCount);
        
        // 2. read the blood
        tophatHealth = topHatReadHealth();
        
        // 3. Reset count (sent to Top Hat)
        wifiPacketCount = 0;
        
        // 4. check if dead
        if (tophatHealth == 0) {
            Serial.println("[TopHat] ⚠️ ROBOT DEAD!");
        }
    }
}

// ============================================================
//           UART communication function
// ============================================================

// Parse the data received from the mainboard
// Format: $PKT,<count>\n
void parseMainBoardData(char* data) {
    if (strncmp(data, "$PKT,", 5) == 0) {
        int count = atoi(data + 5);
        wifiPacketCount += count;  // Cumulative package count
        Serial.printf("[UART] Received PKT count: %d (total: %d)\n", count, wifiPacketCount);
    }
}

// Read the data sent by the motherboard
void readMainBoardUART() {
    while (MainSerial.available()) {
        char c = MainSerial.read();
        
        if (c == '\n') {
            uartRxBuffer[uartRxIndex] = '\0';
            parseMainBoardData(uartRxBuffer);
            uartRxIndex = 0;
        } else if (c != '\r') {
            if (uartRxIndex < sizeof(uartRxBuffer) - 1) {
                uartRxBuffer[uartRxIndex++] = c;
            }
        }
    }
}

// ============================================================
//           Vive read function
// ============================================================

void readViveSensors() {
    unsigned long now = millis();
    
    // ========== read Vive A ==========
    viveAStatus = viveA.status();
    
    if (viveAStatus == VIVE_RECEIVING) {
        uint16_t rawX = viveA.xCoord();
        uint16_t rawY = viveA.yCoord();
        
        // 1. median filter
        oldAX2 = oldAX1; oldAY2 = oldAY1;
        oldAX1 = rawX;   oldAY1 = rawY;
        int filteredX = med3filt(rawX, oldAX1, oldAX2);
        int filteredY = med3filt(rawY, oldAY1, oldAY2);
        
        // 2. Range determination
        if (isValidCoord(filteredX, filteredY)) {
            viveAX = filteredX;
            viveAY = filteredY;
            viveAValid = true;
        } else {
            viveAValid = false;
        }
    } else {
        viveAValid = false;
        // reconnection after signal loss
        if (now - lastSyncTimeA > SYNC_INTERVAL_MS) {
            viveA.sync(2); 
            lastSyncTimeA = now;
        }
    }
    
    // ========== read Vive B ==========
    viveBStatus = viveB.status();
    
    if (viveBStatus == VIVE_RECEIVING) {
        uint16_t rawX = viveB.xCoord();
        uint16_t rawY = viveB.yCoord();
        
        // 1. median filter
        oldBX2 = oldBX1; oldBY2 = oldBY1;
        oldBX1 = rawX;   oldBY1 = rawY;
        int filteredX = med3filt(rawX, oldBX1, oldBX2);
        int filteredY = med3filt(rawY, oldBY1, oldBY2);
        
        // 2. Range determination
        if (isValidCoord(filteredX, filteredY)) {
            viveBX = filteredX;
            viveBY = filteredY;
            viveBValid = true;
        } else {
            viveBValid = false;
        }
    } else {
        viveBValid = false;
        if (now - lastSyncTimeB > SYNC_INTERVAL_MS) {
            viveB.sync(2);
            lastSyncTimeB = now;
        }
    }
}

// ============ Send data to the mainboard (including health points) ============
// Format: $VIVE,xA,yA,validA,xB,yB,validB,health\n
void sendDataToMainBoard() {
    char buffer[80];
    snprintf(buffer, sizeof(buffer), "$VIVE,%d,%d,%d,%d,%d,%d,%d\n",
             viveAX, viveAY, viveAValid ? 1 : 0,
             viveBX, viveBY, viveBValid ? 1 : 0,
             tophatHealth);
    MainSerial.print(buffer);
}

// ============ Setup ============
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("[Board B] Vive + Top Hat Controller");
    Serial.println("========================================");
    Serial.printf("Vive Range: X(%d-%d) Y(%d-%d)\n", 
                  VIVE_X_MIN, VIVE_X_MAX, VIVE_Y_MIN, VIVE_Y_MAX);
    
    // UART initialization
    MainSerial.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("[UART] TX=%d, RX=%d, Baud=%d\n", UART_TX_PIN, UART_RX_PIN, UART_BAUD);
    
    // Top Hat initialization
    initTopHat();
    
    // Vive initialization
    viveA.begin();
    viveB.begin();
    viveA.sync(5);
    viveB.sync(5);
    lastSyncTimeA = millis();
    lastSyncTimeB = millis();
    Serial.println("[Vive] Initialized");
    
    Serial.println("\n[Board B] Ready!\n");
}

// ============ Main Loop ============
void loop() {
    unsigned long now = millis();
    
    // Read the WiFi packet count sent by the motherboard
    readMainBoardUART();
    
    // Top Hat 2Hz update
    topHatUpdate();
    
    // read Vive sensors
    readViveSensors();
    
    // send date to the mainboard (50Hz)
    if (now - lastSendTime >= SEND_INTERVAL_MS) {
        sendDataToMainBoard();
        lastSendTime = now;
    }
    
    static unsigned long lastPrint = 0;
    if (now - lastPrint >= 1000) {
        Serial.printf("[TopHat] HP:%d %s | ", tophatHealth, tophatConnected ? "✓" : "✗");
        Serial.printf("[VIVE] A:(%d,%d) %s | B:(%d,%d) %s\n",
                      viveAX, viveAY, viveAValid ? "OK" : "X",
                      viveBX, viveBY, viveBValid ? "OK" : "X");
        lastPrint = now;
    }
    
    delay(5);
}
