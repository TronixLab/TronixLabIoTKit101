#define BLYNK_TEMPLATE_ID           "TMPLCuxFaoAW"
#define BLYNK_DEVICE_NAME           "IOT Lab Kit"
#define BLYNK_AUTH_TOKEN            "YCgXYMXKWmpf8vJ8r4T7zAGA5hmt2Ruw"
#define BLYNK_PRINT Serial

// create GPIO variables
#define BUTTON_red    33
#define BUTTON_green  32
#define BUTTON_blue   35

#define LED_red_pin   12
#define LED_green_pin 27
#define LED_blue_pin  14
#define LED_dimmer    13

#define DHTPIN 5
#define DHTTYPE DHT11

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>

char auth[] = "YCgXYMXKWmpf8vJ8r4T7zAGA5hmt2Ruw";
char ssid[] = "PLDTHOMEFIBRTjp7K";
char pass[] = "prSantiago57!";

// Declaration of global variables
const float GAMMA = 0.7;
const float RL10 = 50;

volatile bool ledState_red = false;
volatile bool ledState_green = false;
volatile bool ledState_blue = false;

bool changeState_red = false;
bool changeState_green = false;
bool changeState_blue = false;

float temperature;
int humidity;
float lux;
int pot_val;

// define oled settings
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     64
#define SCREEN_ADDRESS    0x3C
#define OLED_RESET        4

// Interrupt Service Routines
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR Switch_LED_Red() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  // timer to debounce pushbutton
  if (interrupt_time - last_interrupt_time > 200)
  {
    // toggle light state
    portENTER_CRITICAL(&mux);
    ledState_red = !(ledState_red);
    portEXIT_CRITICAL(&mux);
  }
  last_interrupt_time = interrupt_time;
}

void IRAM_ATTR Switch_LED_Green() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  // timer to debounce pushbutton
  if (interrupt_time - last_interrupt_time > 200)
  {
    // toggle light state
    portENTER_CRITICAL(&mux);
    ledState_green = !(ledState_green);
    portEXIT_CRITICAL(&mux);
  }
  last_interrupt_time = interrupt_time;
}

void IRAM_ATTR Switch_LED_Blue() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  // timer to debounce pushbutton
  if (interrupt_time - last_interrupt_time > 200)
  {
    // toggle light state
    portENTER_CRITICAL(&mux);
    ledState_blue = !(ledState_blue);
    portEXIT_CRITICAL(&mux);
  }
  last_interrupt_time = interrupt_time;
}

// Create Blynk and sendor objects to call
WidgetLED LED_red(V7);
WidgetLED LED_blue(V8);
WidgetLED LED_green(V9);

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// This function sends Arduino's up time every second to Virtual Pin.
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.

BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();

  Blynk.setProperty(V11, "color", "#23C48E");
  Blynk.virtualWrite(V11, "Connected");
}

void dhtSensor()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
}

void luxSensor()
{
  int adc_val = analogRead(39);
  adc_val = map(adc_val, 0, 4095, 4095, 0);
  float voltage = adc_val / 4095.0 * 3.3;
  float resistance = 2000 * voltage / (1 - voltage / 3.3);
  lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  Serial.println("lux val:" + String(lux));

  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, lux);
}

void potSensor()
{
  int adc_val = analogRead(34);
  pot_val = adc_val;
  Serial.println("ADC val:" + String(adc_val));

  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V3, pot_val);
}

BLYNK_WRITE(V4)
{
  // assigning incoming value from pin V4 (LED red switch) to a variable
  int pinValue = param.asInt();

  // process received value
  ledState_red = pinValue;
}

BLYNK_WRITE(V5)
{
  // assigning incoming value from pin V5 (LED green switch) to a variable
  int pinValue = param.asInt();

  // process received value
  ledState_green = pinValue;
}

BLYNK_WRITE(V6)
{
  // assigning incoming value from pin V6 (LED blue switch) to a variable
  int pinValue = param.asInt();

  // process received value
  ledState_blue = pinValue;
}

BLYNK_WRITE(V10)
{
  // assigning incoming value from pin V10 (slider switch) to a variable
  int pinValue = param.asInt();

  // process received value
  analogWrite(LED_dimmer, pinValue);
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  // GPIO configuration
  pinMode(BUTTON_red, INPUT);
  pinMode(BUTTON_green, INPUT);
  pinMode(BUTTON_blue, INPUT);

  pinMode(LED_red_pin, OUTPUT);
  pinMode(LED_green_pin, OUTPUT);
  pinMode(LED_blue_pin, OUTPUT);

  // attached interrupt routines to respective pushbuttons
  attachInterrupt(digitalPinToInterrupt(BUTTON_red), Switch_LED_Red, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_green), Switch_LED_Green, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_blue), Switch_LED_Blue, FALLING);

  // connect to blynk server
  Blynk.begin(auth, ssid, pass);

  // initialize dht sensor module
  dht.begin();

  // check if the OLED has been initialized
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("OLED screen failed to initialize!"));
    for (;;);
  }

  // setup a function to be called every second
  timer.setInterval(3000L, dhtSensor);
  timer.setInterval(3033L, luxSensor);
  timer.setInterval(3066L, potSensor);
}

void loop()
{
  if (!Blynk.connected()) {
    // try to connect to server with default timeout
    Blynk.connect();
  }

  if (ledState_red != changeState_red) {
    if (ledState_red == HIGH) {
      digitalWrite(LED_red_pin, HIGH);
      Blynk.virtualWrite(V4, 1);
      LED_red.on();
    }
    else {
      digitalWrite(LED_red_pin, LOW);
      Blynk.virtualWrite(V4, 0);
      LED_red.off();
    }
  }
  changeState_red = ledState_red;

  if (ledState_green != changeState_green) {
    if (ledState_green == HIGH) {
      digitalWrite(LED_green_pin, HIGH);
      Blynk.virtualWrite(V5, 1);
      LED_green.on();
    }
    else {
      digitalWrite(LED_green_pin, LOW);
      Blynk.virtualWrite(V5, 0);
      LED_green.off();
    }
  }
  changeState_green = ledState_green;

  if (ledState_blue != changeState_blue) {
    if (ledState_blue == HIGH) {
      digitalWrite(LED_blue_pin, HIGH);
      Blynk.virtualWrite(V6, 1);
      LED_blue.on();
    }
    else {
      digitalWrite(LED_blue_pin, LOW);
      Blynk.virtualWrite(V6, 0);
      LED_blue.off();
    }
  }
  changeState_blue = ledState_blue;

  // print data to OLED display.
  oled.clearDisplay();                // clear display
  oled.setTextSize(1);                // text size
  oled.setTextColor(WHITE);           // text color
  oled.setCursor(0, 0);               // set position to display
  oled.print("ESP32-IOT LAB KIT 101");// next line
  oled.print("\n");                   // next line
  oled.print("\n");                   // next line

  oled.print("Temperature:");         // text to display
  oled.print(temperature);            // value to display
  oled.drawCircle(104, 17, 2, WHITE); // degrees symbol to display
  oled.print(" C");                   // unit to display
  oled.print("\n");                   // next line

  oled.print("Humidity:");            // text to display
  oled.print(humidity);               // value to display
  oled.print("%");                    // unit to display
  oled.print("\n");                   // next line

  oled.print("Light:");               // text to display
  oled.print(lux);                    // value to display
  oled.print(" lx");                  // unit to display
  oled.print("\n");                   // next line

  oled.print("Pot value:");           // text to display
  oled.print(pot_val);                // value to display
  oled.display();                     // show on OLED

  Blynk.run();
  timer.run();
}
