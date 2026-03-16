#if defined(FLOPPER_DISPLAYTEST)

#include <Arduino.h>
#include <TFT_eSPI.h>

static TFT_eSPI tft;

static void banner(uint8_t rot)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("ST7789 TEST", 10, 10);
    tft.setTextSize(1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("240x240  SPI 40MHz", 10, 36);
    tft.drawString("SCLK=18 MOSI=19 DC=32 RST=33 CS=GND", 10, 50);
    tft.drawString((String("rotation=") + rot).c_str(), 10, 64);
}

static void solid(uint16_t c, const char *name)
{
    tft.fillScreen(c);
    tft.setTextColor(TFT_BLACK, c);
    tft.setTextSize(2);
    tft.drawString(name, 10, 10);
    delay(700);
}

static void colorBars()
{
    static const uint16_t cols[] = {
        TFT_WHITE, TFT_YELLOW, TFT_CYAN, TFT_GREEN, TFT_MAGENTA, TFT_RED, TFT_BLUE, TFT_BLACK};
    const int w = tft.width();
    const int h = tft.height();
    const int barW = (w + 7) / 8;
    for (int i = 0; i < 8; i++)
    {
        tft.fillRect(i * barW, 0, barW, h, cols[i]);
    }
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("COLOR BARS", 10, 10);
    delay(900);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println("[DISPLAYTEST] ST7789 TFT_eSPI smoke test");
    Serial.println("[DISPLAYTEST] Expected wiring: SCLK=18 MOSI=19 DC=32 RST=33 CS=GND (or none)");
    Serial.println("[DISPLAYTEST] Backlight: tie BL/LED to 3V3 (or drive high)");

    tft.init();
    tft.setRotation(0);
    banner(0);
    delay(800);

    solid(TFT_WHITE, "WHITE");
    solid(TFT_RED, "RED");
    solid(TFT_GREEN, "GREEN");
    solid(TFT_BLUE, "BLUE");
    colorBars();

    tft.setRotation(1);
    banner(1);
}

void loop()
{
    static uint32_t last = 0;
    static uint16_t x = 0;
    static uint8_t rot = 1;

    const uint32_t now = millis();
    if (now - last < 50)
        return;
    last = now;

    const int w = tft.width();
    const int h = tft.height();
    tft.fillRect(0, h - 30, w, 30, TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString((String("ms ") + now).c_str(), 10, h - 24);

    tft.fillRect(x, h - 12, 12, 12, TFT_BLACK);
    x = (x + 4) % (uint16_t)(w - 12);
    tft.fillRect(x, h - 12, 12, 12, TFT_CYAN);

    if ((now % 5000) < 60)
    {
        rot = (rot + 1) % 4;
        tft.setRotation(rot);
        banner(rot);
    }
}

#endif
