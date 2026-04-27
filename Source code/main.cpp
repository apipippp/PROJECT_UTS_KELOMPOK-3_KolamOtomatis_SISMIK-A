#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= RTC =================
RTC_DS1307 rtc; //deklarasi merk rtc dengan nama "rtc"

// ================= SERVO =================
Servo servoPakan; // deklarasi nama servo
Servo servoBuang;
Servo servoIsi;

// ================= PIN =================
const int pinPot = 34;

const int ledPakan = 25;
const int ledBuang = 26;
const int ledIsi = 27;
const int ledKeruh = 14;

const int pinServoPakan = 18;
const int pinServoBuang = 19;
const int pinServoIsi = 23;

// ================= INTERRUPT =================
const int tombolPin = 4;
volatile bool triggerPakan = false;

// ================= PARAMETER =================
int batasKeruh = 2000; // lebih dari 2000 bakal dianggap keruh
bool sedangGantiAir = false; // inisiasi awal proses ganti air yang berupa 'false' atau 'tidak'. Dipake buat logika air versi 2

// ISR
void IRAM_ATTR manualPakan() {
  triggerPakan = true;
}

// ================= FUNGSI PAKAN =================
void eksekusiPakan() {
  digitalWrite(ledPakan, HIGH); // LED ON

  servoPakan.write(90); // buka
  delay(10000);          // tunggu 5 detik
  servoPakan.write(0);  // tutup

  digitalWrite(ledPakan, LOW); // LED OFF
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  rtc.begin();

  servoPakan.attach(pinServoPakan);
  servoBuang.attach(pinServoBuang);
  servoIsi.attach(pinServoIsi);

  pinMode(ledPakan, OUTPUT);
  pinMode(ledBuang, OUTPUT);
  pinMode(ledIsi, OUTPUT);
  pinMode(ledKeruh, OUTPUT);

  pinMode(tombolPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tombolPin), manualPakan, FALLING);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal");
    while (true);
  }

  display.clearDisplay();
}

// ================= LOOP =================
void loop() {

  DateTime now = rtc.now(); //ngambil waktu real

  // ================= ADC =================
  int nilaiADC = analogRead(pinPot); //baca nilai potensio pakai ADC biar nilainya berupa analog (0-4095)

  // ================= PWM =================
  int nilaiPWM = map(nilaiADC, 0, 4095, 0, 255); // jadi nilai adc dikonversi ke pwm (0-4095 ke 0-255)
  analogWrite(ledKeruh, nilaiPWM);  //nyalain led pakai pwm
  
  // // LOGIKA AIR VERSI 1
  // if (nilaiADC > batasKeruh) {
  //   // sedangGantiAir = true;

  //   digitalWrite(ledBuang, HIGH);
  //   digitalWrite(ledIsi, HIGH);

  //   servoBuang.write(90);
  //   servoIsi.write(90);
  // } else if (nilaiADC < batasKeruh) {
  //   // sedangGantiAir = false;

  //   servoBuang.write(0);
  //   servoIsi.write(0);

  //   digitalWrite(ledBuang, LOW);
  //   digitalWrite(ledIsi, LOW);
  // }


  // LOGIKA AIR VERSI 2
  if (nilaiADC > batasKeruh) {
    sedangGantiAir = true;
  }

  if (sedangGantiAir) {
    digitalWrite(ledBuang, HIGH);
    digitalWrite(ledIsi, HIGH);

    servoBuang.write(90);
    servoIsi.write(90);

    if (nilaiADC < 1000) {
      sedangGantiAir = false;

      servoBuang.write(0);
      servoIsi.write(0);
      
      digitalWrite(ledBuang, LOW);
      digitalWrite(ledIsi, LOW);
    }
  }


  // ================= INTERRUPT =================
  if (triggerPakan) {
    eksekusiPakan(); // panggil fungsi pakan
    triggerPakan = false;
  }

  // ================= RTC SCHEDULE =================
  if ((now.hour() == 12 && now.minute() == 0) ||
      (now.hour() == 15 && now.minute() == 0) ||
      (now.hour() == 18 && now.minute() == 0) ||
      (now.hour() == 21 && now.minute() == 0) ||
      (now.hour() == 0 && now.minute() == 0) ||
      (now.hour() == 3 && now.minute() == 0) ||
      (now.hour() == 6 && now.minute() == 0) ||
      (now.hour() == 9 && now.minute() == 0)) {

    eksekusiPakan(); // panggil fungsi pakan
    delay(60000);    // biar tidak trigger berulang
  }

  // ================= OLED =================
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Jam    : ");
  display.print(now.hour());
  display.print(":");
  display.print(now.minute());

  display.setCursor(0, 10);
  display.print("ADC    : ");
  display.print(nilaiADC);

  display.setCursor(0, 20);
  display.print("PWM    : ");
  display.print(nilaiPWM);

  display.setCursor(0, 30);
  display.print("Status : ");
  display.print(sedangGantiAir ? "GANTI AIR" : "NORMAL");

  display.display();

  delay(500); // delay kecil
}