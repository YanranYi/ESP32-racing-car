#include <WiFi.h>
#include <WebServer.h>
#include "web_interface_hybrid_v9.h"
#include <Wire.h>
#include <VL53L1X.h>
#include "field_config.h"

// ============ Vive Offset (Offset of the dual Vive sensors) ============
#define VIVE_OFFSET_X 5
#define VIVE_OFFSET_Y 5

#include "navigation.h"

// ============ WiFi settings ============
const char* ssid = "ESP32_RaceCar";
const char* password = "12345678";

WebServer server(80);

// ============ UART settings (communicate with secondary board) ============
#define UART_TX_PIN 5    // mianboard TX -> secondary board RX
#define UART_RX_PIN 6    // mainboard RX <- secondary board TX
#define UART_BAUD 115200

HardwareSerial ViveSerial(1);

// UART receive buffer
char uartBuffer[128];
int uartBufferIndex = 0;

// Vive data(receive from UART)
int viveRawAX = 0, viveRawAY = 0;
int viveRawBX = 0, viveRawBY = 0;
bool viveRawAValid = false, viveRawBValid = false;
unsigned long lastViveUpdate = 0;

// ============ Top Hat data (receive from secondary board) ============
uint8_t tophatHealth = 255;       // blood (255 = full, 0 = death)
volatile uint32_t wifiPacketCount = 0;  // WiFi packets counting(To be sent to the secondary board)
unsigned long lastPacketSendTime = 0;
#define PACKET_SEND_INTERVAL_MS 500  // 500ms send one packet counting

// ============ Pin definitions ============
#define MOTOR_A_PWM 40  // black line
#define MOTOR_A_IN1 41  // white line
#define MOTOR_A_IN2 42  // grey line

#define MOTOR_B_PWM 48  // black line
#define MOTOR_B_IN1 38  // white line
#define MOTOR_B_IN2 39  // grey line

#define ENCODER_A_A_PIN 21
#define ENCODER_A_B_PIN 47
#define ENCODER_B_A_PIN 14
#define ENCODER_B_B_PIN 13

// ============ TOF Pins ============
#define I2C_SDA 8
#define I2C_SCL 9

#define TOF_FRONT_XSHUT 10
#define TOF_LEFT1_XSHUT 11
#define TOF_LEFT2_XSHUT 12

#define TOF_FRONT_ADDR 0x30
#define TOF_LEFT1_ADDR 0x31
#define TOF_LEFT2_ADDR 0x32

// ============ PWM settings ============
#define PWM_FREQ 20000
#define PWM_RESOLUTION 10
#define PWM_MAX 1023
#define PWM_MIN 0

// ============ encoder ============
#define SLOTS_PER_REV 270

volatile unsigned long encoderCountA = 0;
volatile unsigned long encoderCountB = 0;

int encoderDirectionA = 1;
int encoderDirectionB = 1;

int lastEncoderStateA1 = LOW, lastEncoderStateA2 = LOW;
int lastEncoderStateB1 = LOW, lastEncoderStateB2 = LOW;

unsigned long lastCountA = 0;
unsigned long lastCountB = 0;
unsigned long lastCalcTime = 0;

float rpmA = 0;
float rpmB = 0;

// ============ PID controller ============
struct PIDController {
    float kp = 2.5;
    float ki = 0.6;
    float kd = 0.1;
    float targetSpeed = 0;
    float currentSpeed = 0;
    float error = 0;
    float lastError = 0;
    float integral = 0;
    int output = 0;
    float calibration = 1.0;
    float integralMax = 300.0;
};

PIDController pidA;
PIDController pidB;

// ============ TOF sensors ============
VL53L1X tofFront, tofLeft1, tofLeft2;
int tofFrontDist = 0, tofLeft1Dist = 0, tofLeft2Dist = 0;
bool tofOK = false;

// ============ wall follow parameters ============
volatile bool wallFollowEnabled = false;

const float TOF_SPACING_MM = 143.0;

float wallTargetDist = 200.0f;
float wallDistKp     = 0.15f;
float wallAngleKp    = 2.5f;
float frontObsDist   = 300.0f;
int   wallFollowSpeedPercent = 80;
int   wallFollowMaxTurnRight = 100;
int   wallFollowMaxTurnLeft = 100;

const float FRONT_PANIC_DIST = 50.0f;
const float WALL_LOST_DIST = 600.0f;

const unsigned long SEQ_EXIT_STRAIGHT_MS = 300;
const unsigned long SEQ_EXIT_TURN_MS     = 500;
const unsigned long SEQ_EXIT_PAUSE_MS    = 200;

float debugWallAngle = 0;
float debugDistError = 0;

enum WallFollowState {
    WF_NORMAL,
    WF_OBSTACLE_FRONT,
    WF_PANIC_BACKUP,
    WF_LOST_WALL,
    WF_CORNER,
    WF_EXITING
};

WallFollowState wfState = WF_NORMAL;

unsigned long sequenceStartTime = 0;
bool exitSequenceActive = false;

// ============ Feedforward parameters ============
struct FeedforwardParams {
    float kv = 8.9;
    float staticFriction = 60.0;
} ffParams;

// ============ car control variables ============
volatile int currentSpeed = 0;
volatile int currentTurn = 0;
volatile float leftTune = 1.0;
volatile float rightTune = 1.0;
volatile bool motorEnabled = false;

int leftPWM = 0, rightPWM = 0;
int leftFF = 0, rightFF = 0;
int leftCorr = 0, rightCorr = 0;

// ============ motor controller ============
void setMotorA(int pwm, int direction) {
    if (direction == 1) {
        digitalWrite(MOTOR_A_IN1, HIGH);
        digitalWrite(MOTOR_A_IN2, LOW);
    } else if (direction == -1) {
        digitalWrite(MOTOR_A_IN1, LOW);
        digitalWrite(MOTOR_A_IN2, HIGH);
    } else {
        digitalWrite(MOTOR_A_IN1, LOW);
        digitalWrite(MOTOR_A_IN2, LOW);
    }
    pwm = constrain(abs(pwm), 0, PWM_MAX);
    ledcWrite(MOTOR_A_PWM, pwm);
}

void setMotorB(int pwm, int direction) {
    direction = -direction;
    if (direction == 1) {
        digitalWrite(MOTOR_B_IN1, HIGH);
        digitalWrite(MOTOR_B_IN2, LOW);
    } else if (direction == -1) {
        digitalWrite(MOTOR_B_IN1, LOW);
        digitalWrite(MOTOR_B_IN2, HIGH);
    } else {
        digitalWrite(MOTOR_B_IN1, LOW);
        digitalWrite(MOTOR_B_IN2, LOW);
    }
    pwm = constrain(abs(pwm), 0, PWM_MAX);
    ledcWrite(MOTOR_B_PWM, pwm);
}

void stopAllMotors() {
    setMotorA(0, 0);
    setMotorB(0, 0);
    pidA.integral = pidB.integral = 0;
    pidA.targetSpeed = pidB.targetSpeed = 0;
    leftPWM = rightPWM = 0;
    leftFF = rightFF = 0;
    leftCorr = rightCorr = 0;
}

// ============ read encoder ============
void updateEncoders() {
    int a1 = digitalRead(ENCODER_A_A_PIN);
    int a2 = digitalRead(ENCODER_A_B_PIN);
    int b1 = digitalRead(ENCODER_B_A_PIN);
    int b2 = digitalRead(ENCODER_B_B_PIN);

    if (a1 != lastEncoderStateA1) {
        encoderCountA++;
        encoderDirectionA = (a1 == HIGH) ? ((a2 == LOW) ? 1 : -1) : ((a2 == HIGH) ? 1 : -1);
    }
    if (b1 != lastEncoderStateB1) {
        encoderCountB++;
        encoderDirectionB = (b1 == HIGH) ? ((b2 == LOW) ? 1 : -1) : ((b2 == HIGH) ? 1 : -1);
    }

    lastEncoderStateA1 = a1;
    lastEncoderStateA2 = a2;
    lastEncoderStateB1 = b1;
    lastEncoderStateB2 = b2;
}

void calculateSpeed() {
    unsigned long now = millis();
    float dt = (now - lastCalcTime) / 1000.0;
    if (dt < 0.01) dt = 0.1;

    long dA = encoderCountA - lastCountA;
    long dB = encoderCountB - lastCountB;

    rpmA = (dA * encoderDirectionA * 60.0) / (SLOTS_PER_REV * dt);
    rpmB = (dB * encoderDirectionB * 60.0) / (SLOTS_PER_REV * dt);

    pidA.currentSpeed = abs(rpmA);
    pidB.currentSpeed = abs(rpmB);

    lastCountA = encoderCountA;
    lastCountB = encoderCountB;
    lastCalcTime = now;
}

// ============ TOF initialization ============
void initTOF() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);

    pinMode(TOF_FRONT_XSHUT, OUTPUT);
    pinMode(TOF_LEFT1_XSHUT, OUTPUT);
    pinMode(TOF_LEFT2_XSHUT, OUTPUT);

    digitalWrite(TOF_FRONT_XSHUT, LOW);
    digitalWrite(TOF_LEFT1_XSHUT, LOW);
    digitalWrite(TOF_LEFT2_XSHUT, LOW);
    delay(20);

    digitalWrite(TOF_FRONT_XSHUT, HIGH);
    delay(20);
    if (tofFront.init()) {
        tofFront.setAddress(TOF_FRONT_ADDR);
        tofFront.setDistanceMode(VL53L1X::Short);
        tofFront.setMeasurementTimingBudget(20000);
        tofFront.startContinuous(33);
        Serial.println("[TOF] Front OK");
    } else {
        Serial.println("[TOF] Front FAILED");
    }

    digitalWrite(TOF_LEFT1_XSHUT, HIGH);
    delay(20);
    if (tofLeft1.init()) {
        tofLeft1.setAddress(TOF_LEFT1_ADDR);
        tofLeft1.setDistanceMode(VL53L1X::Short);
        tofLeft1.setMeasurementTimingBudget(20000);
        tofLeft1.startContinuous(33);
        Serial.println("[TOF] Left1 OK");
    } else {
        Serial.println("[TOF] Left1 FAILED");
    }

    digitalWrite(TOF_LEFT2_XSHUT, HIGH);
    delay(20);
    if (tofLeft2.init()) {
        tofLeft2.setAddress(TOF_LEFT2_ADDR);
        tofLeft2.setDistanceMode(VL53L1X::Short);
        tofLeft2.setMeasurementTimingBudget(20000);
        tofLeft2.startContinuous(33);
        Serial.println("[TOF] Left2 OK");
    } else {
        Serial.println("[TOF] Left2 FAILED");
    }

    tofOK = true;
}

void readTOF() {
    if (!tofOK) return;
    
    if (tofFront.dataReady()) {
        tofFrontDist = tofFront.read(false);
        if (tofFront.timeoutOccurred()) tofFrontDist = 0;
    }
    if (tofLeft1.dataReady()) {
        tofLeft1Dist = tofLeft1.read(false);
        if (tofLeft1.timeoutOccurred()) tofLeft1Dist = 0;
    }
    if (tofLeft2.dataReady()) {
        tofLeft2Dist = tofLeft2.read(false);
        if (tofLeft2.timeoutOccurred()) tofLeft2Dist = 0;
    }
}

// ============ UART initialization ============
void initUART() {
    ViveSerial.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.printf("[UART] TX=%d, RX=%d, Baud=%d\n", UART_TX_PIN, UART_RX_PIN, UART_BAUD);
}

// ============ Parse UART data ============
// Format: $VIVE,xA,yA,validA,xB,yB,validB,health\n
void parseViveData(char* data) {
    if (strncmp(data, "$VIVE,", 6) != 0) return;
    
    char* ptr = data + 6;
    int values[7];
    int idx = 0;
    
    char* token = strtok(ptr, ",");
    while (token != NULL && idx < 7) {
        values[idx++] = atoi(token);
        token = strtok(NULL, ",");
    }
    
    if (idx >= 6) {
        viveRawAX = values[0];
        viveRawAY = values[1];
        viveRawAValid = (values[2] == 1);
        viveRawBX = values[3];
        viveRawBY = values[4];
        viveRawBValid = (values[5] == 1);
        lastViveUpdate = millis();
        
        // Parse the blood volume (if there is a 7th field)
        if (idx >= 7) {
            tophatHealth = (uint8_t)values[6];
        }
        
        // Update data for the navigation system of Vive
        updateDualViveFromRaw(viveRawAX, viveRawAY, viveRawAValid,
                              viveRawBX, viveRawBY, viveRawBValid);
    }
}

// ============ Read UART data ============
void readUART() {
    while (ViveSerial.available()) {
        char c = ViveSerial.read();
        
        if (c == '\n') {
            uartBuffer[uartBufferIndex] = '\0';
            parseViveData(uartBuffer);
            uartBufferIndex = 0;
        } else if (c != '\r') {
            if (uartBufferIndex < sizeof(uartBuffer) - 1) {
                uartBuffer[uartBufferIndex++] = c;
            }
        }
    }
    
    // Check for timeout of Vive data
    if (millis() - lastViveUpdate > 1000) {
        viveRawAValid = false;
        viveRawBValid = false;
    }
}

// ============ send WiFi packets to secondary board ============
void sendPacketCountToBoard_B() {
    unsigned long now = millis();
    
    if (now - lastPacketSendTime >= PACKET_SEND_INTERVAL_MS) {
        lastPacketSendTime = now;
        
        // sending format: $PKT,<count>\n
        uint8_t countToSend = (wifiPacketCount > 255) ? 255 : (uint8_t)wifiPacketCount;
        
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "$PKT,%d\n", countToSend);
        ViveSerial.print(buffer);
        
        Serial.printf("[UART] Sent PKT: %d\n", countToSend);
        
        // Reset the count
        wifiPacketCount = 0;
    }
}

// ============ FF + PID ============
int calculateFeedforward(float targetRPM) {
    if (targetRPM < 0.1) return 0;
    float ff = ffParams.kv * targetRPM + ffParams.staticFriction;
    return constrain((int)ff, 0, PWM_MAX);
}

int calculatePID(PIDController* pid) {
    pid->error = pid->targetSpeed - pid->currentSpeed;
    pid->integral = constrain(pid->integral + pid->error, -pid->integralMax, pid->integralMax);
    float d = pid->error - pid->lastError;
    float out = pid->kp * pid->error + pid->ki * pid->integral + pid->kd * d;
    pid->lastError = pid->error;
    pid->output = (int)out;
    return pid->output;
}

// ============ wall follow ============
int calculateWallFollowTurn() {
    float d1 = (tofLeft1Dist > 30 && tofLeft1Dist < 600) ? tofLeft1Dist : 700.0f;
    float d2 = (tofLeft2Dist > 30 && tofLeft2Dist < 600) ? tofLeft2Dist : 700.0f;

    bool frontValid = (tofFrontDist > 0 && tofFrontDist < 2000);
    bool frontPanic = (frontValid && tofFrontDist < FRONT_PANIC_DIST);
    bool frontBlocked = (frontValid && tofFrontDist < frontObsDist);
    
    bool leftClose = (tofLeft1Dist > 30 && tofLeft1Dist < 100) || 
                     (tofLeft2Dist > 30 && tofLeft2Dist < 100);

    bool headOut = (d1 >= 999.0); 
    bool tailIn = (d2 < WALL_LOST_DIST);
    bool isExitingCorner = (headOut && tailIn);

    if (exitSequenceActive) {
        wfState = WF_EXITING;
        unsigned long elapsed = millis() - sequenceStartTime;
        
        if (elapsed < SEQ_EXIT_STRAIGHT_MS) {
            debugWallAngle = 0; debugDistError = 0;
            return 0;
        } else if (elapsed < SEQ_EXIT_STRAIGHT_MS + SEQ_EXIT_TURN_MS) {
            debugWallAngle = 0; debugDistError = 0;
            return -(int)(wallFollowMaxTurnLeft * 0.8);
        } else if (elapsed < SEQ_EXIT_STRAIGHT_MS + SEQ_EXIT_TURN_MS + SEQ_EXIT_PAUSE_MS) {
            debugWallAngle = 0; debugDistError = 0;
            return 0;
        } else {
            exitSequenceActive = false;
        }
        
        if (exitSequenceActive) return 0;
    }
    
    if (isExitingCorner && !exitSequenceActive) {
        exitSequenceActive = true;
        sequenceStartTime = millis();
        wfState = WF_EXITING;
        return 0;
    }
    
    if (frontPanic) {
        wfState = WF_PANIC_BACKUP;
        debugWallAngle = 0; debugDistError = 0;
        return 0;
    }
    
    if (frontBlocked && leftClose) {
        wfState = WF_CORNER;
        debugWallAngle = 0; debugDistError = 0;
        return 0;
    }
    
    if (frontBlocked) {
        wfState = WF_OBSTACLE_FRONT;
        debugWallAngle = 0; debugDistError = 0;
        return 0;
    }
    
    if (d1 >= 999.0 && d2 >= 999.0) {
        wfState = WF_LOST_WALL;
        debugWallAngle = 0; 
        debugDistError = 0;
        return -(int)(wallFollowMaxTurnLeft * 0.7);
    }

    wfState = WF_NORMAL;
    
    float diff = d1 - d2;
    float wallAngleRad = atan2(diff, TOF_SPACING_MM);
    float wallAngleDeg = wallAngleRad * 180.0 / PI;
    
    float distError = wallTargetDist - d1;
    
    debugWallAngle = wallAngleDeg;
    debugDistError = distError;
    
    float turn = (wallDistKp * distError) - (wallAngleKp * wallAngleDeg);
    
    return constrain((int)turn, -wallFollowMaxTurnLeft, wallFollowMaxTurnRight);
}

// ============ hybrid motor control ============
void hybridMotorControl() {
    if (!motorEnabled && !wallFollowEnabled && !isNavActive()) {
        stopAllMotors();
        return;
    }

    int speedCmd = currentSpeed;
    int turnCmd = currentTurn;
    int dirCmd = 1;

    // control priority
    if (isNavActive()) {
        if (navState == NAV_CLIMBING_RAMP || navState == NAV_WALL_FOLLOW) {
            if (tofOK) {
                turnCmd = calculateWallFollowTurn();
                
                switch (wfState) {
                    case WF_PANIC_BACKUP:
                        speedCmd = 0;
                        turnCmd = 150;
                        break;
                    case WF_OBSTACLE_FRONT:
                    case WF_CORNER:
                        speedCmd = 30;
                        turnCmd = 100;
                        break;
                    case WF_EXITING:
                        if (millis() - sequenceStartTime < SEQ_EXIT_STRAIGHT_MS) {
                            speedCmd = navSpeedRamp; turnCmd = 0;
                        } else {
                            speedCmd = (int)(navSpeedRamp * 0.7); turnCmd = 0;
                        }
                        break;
                    case WF_LOST_WALL:
                        speedCmd = (int)(navSpeedRamp * 0.7);
                        break;
                    default:
                        speedCmd = (navState == NAV_CLIMBING_RAMP) ? navSpeedRamp : wallFollowSpeedPercent;
                        break;
                }
            }
        }
        else if (navState == NAV_TURN_TO_BUTTON) {
            speedCmd = 0;
            turnCmd = -navTurnSpeed;
        }
        else {
            speedCmd = navSpeedCmd;
            turnCmd = navTurnCmd;
            dirCmd = navDirCmd;
        }
        
        motorEnabled = true;
    }
    else if (wallFollowEnabled && tofOK) {
        turnCmd = calculateWallFollowTurn();
        
        switch (wfState) {
            case WF_PANIC_BACKUP:
                speedCmd = 0;
                turnCmd = 150;
                break;
            case WF_OBSTACLE_FRONT:
            case WF_CORNER:
                speedCmd = 30;
                turnCmd = 100;
                break;
            case WF_EXITING:
                if (millis() - sequenceStartTime < SEQ_EXIT_STRAIGHT_MS) {
                    speedCmd = wallFollowSpeedPercent; turnCmd = 0;
                } else {
                    speedCmd = (int)(wallFollowSpeedPercent * 0.7); turnCmd = 0;
                }
                break;
            case WF_LOST_WALL:
                speedCmd = (int)(wallFollowSpeedPercent * 0.7);
                break;
            default:
                speedCmd = wallFollowSpeedPercent;
                break;
        }
        
        if (tofFrontDist == 0) speedCmd = min(speedCmd, 40);
        motorEnabled = true;
    }

    if (speedCmd == 0 && turnCmd == 0) {
        stopAllMotors();
        return;
    }

    float maxRPM = 108.0;
    
    float actualSpeed = speedCmd;
    if (dirCmd < 0) {
        actualSpeed = -speedCmd;
    }
    
    float throttleRPM = (actualSpeed / 100.0) * maxRPM;
    float steerRPM = (turnCmd / 100.0) * maxRPM * 1.2;

    float leftTargetSigned = throttleRPM + steerRPM;
    float rightTargetSigned = throttleRPM - steerRPM;

    leftTargetSigned *= leftTune;
    rightTargetSigned *= rightTune;

    int dirA = (leftTargetSigned >= 0) ? 1 : -1;
    int dirB = (rightTargetSigned >= 0) ? 1 : -1;
    
    pidA.targetSpeed = abs(leftTargetSigned);
    pidB.targetSpeed = abs(rightTargetSigned);

    leftFF = calculateFeedforward(pidA.targetSpeed);
    rightFF = calculateFeedforward(pidB.targetSpeed);
    leftCorr = calculatePID(&pidA);
    rightCorr = calculatePID(&pidB);

    leftPWM = constrain(leftFF + leftCorr, 0, PWM_MAX);
    rightPWM = constrain(rightFF + rightCorr, 0, PWM_MAX);

    setMotorA(leftPWM, dirA);
    setMotorB(rightPWM, dirB);
}

// ============ Web server ============
void handleRoot() {
    wifiPacketCount++;
    server.send_P(200, "text/html", INDEX_HTML_HYBRID);
}

void handleSetMotor() {
    wifiPacketCount++;
    
    if (server.hasArg("speed") && server.hasArg("forwardBackward") && server.hasArg("turnRate")) {
        int speed = server.arg("speed").toInt();
        int turn = server.arg("turnRate").toInt();
        String dir = server.arg("forwardBackward");

        wallFollowEnabled = false;
        navStop();
        motorEnabled = true;
        currentSpeed = (dir == "Backward") ? -speed : speed;
        currentTurn = turn;

        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

void handleSetPID() {
    wifiPacketCount++;
    
    if (server.hasArg("kpL")) pidA.kp = server.arg("kpL").toFloat();
    if (server.hasArg("kiL")) pidA.ki = server.arg("kiL").toFloat();
    if (server.hasArg("kdL")) pidA.kd = server.arg("kdL").toFloat();
    if (server.hasArg("kpR")) pidB.kp = server.arg("kpR").toFloat();
    if (server.hasArg("kiR")) pidB.ki = server.arg("kiR").toFloat();
    if (server.hasArg("kdR")) pidB.kd = server.arg("kdR").toFloat();

    pidA.integral = pidB.integral = 0;
    server.send(200, "text/plain", "PID Updated");
}

void handleSetFF() {
    wifiPacketCount++;
    
    if (server.hasArg("kv")) ffParams.kv = server.arg("kv").toFloat();
    if (server.hasArg("static")) ffParams.staticFriction = server.arg("static").toFloat();
    server.send(200, "text/plain", "FF Updated");
}

void handleSetWheelTuning() {
    wifiPacketCount++;
    
    if (server.hasArg("leftTune")) leftTune = constrain(server.arg("leftTune").toFloat(), 0.5, 1.5);
    if (server.hasArg("rightTune")) rightTune = constrain(server.arg("rightTune").toFloat(), 0.5, 1.5);
    server.send(200, "text/plain", "Wheel Updated");
}

void handleStartWallFollow() {
    wifiPacketCount++;
    
    navStop();
    wallFollowEnabled = true;
    motorEnabled = true;
    currentSpeed = wallFollowSpeedPercent;
    server.send(200, "text/plain", "OK");
}

void handleStopWallFollow() {
    wifiPacketCount++;
    
    wallFollowEnabled = false;
    motorEnabled = false;
    currentSpeed = 0;
    currentTurn = 0;
    exitSequenceActive = false;
    wfState = WF_NORMAL;
    stopAllMotors();
    server.send(200, "text/plain", "OK");
}

void handleSetTofParams() {
    wifiPacketCount++;
    
    if (server.hasArg("speed")) wallFollowSpeedPercent = server.arg("speed").toInt();
    if (server.hasArg("side")) wallTargetDist = server.arg("side").toFloat() * 10.0;
    if (server.hasArg("front")) frontObsDist = server.arg("front").toFloat() * 10.0;
    if (server.hasArg("maxTurnR")) wallFollowMaxTurnRight = server.arg("maxTurnR").toInt();
    if (server.hasArg("maxTurnL")) wallFollowMaxTurnLeft = server.arg("maxTurnL").toInt();
    if (server.hasArg("distKp")) wallDistKp = server.arg("distKp").toFloat();
    if (server.hasArg("angleKp")) wallAngleKp = server.arg("angleKp").toFloat();
    server.send(200, "text/plain", "OK");
}

void handleSetWallFollowPID() {
    wifiPacketCount++;
    
    if (server.hasArg("distKp")) wallDistKp = server.arg("distKp").toFloat();
    if (server.hasArg("angleKp")) wallAngleKp = server.arg("angleKp").toFloat();
    server.send(200, "text/plain", "OK");
}

void handleEmergencyStop() {
    wifiPacketCount++;
    
    motorEnabled = false;
    wallFollowEnabled = false;
    navStop();
    currentSpeed = 0;
    currentTurn = 0;
    stopAllMotors();
    server.send(200, "text/plain", "STOP");
}

// ========== Navigation processor ==========
void handleSetTeam() {
    wifiPacketCount++;
    
    if (server.hasArg("team")) {
        int team = server.arg("team").toInt();
        setTeam(team);
        Serial.printf("Team set to: %s\n", team == TEAM_RED ? "RED" : "BLUE");
    }
    server.send(200, "text/plain", "OK");
}

void handleTaskCaptureLow() {
    wifiPacketCount++;
    
    wallFollowEnabled = false;
    taskCaptureLowTower();
    server.send(200, "text/plain", "OK");
}

void handleTaskCaptureHigh() {
    wifiPacketCount++;
    
    wallFollowEnabled = false;
    taskCaptureHighTower();
    server.send(200, "text/plain", "OK");
}

void handleTaskAttackNexus() {
    wifiPacketCount++;
    
    wallFollowEnabled = false;
    taskAttackNexus();
    server.send(200, "text/plain", "OK");
}

void handleTaskWallFollow() {
    wifiPacketCount++;
    
    navState = NAV_WALL_FOLLOW;
    currentTask = TASK_WALL_FOLLOW_FULL;
    wallFollowEnabled = true;
    motorEnabled = true;
    currentSpeed = wallFollowSpeedPercent;
    server.send(200, "text/plain", "OK");
}

void handleStopNavigation() {
    wifiPacketCount++;
    
    navStop();
    wallFollowEnabled = false;
    motorEnabled = false;
    currentSpeed = 0;
    currentTurn = 0;
    exitSequenceActive = false;
    wfState = WF_NORMAL;
    stopAllMotors();
    server.send(200, "text/plain", "OK");
}

void handleGotoSingle() {
    wifiPacketCount++;
    
    if (server.hasArg("x") && server.hasArg("y")) {
        int x = server.arg("x").toInt();
        int y = server.arg("y").toInt();
        wallFollowEnabled = false;
        taskGotoSingle(x, y);
    }
    server.send(200, "text/plain", "OK");
}

void handleGotoThree() {
    wifiPacketCount++;
    
    if (server.hasArg("x1") && server.hasArg("y1") &&
        server.hasArg("x2") && server.hasArg("y2") &&
        server.hasArg("x3") && server.hasArg("y3")) {
        int x1 = server.arg("x1").toInt();
        int y1 = server.arg("y1").toInt();
        int x2 = server.arg("x2").toInt();
        int y2 = server.arg("y2").toInt();
        int x3 = server.arg("x3").toInt();
        int y3 = server.arg("y3").toInt();
        wallFollowEnabled = false;
        taskGotoThree(x1, y1, x2, y2, x3, y3);
    }
    server.send(200, "text/plain", "OK");
}

void handleGotoSmart() {
    wifiPacketCount++;
    
    if (server.hasArg("x") && server.hasArg("y")) {
        int x = server.arg("x").toInt();
        int y = server.arg("y").toInt();
        wallFollowEnabled = false;
        taskGotoSmart(x, y);
    }
    server.send(200, "text/plain", "OK");
}

// ========== state processor ==========
void handleGetStatus() {
    String json = "{";
    
    // motor state
    json += "\"leftSpeed\":" + String(rpmA, 1) + ",";
    json += "\"rightSpeed\":" + String(rpmB, 1) + ",";
    json += "\"leftTarget\":" + String(pidA.targetSpeed, 1) + ",";
    json += "\"rightTarget\":" + String(pidB.targetSpeed, 1) + ",";
    json += "\"leftError\":" + String(pidA.error, 1) + ",";
    json += "\"rightError\":" + String(pidB.error, 1) + ",";
    json += "\"leftPWM\":" + String(leftPWM) + ",";
    json += "\"rightPWM\":" + String(rightPWM) + ",";
    json += "\"leftFF\":" + String(leftFF) + ",";
    json += "\"rightFF\":" + String(rightFF) + ",";
    json += "\"leftCorr\":" + String(leftCorr) + ",";
    json += "\"rightCorr\":" + String(rightCorr) + ",";
    
    // TOF reading
    json += "\"tofFront\":" + String(tofFrontDist) + ",";
    json += "\"tofLeft1\":" + String(tofLeft1Dist) + ",";
    json += "\"tofLeft2\":" + String(tofLeft2Dist) + ",";
    
    // wall follow state
    json += "\"wfRunning\":" + String(wallFollowEnabled ? "true" : "false") + ",";
    json += "\"wallAngle\":" + String(debugWallAngle, 1) + ",";
    json += "\"distError\":" + String(debugDistError, 0) + ",";
    
    const char* stateNames[] = {"NORMAL", "OBS_FRONT", "PANIC", "LOST_WALL", "CORNER", "EXITING"};
    json += "\"wfState\":\"" + String(stateNames[wfState]) + "\",";
    
    // Vive data(After integration)
    json += "\"viveX\":" + String(vive.x) + ",";
    json += "\"viveY\":" + String(vive.y) + ",";
    json += "\"viveValid\":" + String(vive.valid ? "true" : "false") + ",";
    json += "\"viveHeading\":" + String(vive.heading * 180.0 / PI, 1) + ",";
    
    // UART original data(from Board_B)
    json += "\"uartRawAX\":" + String(viveRawAX) + ",";
    json += "\"uartRawAY\":" + String(viveRawAY) + ",";
    json += "\"uartRawAValid\":" + String(viveRawAValid ? "true" : "false") + ",";
    json += "\"uartRawBX\":" + String(viveRawBX) + ",";
    json += "\"uartRawBY\":" + String(viveRawBY) + ",";
    json += "\"uartRawBValid\":" + String(viveRawBValid ? "true" : "false") + ",";
    json += "\"uartAge\":" + String(millis() - lastViveUpdate) + ",";
    
    // Top Hat blood
    json += "\"tophatHealth\":" + String(tophatHealth) + ",";
    
    // Navigation state
    json += "\"navState\":\"" + String(getNavStateString()) + "\",";
    json += "\"navTask\":\"" + String(getTaskString()) + "\",";
    json += "\"navTargetX\":" + String(currentTarget.x) + ",";
    json += "\"navTargetY\":" + String(currentTarget.y) + ",";
    json += "\"navDistance\":" + String(debugNavDistance, 0) + ",";
    json += "\"navProgress\":" + String(getWaypointProgress()) + ",";
    json += "\"navTotal\":" + String(getTotalWaypoints()) + ",";
    json += "\"nexusAttackCount\":" + String(getNexusAttackProgress()) + ",";
    
    // control value
    json += "\"speed\":" + String(currentSpeed) + ",";
    json += "\"turn\":" + String(currentTurn) + ",";
    json += "\"motorEnabled\":" + String(motorEnabled ? "true" : "false");
    
    json += "}";
    server.send(200, "application/json", json);
}

// ============ Setup ============
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("Board A - Main Controller + Top Hat");
    Serial.println("Motor + TOF + Web + UART + TopHat HP");
    Serial.println("========================================\n");

    // motor pin
    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_IN1, OUTPUT);
    pinMode(MOTOR_B_IN2, OUTPUT);

    ledcAttach(MOTOR_A_PWM, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(MOTOR_B_PWM, PWM_FREQ, PWM_RESOLUTION);
    
    stopAllMotors();
    Serial.println("[OK] Motors configured");

    // encoder pin
    pinMode(ENCODER_A_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_A_B_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_B_PIN, INPUT_PULLUP);

    lastEncoderStateA1 = digitalRead(ENCODER_A_A_PIN);
    lastEncoderStateA2 = digitalRead(ENCODER_A_B_PIN);
    lastEncoderStateB1 = digitalRead(ENCODER_B_A_PIN);
    lastEncoderStateB2 = digitalRead(ENCODER_B_B_PIN);
    Serial.println("[OK] Encoders configured");

    // TOF sensors
    initTOF();

    // UART communication (with board B)
    initUART();

    // Navigation system
    navInit();

    // WiFi AP
    Serial.println("[INFO] Starting WiFi AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password, 10, 0, 4);
    WiFi.setSleep(false);
    delay(500);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.println("[OK] WiFi AP Started");
    Serial.printf("    SSID: %s\n", ssid);
    Serial.printf("    Password: %s\n", password);
    Serial.print("    IP: ");
    Serial.println(IP);

    // Web Server routing
    server.on("/", handleRoot);
    server.on("/setMotor", handleSetMotor);
    server.on("/setPID", handleSetPID);
    server.on("/setFF", handleSetFF);
    server.on("/setWheelTuning", handleSetWheelTuning);
    server.on("/startWallFollow", handleStartWallFollow);
    server.on("/stopWallFollow", handleStopWallFollow);
    server.on("/setTofParams", handleSetTofParams);
    server.on("/setWallFollowPID", handleSetWallFollowPID);
    server.on("/emergencyStop", handleEmergencyStop);
    server.on("/getStatus", handleGetStatus);
    
    server.on("/setTeam", handleSetTeam);
    server.on("/taskCaptureLow", handleTaskCaptureLow);
    server.on("/taskCaptureHigh", handleTaskCaptureHigh);
    server.on("/taskAttackNexus", handleTaskAttackNexus);
    server.on("/taskWallFollow", handleTaskWallFollow);
    server.on("/stopNavigation", handleStopNavigation);
    server.on("/gotoSingle", handleGotoSingle);
    server.on("/gotoThree", handleGotoThree);
    server.on("/gotoSmart", handleGotoSmart);

    server.begin();
    Serial.println("[OK] Web server started");
    
    lastCalcTime = millis();
    lastPacketSendTime = millis();
    
    Serial.println("\n========================================");
    Serial.println("System Ready!");
    Serial.printf("Connect to WiFi: %s\n", ssid);
    Serial.print("IP: ");
    Serial.println(IP);
    Serial.println("========================================\n");
}

// ============ Main Loop ============
void loop() {
    server.handleClient();
    updateEncoders();
    
    // Read UART data(Vive + blood)
    readUART();
    
    // send WiFi packets to board B (500ms)
    sendPacketCountToBoard_B();

    static unsigned long lastControlTime = 0;
    unsigned long now = millis();

    // control loop 10Hz
    if (now - lastControlTime >= 100) {
        readTOF();
        calculateSpeed();
        
        // update navigation state
        navUpdate();
        
        // hybrid motor control
        hybridMotorControl();
        
        lastControlTime = now;
    }
    
    static unsigned long lastPrint = 0;
    if (now - lastPrint >= 1000) {
        Serial.printf("[TopHat] HP:%d | ", tophatHealth);
        
        if (isNavActive()) {
            Serial.printf("[NAV] State:%s Task:%s | Vive:(%d,%d) H:%.1f° | Dist:%.0f",
                          getNavStateString(), getTaskString(),
                          vive.x, vive.y, vive.heading * 180.0 / PI,
                          debugNavDistance);
            if (currentTask == TASK_ATTACK_NEXUS) {
                Serial.printf(" | NexusAtk:%d/%d", getNexusAttackProgress(), NEXUS_ATTACK_COUNT);
            }
            Serial.println();
        } else if (wallFollowEnabled) {
            Serial.printf("[WF] TOF: F=%d L1=%d L2=%d | Angle:%.1f° | Turn:%d\n",
                          tofFrontDist, tofLeft1Dist, tofLeft2Dist,
                          debugWallAngle, currentTurn);
        } else {
            // show Vive UART state
            Serial.printf("[VIVE] A:(%d,%d)%s B:(%d,%d)%s | Age:%ldms\n",
                          viveRawAX, viveRawAY, viveRawAValid ? "✓" : "✗",
                          viveRawBX, viveRawBY, viveRawBValid ? "✓" : "✗",
                          millis() - lastViveUpdate);
        }
        
        lastPrint = now;
    }

    delayMicroseconds(500);
}
