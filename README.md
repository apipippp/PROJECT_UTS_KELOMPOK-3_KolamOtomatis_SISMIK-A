# Sistem Kolam Otomatis (ESP32)

## Anggota Kelompok:
  1. Afif Nur Rahman                      (H1H024016)
  2. Diva Syahita Mawarni                 (H1H024015)
  3. Hana Nur Fathiyyah	                  (H1H024017)
  4. Ardina Jihan Mariska	                (H1H024018)
  5. Difa’ Tamaya Maulidina Adz Dzikro    (H1H024019)
  6. Kaira Meilasya Nayada                (H1H024020)
  7. Alma Maida Wirastuti                 (H1H024021)


## Deskripsi Proyek

Sistem ini merupakan **otomatisasi kolam ikan berbasis ESP32** yang memiliki fitur utama:

* Pemberian pakan otomatis berdasarkan waktu (RTC)
* Pemberian pakan manual menggunakan tombol (interrupt)
* Monitoring kekeruhan air (ADC)
* Penggantian air otomatis
* Monitoring data melalui OLED

---

## Komponen yang Digunakan

* ESP32
* RTC DS1307
* OLED SSD1306 (I2C)
* 3x Servo Motor (Pakan, Buang, Isi)
* Potensiometer (sebagai simulasi sensor kekeruhan)
* LED indikator
* Push button (interrupt)

---

## Konfigurasi Pin

| Komponen         | Pin |
| ---------------- | --- |
| Potensiometer    | 34  |
| LED Pakan        | 25  |
| LED Buang        | 26  |
| LED Isi          | 27  |
| LED Keruh (PWM)  | 14  |
| Servo Pakan      | 18  |
| Servo Buang      | 19  |
| Servo Isi        | 23  |
| Tombol Interrupt | 4   |

---

## Alur Logika Sistem

### 1. Inisialisasi (Setup)

Saat sistem dinyalakan:

* Serial komunikasi dimulai
* I2C (`Wire`) diaktifkan untuk RTC & OLED
* RTC mulai membaca waktu real-time
* Servo di-attach ke pin masing-masing
* Semua LED diset sebagai OUTPUT
* Tombol diset sebagai INPUT_PULLUP
* Interrupt diaktifkan pada tombol (FALLING trigger)
* OLED diinisialisasi

---

### 2. Pembacaan Sensor (ADC → PWM)

```cpp
nilaiADC = analogRead(pinPot);
nilaiPWM = map(nilaiADC, 0, 4095, 0, 255);
analogWrite(ledKeruh, nilaiPWM);
```

**Penjelasan:**

* Nilai ADC (0–4095) dibaca dari potensiometer
* Nilai dikonversi menjadi PWM (0–255)
* Digunakan untuk mengatur intensitas LED indikator kekeruhan

---

### 3. Logika Penggantian Air (Versi 2)

#### Trigger Mulai Ganti Air

```cpp
if (nilaiADC > batasKeruh) {
  sedangGantiAir = true;
}
```

Jika nilai ADC melebihi batas (air dianggap keruh), maka sistem mulai mengganti air.

---

#### Proses Penggantian Air

```cpp
if (sedangGantiAir) {
  digitalWrite(ledBuang, HIGH);
  digitalWrite(ledIsi, HIGH);

  servoBuang.write(90);
  servoIsi.write(90);
}
```

Saat proses berlangsung:

* LED Buang & Isi menyala
* Servo Buang & Isi terbuka (menguras & mengisi air)

---

#### ⏹Stop Penggantian Air

```cpp
if (nilaiADC < 1000) {
  sedangGantiAir = false;

  servoBuang.write(0);
  servoIsi.write(0);

  digitalWrite(ledBuang, LOW);
  digitalWrite(ledIsi, LOW);
}
```

Jika air sudah cukup bersih:

* Proses dihentikan
* Servo ditutup
* LED dimatikan

---

### 4. Sistem Pemberian Pakan

#### Manual (Interrupt)

```cpp
void IRAM_ATTR manualPakan() {
  triggerPakan = true;
}
```

Saat tombol ditekan:

* Interrupt aktif
* Variabel `triggerPakan` menjadi true

```cpp
if (triggerPakan) {
  eksekusiPakan();
  triggerPakan = false;
}
```

---

#### Otomatis (RTC Schedule)

Pakan diberikan pada jam:

* 00:00
* 03:00
* 06:00
* 09:00
* 12:00
* 15:00
* 18:00
* 21:00

```cpp
if ((now.hour() == 12 && now.minute() == 0) ||
    (now.hour() == 15 && now.minute() == 0) ||
    (now.hour() == 18 && now.minute() == 0) ||
    (now.hour() == 21 && now.minute() == 0) ||
    (now.hour() == 0 && now.minute() == 0) ||
    (now.hour() == 3 && now.minute() == 0) ||
    (now.hour() == 6 && now.minute() == 0) ||
    (now.hour() == 9 && now.minute() == 0)) {

    eksekusiPakan(); 
    delay(60000);    
}
```

---

#### Mekanisme Servo Pakan

```cpp
void eksekusiPakan() {
  digitalWrite(ledPakan, HIGH);

  servoPakan.write(90);
  delay(10000);
  servoPakan.write(0);

  digitalWrite(ledPakan, LOW);
}
```

Alur:

* LED pakan menyala
* Servo membuka (mengeluarkan pakan)
* Delay 10 detik
* Servo menutup kembali
* LED mati

---

### 5. Tampilan OLED

OLED menampilkan:

```cpp
Jam    : HH:MM
ADC    : nilaiADC
PWM    : nilaiPWM
Status : NORMAL / GANTI AIR
```

Status:

* **NORMAL** → kondisi air bersih
* **GANTI AIR** → sedang proses penggantian

---

### 6. Loop Utama

Setiap siklus loop:

1. Membaca waktu dari RTC
2. Membaca nilai ADC (kekeruhan)
3. Mengatur LED PWM
4. Menjalankan logika penggantian air
5. Mengecek interrupt tombol
6. Mengecek jadwal pakan otomatis
7. Update tampilan OLED
8. Delay 500ms

---

## Catatan

* RTC harus disetting terlebih dahulu agar waktu akurat
* Potensiometer hanya simulasi, bisa diganti sensor kekeruhan asli
* Sistem menggunakan logika state (`sedangGantiAir`) agar lebih stabil
