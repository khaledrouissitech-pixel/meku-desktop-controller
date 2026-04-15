#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int joyX = A0;
int joyY = A1;

int state = 0;
int menuIndex = 0;

int volume = 50;

int led1 = 9;
int led2 = 10;

int brightness1 = 128;
int brightness2 = 128;

int cpu = 0;
int ram = 0;
unsigned long lastStatsUpdate = 0;

bool moved = false;

unsigned long lastRepeat = 0;
int repeatDelay = 80;

byte bar0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte bar1[8] = {16, 16, 16, 16, 16, 16, 16, 16};
byte bar2[8] = {24, 24, 24, 24, 24, 24, 24, 24};
byte bar3[8] = {28, 28, 28, 28, 28, 28, 28, 28};
byte bar4[8] = {30, 30, 30, 30, 30, 30, 30, 30};
byte bar5[8] = {31, 31, 31, 31, 31, 31, 31, 31};

void setup()
{
    Serial.begin(9600);

    lcd.init();
    lcd.backlight();

    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);

    analogWrite(led1, brightness1);
    analogWrite(led2, brightness2);

    lcd.createChar(0, bar0);
    lcd.createChar(1, bar1);
    lcd.createChar(2, bar2);
    lcd.createChar(3, bar3);
    lcd.createChar(4, bar4);
    lcd.createChar(5, bar5);

    drawMain();
}

void loop()
{
    readSerial();

    int x = analogRead(joyX);
    int y = analogRead(joyY);

    if (state == 7)
    {
        if (millis() - lastStatsUpdate > 1000)
        {
            drawStats();
            lastStatsUpdate = millis();
        }

        if (!moved && x < 300)
        {
            back();
            moved = true;
        }

        if (x > 400 && x < 600)
            moved = false;

        return;
    }

    if (state == 3 || state == 5 || state == 6)
    {
        if (millis() - lastRepeat > repeatDelay)
        {
            if (y < 300)
            {
                up();
                lastRepeat = millis();
            }

            if (y > 700)
            {
                down();
                lastRepeat = millis();
            }
        }

        if (!moved && x < 300)
        {
            back();
            moved = true;
        }

        if (x > 400 && x < 600)
            moved = false;

        static int lastVol = -1;

        if (state == 3 && volume != lastVol)
        {
            drawVolume();
            lastVol = volume;
        }

        return;
    }

    if (!moved)
    {
        if (y < 300)
        {
            up();
            moved = true;
        }
        else if (y > 700)
        {
            down();
            moved = true;
        }
        else if (x > 700)
        {
            enter();
            moved = true;
        }
        else if (x < 300)
        {
            back();
            moved = true;
        }
    }

    if (x > 400 && x < 600 && y > 400 && y < 600)
    {
        moved = false;
    }
}

void readSerial()
{
    while (Serial.available())
    {
        String line = Serial.readStringUntil('\n');

        if (line.startsWith("CPU:"))
            cpu = line.substring(4).toInt();
        if (line.startsWith("RAM:"))
            ram = line.substring(4).toInt();
        if (line.startsWith("VOL:"))
            volume = line.substring(4).toInt();
    }
}

void up()
{
    if (state == 0 || state == 1 || state == 4)
    {
        menuIndex--;
        redraw();
    }

    if (state == 3)
    {
        Serial.println("VOLUP");
    }

    if (state == 5)
    {
        brightness1 += 5;
        if (brightness1 > 255)
            brightness1 = 255;
        analogWrite(led1, brightness1);
        drawLight1();
    }

    if (state == 6)
    {
        brightness2 += 5;
        if (brightness2 > 255)
            brightness2 = 255;
        analogWrite(led2, brightness2);
        drawLight2();
    }
}

void down()
{
    if (state == 0 || state == 1 || state == 4)
    {
        menuIndex++;
        redraw();
    }

    if (state == 3)
    {
        Serial.println("VOLDOWN");
    }

    if (state == 5)
    {
        brightness1 -= 5;
        if (brightness1 < 0)
            brightness1 = 0;
        analogWrite(led1, brightness1);
        drawLight1();
    }

    if (state == 6)
    {
        brightness2 -= 5;
        if (brightness2 < 0)
            brightness2 = 0;
        analogWrite(led2, brightness2);
        drawLight2();
    }
}

void enter()
{
    if (state == 0)
    {
        if (menuIndex == 0)
        {
            state = 1;
            menuIndex = 0;
            drawMusic();
        }
        else if (menuIndex == 1)
        {
            state = 4;
            menuIndex = 0;
            drawLights();
        }
        else if (menuIndex == 2)
        {
            state = 7;
            drawStats();
        }
    }
    else if (state == 1)
    {
        if (menuIndex == 0)
        {
            state = 2;
            lcd.clear();
            lcd.print("RIGHT: Play");
            lcd.setCursor(0, 1);
            lcd.print("LEFT: Back");
        }
        else if (menuIndex == 1)
        {
            state = 3;
            drawVolume();
        }
    }
    else if (state == 4)
    {
        if (menuIndex == 0)
        {
            state = 5;
            drawLight1();
        }
        else if (menuIndex == 1)
        {
            state = 6;
            drawLight2();
        }
    }
    else if (state == 2)
    {
        Serial.println("PLAY");
    }
}

void back()
{
    if (state == 1 || state == 4 || state == 7)
    {
        state = 0;
        menuIndex = 0;
        drawMain();
    }
    else if (state == 2 || state == 3)
    {
        state = 1;
        menuIndex = 0;
        drawMusic();
    }
    else if (state == 5 || state == 6)
    {
        state = 4;
        menuIndex = 0;
        drawLights();
    }
}

void redraw()
{
    if (state == 0)
    {
        if (menuIndex < 0)
            menuIndex = 2;
        if (menuIndex > 2)
            menuIndex = 0;
        drawMain();
    }

    if (state == 1)
    {
        if (menuIndex < 0)
            menuIndex = 1;
        if (menuIndex > 1)
            menuIndex = 0;
        drawMusic();
    }

    if (state == 4)
    {
        if (menuIndex < 0)
            menuIndex = 1;
        if (menuIndex > 1)
            menuIndex = 0;
        drawLights();
    }
}

void drawMain()
{
    lcd.clear();

    if (menuIndex == 0)
    {
        lcd.print("> Music");
        lcd.setCursor(0, 1);
        lcd.print("  Lights");
    }
    else if (menuIndex == 1)
    {
        lcd.print("> Lights");
        lcd.setCursor(0, 1);
        lcd.print("  Stats");
    }
    else
    {
        lcd.print("> Stats");
        lcd.setCursor(0, 1);
        lcd.print("  Music");
    }
}

void drawMusic()
{
    lcd.clear();
    if (menuIndex == 0)
    {
        lcd.print("> Play");
        lcd.setCursor(0, 1);
        lcd.print("  Volume");
    }
    else
    {
        lcd.print("  Play");
        lcd.setCursor(0, 1);
        lcd.print("> Volume");
    }
}

void drawLights()
{
    lcd.clear();
    if (menuIndex == 0)
    {
        lcd.print("> Light 1");
        lcd.setCursor(0, 1);
        lcd.print("  Light 2");
    }
    else
    {
        lcd.print("  Light 1");
        lcd.setCursor(0, 1);
        lcd.print("> Light 2");
    }
}

void drawStats()
{
    lcd.clear();
    lcd.print("CPU:");
    lcd.print(cpu);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("RAM:");
    lcd.print(ram);
    lcd.print("%");
}

void drawVolume()
{
    lcd.clear();
    lcd.print("Volume ");
    lcd.print(volume);
    lcd.print("%");

    drawBar(map(volume, 0, 100, 0, 80));
}

void drawLight1()
{
    lcd.clear();
    lcd.print("Light1 ");
    lcd.print(map(brightness1, 0, 255, 0, 100));
    lcd.print("%");

    drawBar(map(brightness1, 0, 255, 0, 80));
}

void drawLight2()
{
    lcd.clear();
    lcd.print("Light2 ");
    lcd.print(map(brightness2, 0, 255, 0, 100));
    lcd.print("%");

    drawBar(map(brightness2, 0, 255, 0, 80));
}

void drawBar(int total)
{
    lcd.setCursor(0, 1);

    for (int i = 0; i < 16; i++)
    {
        int lvl = total - i * 5;

        if (lvl >= 5)
            lcd.write(byte(5));
        else if (lvl > 0)
            lcd.write(byte(lvl));
        else
            lcd.write(byte(0));
    }
}