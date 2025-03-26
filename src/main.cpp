#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <EEPROM.h>
#include <Keypad.h>

#define BLYNK_TEMPLATE_ID "TMPL6EHKH13Y6"
#define BLYNK_TEMPLATE_NAME "TEST"
#define BLYNK_AUTH_TOKEN "fUy7itGiVL3OWiMSRnrvT0GWq1q0A2rw"

#include <BlynkSimpleEsp32.h>

#define PIN_SG90 16

// WiFi credentials
const char *ssid = "DESKTOP-1F240GG 6175";
const char *pass = "0693V6,o";
const char *secret_key = "aryminh";

// Web server trên cổng 80
WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo sg90;

unsigned char index_t = 0;
unsigned char error_in = 0;

// init keypad
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
const int button = 12; // button input

char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {14, 27, 26, 25}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {33, 32, 18, 19}; // connect to the column pinouts of the keypad

int addr = 0;
char password[6] = "99999";     // Mat Khau mac dịnh
char pass_def[6] = "99999";     // Mat khau goc neu reset
char mode_changePass[6] = "#D#D#";
char mode_resetPass[6] = "#C#C#";

char data_input[6];
char new_pass1[6];
char new_pass2[6];

unsigned char in_num = 0, error_pass = 0, isMode = 0;
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void writeEpprom(char data[])
{
    unsigned char i = 0;
    for (i = 0; i < 5; i++)
    {
        EEPROM.write(i, data[i]);
    }
    EEPROM.commit();
}

void readEpprom()
{
    unsigned char i = 0;
    for (i = 0; i < 5; i++)
    {
        password[i] = EEPROM.read(i);
    }
}

void clear_data_input() // xoa gia tri nhap vao hien tai
{
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        data_input[i] = '\0';
    }
}

unsigned char isBufferdata(char data[]) // Kiem tra buffer da co gia tri chua
{
    unsigned char i = 0;
    for (i = 0; i < 5; i++)
    {
        if (data[i] == '\0')
        {
            return 0;
        }
    }
    return 1;
}

bool compareData(char data1[], char data2[]) // Kiem tra 2 cai buffer co giong nhau hay khong
{
    unsigned char i = 0;
    for (i = 0; i < 5; i++)
    {
        if (data1[i] != data2[i])
        {
            return false;
        }
    }
    return true;
}

void insertData(char data1[], char data2[]) // Gan buffer 2 cho buffer 1
{
    unsigned char i = 0;
    for (i = 0; i < 5; i++)
    {
        data1[i] = data2[i];
    }
}

void getData() // Nhan buffer tu ban phim
{
    char key = keypad.getKey(); // Doc gia tri ban phim
    if (key)
    {
        // Serial.println("key != 0");
        if (in_num == 0)
        {
            data_input[0] = key;
            lcd.setCursor(5, 1);
            lcd.print(data_input[0]);
            delay(200);
            lcd.setCursor(5, 1);
            lcd.print("*");
        }
        if (in_num == 1)
        {
            data_input[1] = key;
            lcd.setCursor(6, 1);
            lcd.print(data_input[1]);
            delay(200);
            lcd.setCursor(6, 1);
            lcd.print("*");
        }
        if (in_num == 2)
        {
            data_input[2] = key;
            lcd.setCursor(7, 1);
            lcd.print(data_input[2]);
            delay(200);
            lcd.setCursor(7, 1);
            lcd.print("*");
        }
        if (in_num == 3)
        {
            data_input[3] = key;
            lcd.setCursor(8, 1);
            lcd.print(data_input[3]);
            delay(200);
            lcd.setCursor(8, 1);
            lcd.print("*");
        }
        if (in_num == 4)
        {
            data_input[4] = key;
            lcd.setCursor(9, 1);
            lcd.print(data_input[4]);
            delay(200);
            lcd.setCursor(9, 1);
            lcd.print("*");
        }
        if (in_num == 4)
        {
            Serial.println(data_input);
            in_num = 0;
        }
        else
        {
            in_num++;
        }
    }
}

void checkPass() // kiem tra password
{
    getData();
    if (isBufferdata(data_input))
    {
        if (compareData(data_input, password)) // Dung pass
        {
            lcd.clear();
            clear_data_input();
            index_t = 3;
        }
        else if (compareData(data_input, mode_changePass))
        {
            // Serial.print("mode_changePass");
            lcd.clear();
            clear_data_input();
            index_t = 1;
        }
        else if (compareData(data_input, mode_resetPass))
        {
            // Serial.print("mode_resetPass");
            lcd.clear();
            clear_data_input();
            index_t = 2;
        }
        else
        {
            if (error_pass == 2)
            {
                clear_data_input();
                lcd.clear();
                index_t = 4;
            }
            Serial.print("Error");
            lcd.clear();
            lcd.setCursor(1, 1);
            lcd.print("WRONG PASSWORD");
            clear_data_input();
            error_pass++;
            delay(1000);
            lcd.clear();
        }
    }
}

void openDoor()
{
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("---OPENDOOR---");
    Serial.println("Door opening...");
    sg90.write(180);
    delay(5000);
    sg90.write(0);
    lcd.clear();

    Blynk.virtualWrite(V0, LOW);
    index_t = 0;
}

void error()
{
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("WRONG 3 TIME");
    lcd.setCursor(1, 1);
    lcd.print("Wait 1 minutes");
    unsigned char minute = 0;
    unsigned char i = 30;
    while (i > 0)
    {
        if (i == 1 && minute > 0)
        {
            minute--;
            i = 59;
        }
        if (i == 1 && minute == 0)
        {
            break;
        }
        i--;
        delay(1000);
    }
    lcd.clear();
    index_t = 0;
}

void changePass() {// Thay đổi mật khẩu
    lcd.setCursor(0, 0);
    lcd.print("-- Change Pass --");
    delay(3000);
    lcd.setCursor(0, 0);
    lcd.print("--- New Pass ---");
    
    while (1) {
        getData();
        if (isBufferdata(data_input))
        {
            insertData(new_pass1, data_input);
            clear_data_input();
            break;
        }
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("---- AGAIN ----");
    while (1) {
        getData();
        if (isBufferdata(data_input))
        {
            insertData(new_pass2, data_input);
            clear_data_input();
            break;
        }
    }
    delay(1000);
    if (!compareData(new_pass1, new_pass2)) { // Kiểm tra nhập lại mật khẩu có khớp không
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("-- Mismatched --");
        delay(1000);
        lcd.clear();
        index_t = 0;
        return;
    }

    else if (compareData(new_pass2, mode_resetPass) || compareData(new_pass2, mode_changePass))  {// Kiểm tra mật khẩu mới có trùng với pass_def không
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("- Invalid Pass -");
        lcd.setCursor(0, 1);
        lcd.print("   Try Again!");
        delay(2000);
        lcd.clear();
        index_t = 0;
        return;
    }
    else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("--- Success ---");
        delay(1000);
        writeEpprom(new_pass2);  // Lưu mật khẩu mới vào EEPROM
        insertData(password, new_pass2);
    }
    lcd.clear();
    index_t = 0;
}


void resetPass()
{
    unsigned char choise = 0;
    // Serial.println("Pass reset");
    lcd.setCursor(0, 0);
    lcd.print("---Reset Pass---");
    getData();
    if (isBufferdata(data_input))
    {
        if (compareData(data_input, password))
        {
            lcd.clear();
            clear_data_input();
            while (1)
            {
                lcd.setCursor(0, 0);
                lcd.print("---Reset Pass---");
                char key = keypad.getKey();
                if (choise == 0)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(">");
                    lcd.setCursor(2, 1);
                    lcd.print("YES");
                    lcd.setCursor(9, 1);
                    lcd.print(" ");
                    lcd.setCursor(11, 1);
                    lcd.print("NO");
                }
                if (choise == 1)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(" ");
                    lcd.setCursor(2, 1);
                    lcd.print("YES");
                    lcd.setCursor(9, 1);
                    lcd.print(">");
                    lcd.setCursor(11, 1);
                    lcd.print("NO");
                }
                if (key == 'D')
                {
                    if (choise == 1)
                    {
                        choise = 0;
                    }
                    else
                    {
                        choise++;
                    }
                }
                if (key == '#' && choise == 0)
                {
                    lcd.clear();
                    delay(1000);
                    writeEpprom(pass_def);
                    insertData(password, pass_def);
                    lcd.setCursor(0, 0);
                    lcd.print("---Reset ok---");
                    delay(1000);
                    lcd.clear();
                    break;
                }
                if (key == '#' && choise == 1)
                {
                    lcd.clear();
                    break;
                }
            }
            index_t = 0;
        }
        else
        {
            index_t = 0;
            lcd.clear();
        }
    }
}

void handleOpenDoor()
{
    if (server.hasArg("key") && server.arg("key") == secret_key)
    {
        openDoor();
        server.send(200, "text/plain", "Door opened");
    }
    else
    {
        server.send(403, "text/plain", "Forbidden");
    }
}

void setup()
{
    Serial.begin(9600);
    WiFi.begin(ssid, pass);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("ESP32 IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nFailed to connect to WiFi");
    }

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    sg90.setPeriodHertz(50);
    sg90.attach(PIN_SG90, 500, 2400);
    pinMode(button, INPUT_PULLUP);
    lcd.init();
    lcd.backlight();
    lcd.print("   SYSTEM INIT   ");
    delay(2000);
    lcd.clear();

    server.on("/open", HTTP_GET, handleOpenDoor);
    server.begin();

    Serial.print("PASSWORK: ");
    Serial.println(password);
}

void loop()
{
    server.handleClient();

    Blynk.run();
    lcd.setCursor(1, 0);
    lcd.print("Enter Password");
    checkPass();
    if (digitalRead(button) == 0) {
        openDoor();
    }
    while (index_t == 1)
    {
        String newPassword = "";
        changePass(); 
        for (int i = 1; i < 5; i++) {
            newPassword += (char)EEPROM.read(i);
        }
        Blynk.logEvent("NOTIFICATION", "New password: " + newPassword);
        error_pass = 0;
    }

    while (index_t == 2)
    {
        resetPass();
        Blynk.logEvent("NOTIFICATION", "Password: 99999");
        error_pass = 0;
    }

    while (index_t == 3)
    {
        Blynk.logEvent("NOTIFICATION", "Opened door.");
        openDoor();
        error_pass = 0;
    }

    while (index_t == 4)
    {
        Blynk.logEvent("WARNING", "Wrong password.");
        error();
        error_pass = 0;
    }
}

BLYNK_WRITE(V0)
{
    int pinValue = param.asInt();
    if (pinValue == 1)
    {
        index_t = 3;
    }
}

BLYNK_WRITE(V1) {
    int changepass = param.asInt();
    if (changepass == 1) {
        index_t = 1;
    }
}

BLYNK_WRITE(V2) {
    int resetpass = param.asInt();
    if (resetpass == 1) {
        index_t = 2;
    }
}
BLYNK_WRITE(V3) {
    int posittion = param.asInt();
    sg90.write(posittion);
}
