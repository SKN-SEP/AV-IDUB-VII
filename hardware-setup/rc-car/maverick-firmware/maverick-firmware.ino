#include <Servo.h>

// Communication configuration
const uint8_t HEADER = 0xAA;
const int PACKET_SIZE = 4;
const unsigned long FAILSAFE_TIMEOUT_MS = 300; 
const unsigned long BAUD_RATE = 115200;

// Pins
const int SERVO_PIN = 5;
const int ESC_PIN = 6;

// Steering pulse widths
const int SERVO_L = 2100;
const int SERVO_N = 1500;
const int SERVO_R = 1020;

// Throttle pulse widths
const int ESC_FWD_MAX = 1675;
const int ESC_FWD_MIN = 1570;
const int ESC_STOP = 1495;
const int ESC_BWD_BRAKE = 1400;
const int ESC_BWD_MAX = 1125;
const int ESC_BWD_MIN = 1420;

// Global Variables
Servo esc, servo;
int8_t targetThrottle = 0;   // -100 to 100
int8_t targetSteering = 0;   // -100 to 100
unsigned long lastPacketTime = 0;

// State Machine Variables
enum DriveState { DRIVE_FWD, BRAKING, READY_REV, DRIVE_BWD };
DriveState currentState = DRIVE_FWD;
unsigned long stateTimer = 0;

void setup() {
    Serial.begin(BAUD_RATE);
    servo.attach(SERVO_PIN);
    esc.attach(ESC_PIN);
    
    // Initialize to neutral
    servo.writeMicroseconds(SERVO_N);
    esc.writeMicroseconds(ESC_STOP);
    lastPacketTime = millis();
}

void loop() {
    // 1. Packet Parsing (Header Hunting)
    while (Serial.available() >= PACKET_SIZE) {
        if (Serial.peek() == HEADER) {
            Serial.read();
            int8_t t = (int8_t)Serial.read();
            int8_t s = (int8_t)Serial.read();
            uint8_t crc = (uint8_t)Serial.read();

            // Checksum Validation
            if (crc == (HEADER ^ (uint8_t)t ^ (uint8_t)s)) {
                targetThrottle = t;
                targetSteering = s;
                lastPacketTime = millis();
                Serial.write('A'); // Send ACK back to Jetson
            }
        } else {
            Serial.read(); // Discard junk byte to find 0xAA
        }
    }

    // 2. Failsafe
    if (millis() - lastPacketTime > FAILSAFE_TIMEOUT_MS) {
        targetThrottle = 0;
        targetSteering = 0;
    }

    // 3. Execute Steering (Instant)
    int steeringPWM = (targetSteering < 0) 
        ? map(targetSteering, -100, 0, SERVO_L, SERVO_N) 
        : map(targetSteering, 0, 100, SERVO_N, SERVO_R);
    servo.writeMicroseconds(steeringPWM);

    // 4. Execute Throttle State Machine (Double-Tap Protection)
    updateESC();
}

void updateESC() {
    int throttlePWM;
    if (targetThrottle == 0)
        throttlePWM = ESC_STOP;
    else
        throttlePWM = (targetThrottle < 0)
        ? map(targetThrottle, -100, -1, ESC_BWD_MAX, ESC_BWD_MIN)
        : map(targetThrottle, 1, 100, ESC_FWD_MIN, ESC_FWD_MAX);

    switch (currentState) {
        case DRIVE_FWD:
            if (targetThrottle < 0) {
                esc.writeMicroseconds(ESC_BWD_BRAKE); // Pulse reverse to Brake
                stateTimer = millis();
                currentState = BRAKING;
            } else {
                esc.writeMicroseconds(throttlePWM);
            }
            break;

        case BRAKING:
            // Hold the brake for 250ms
            if (millis() - stateTimer > 250) {
                esc.writeMicroseconds(ESC_STOP);
                stateTimer = millis();
                currentState = READY_REV;
            }
            break;

        case READY_REV:
            // Must stay in Neutral for 150ms to unlock reverse
            if (millis() - stateTimer > 150) {
                if (targetThrottle < 0) 
                    currentState = DRIVE_BWD;
                else 
                    currentState = DRIVE_FWD;
            }
            break;

        case DRIVE_BWD:
            if (targetThrottle >= 0)
                currentState = DRIVE_FWD;
            else
                esc.writeMicroseconds(throttlePWM);

            break;
    }
}