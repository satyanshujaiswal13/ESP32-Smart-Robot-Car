/*************************************************
 * ESP32 SMART ROBOT CAR
 * Part 1
 *************************************************/

#include <BluetoothSerial.h>
#include <LiquidCrystal.h>

BluetoothSerial SerialBT;

/************* LCD PINS *************/
#define LCD_RS 23
#define LCD_EN 5
#define LCD_D4 13
#define LCD_D5 12
#define LCD_D6 14
#define LCD_D7 27

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/************* MOTOR PINS *************/
#define IN1 18
#define IN2 19
#define IN3 21
#define IN4 22

/************* SENSOR PINS *************/
#define IR_PIN 17

#define TRIG_PIN 25
#define ECHO_PIN 26

/************* VARIABLES *************/
String lcdMessage = "ESP32 Robot";

long duration = 0;
float distanceCM = 0;

bool obstacle = false;

unsigned long previousLCD = 0;
unsigned long previousSensor = 0;

const unsigned long lcdInterval = 500;
const unsigned long sensorInterval = 150;

/*************************************************
                MOTOR FUNCTIONS
*************************************************/

void stopMotors()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void moveForward()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void moveBackward()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnLeft()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

/*************************************************
            FUNCTION DECLARATIONS
*************************************************/

void updateLCD();
void readSensors();
void processBluetooth();

/*************************************************
                    SETUP
*************************************************/

void setup()
{
    Serial.begin(115200);

    SerialBT.begin("ESP32_Robot_Car");

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(IR_PIN, INPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    stopMotors();

    lcd.begin(16,2);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ESP32 ROBOT");

    lcd.setCursor(0,1);
    lcd.print("Starting...");

    delay(2000);

    lcd.clear();

    Serial.println("ESP32 Robot Ready");
}

/*************************************************
                LOOP
*************************************************/

void loop()
{
    processBluetooth();

    if(millis()-previousSensor>=sensorInterval)
    {
        previousSensor=millis();
        readSensors();
    }

    if(millis()-previousLCD>=lcdInterval)
    {
        previousLCD=millis();
        updateLCD();
    }
}
/*************************************************
 * PART 2
 * Sensors + LCD Functions
 *************************************************/

/************* ULTRASONIC *************/
float getDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if(duration == 0)
    {
        return 400;
    }

    return (duration * 0.0343) / 2.0;
}

/*************************************************
            READ ALL SENSORS
*************************************************/

void readSensors()
{
    distanceCM = getDistance();

    // Most IR obstacle sensors:
    // LOW = obstacle detected
    // HIGH = no obstacle

    obstacle = (digitalRead(IR_PIN) == LOW);

    if(obstacle)
    {
        stopMotors();
    }
}

/*************************************************
                LCD DISPLAY
*************************************************/

void updateLCD()
{
    lcd.clear();

    // -------- First Line --------

    lcd.setCursor(0,0);

    if(obstacle)
    {
        lcd.print("OBJECT FOUND");
    }
    else
    {
        if(lcdMessage.length() > 16)
        {
            lcd.print(lcdMessage.substring(0,16));
        }
        else
        {
            lcd.print(lcdMessage);
        }
    }

    // -------- Second Line --------

    lcd.setCursor(0,1);

    lcd.print("D:");

    lcd.print(distanceCM,1);

    lcd.print("cm");

    if(obstacle)
    {
        lcd.print(" !");
    }
}

/*************************************************
      BLUETOOTH MOVEMENT FUNCTIONS
*************************************************/

void executeCommand(char cmd)
{
    if(obstacle)
    {
        stopMotors();
        return;
    }

    switch(cmd)
    {
        case 'F':
            moveForward();
            break;

        case 'B':
            moveBackward();
            break;

        case 'L':
            turnLeft();
            break;

        case 'R':
            turnRight();
            break;

        case 'S':
            stopMotors();
            break;
    }
}

/*************************************************
    MESSAGE EXTRACTION HELPER
*************************************************/

void setLCDMessage(String msg)
{
    msg.trim();

    if(msg.length()==0)
        return;

    lcdMessage = msg;

    Serial.print("LCD Message: ");
    Serial.println(lcdMessage);
}
/*************************************************
 * PART 3
 * Bluetooth Parser + Main Control
 *************************************************/

String btBuffer = "";

void processBluetooth()
{
    while (SerialBT.available())
    {
        char c = SerialBT.read();

        // End of command
        if (c == '\n' || c == '\r')
        {
            if (btBuffer.length() > 0)
            {
                btBuffer.trim();

                Serial.print("Received: ");
                Serial.println(btBuffer);

                // LCD Message
                if (btBuffer.startsWith("MSG:"))
                {
                    String msg = btBuffer.substring(4);
                    setLCDMessage(msg);
                }

                // Movement Commands
                else if (btBuffer.equalsIgnoreCase("F"))
                {
                    executeCommand('F');
                }

                else if (btBuffer.equalsIgnoreCase("B"))
                {
                    executeCommand('B');
                }

                else if (btBuffer.equalsIgnoreCase("L"))
                {
                    executeCommand('L');
                }

                else if (btBuffer.equalsIgnoreCase("R"))
                {
                    executeCommand('R');
                }

                else if (btBuffer.equalsIgnoreCase("S"))
                {
                    executeCommand('S');
                }

                btBuffer = "";
            }
        }
        else
        {
            btBuffer += c;
        }
    }
}