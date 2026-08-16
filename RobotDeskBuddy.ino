#include <LiquidCrystal.h>

// --- PİN TANIMLAMALARI ---
const int BUTTON = 7;     // Kronometre Başlat/Durdur butonu
const int PR = A5;        // LDR Işık Sensörü
const int PIEZO = 2;      // Buzzer
const int GAME = 3;       // Mod Değiştirme Butonu (Mod 0 <-> Mod 1)

const int RS = 8;
const int E = 9;
const int DB4 = 10;
const int DB5 = 11;
const int DB6 = 12;
const int DB7 = 13;

LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);

// --- ÖZEL LCD KARAKTERLERİ ---
byte normalGoz[8] = {
  B01110,
  B11111,
  B11111,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000
};

byte yorgunGoz[8] = {
  B00000,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte kalpSembol[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

// --- SİSTEM DEĞİŞKENLERİ ---
int sistemModu = 0;       // 0: Işık/Ruh Hali, 1: Kronometre
int prValue;

// Debounce Zamanlayıcıları
unsigned long sonGameBasma = 0;
unsigned long sonBtnBasma = 0;
const unsigned long debounceGecikmesi = 300;

// Kronometre Değişkenleri
bool kronometreCalisiyor = false;
unsigned long kronometreBaslangic = 0;
unsigned long gecenToplamSure = 0;

// Boşta Kalma (Sıkılma) Takibi
unsigned long sonEtkilesimZamani = 0;
const unsigned long sikilmaSuresi = 15000; 
bool sikildiMi = false;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(GAME, INPUT_PULLUP);
  pinMode(PR, INPUT);
  pinMode(PIEZO, OUTPUT);

  lcd.begin(16, 2);

  // Özel karakterleri belleğe yükle
  lcd.createChar(0, normalGoz);
  lcd.createChar(2, yorgunGoz);
  lcd.createChar(3, kalpSembol);

  // Açılış Animasyonu
  lcd.setCursor(4, 0);
  lcd.write(byte(2)); 
  lcd.print(" Uyanior ");
  lcd.write(byte(2));
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.write(byte(0));
  lcd.print(" Merhaba! ");
  lcd.write(byte(0));
  
  lcd.setCursor(0, 1);
  lcd.print("  Gunaydin! <3  ");

  tone(PIEZO, 1000, 100);
  delay(2500);
  lcd.clear();

  sonEtkilesimZamani = millis();
}

void loop() {
  unsigned long simdikiZaman = millis();

  // --- MOD DEĞİŞTİRME (GAME BUTONU) ---
  if (digitalRead(GAME) == LOW && (simdikiZaman - sonGameBasma) > debounceGecikmesi) {
    sistemModu = (sistemModu + 1) % 2; // 0 ve 1 arasında geçiş yap
    sonGameBasma = simdikiZaman;
    sonEtkilesimZamani = simdikiZaman;
    sikildiMi = false;
    tone(PIEZO, 1200, 100);
    lcd.clear();
  }

  // Tuş basıldığında sıkılma sayacını sıfırla
  if (digitalRead(BUTTON) == LOW || digitalRead(GAME) == LOW) {
    sonEtkilesimZamani = simdikiZaman;
    if (sikildiMi) {
      sikildiMi = false;
      lcd.clear();
    }
  }

  // --- SIKILMA DAVRANIŞI ---
  if ((simdikiZaman - sonEtkilesimZamani > sikilmaSuresi) && !sikildiMi && sistemModu == 0) {
    sikildiMi = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sikildim ama... ");
    lcd.setCursor(0, 1);
    lcd.print("Bana dokunsana? ");
    tone(PIEZO, 600, 80);
    delay(200);
    tone(PIEZO, 400, 80);
  }

  if (sikildiMi) {
    return; 
  }

  // --- ÇALIŞMA MODLARI ---
  if (sistemModu == 0) {
    // MOD 0: Işık Sensörü & Ruh Hali
    prValue = analogRead(PR);

    lcd.setCursor(0, 0);
    lcd.print("Isik: ");
    lcd.print(prValue);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    if (prValue < 300) {
      lcd.print("Mod: Uyuyor Zzz ");
      lcd.setCursor(14, 1); lcd.write(byte(2)); // Uykulu göz
    } else {
      lcd.print("Mod: Calisiyor  ");
      lcd.setCursor(14, 1); lcd.write(byte(0)); // Normal göz
    }
    delay(200);

  } 
  else if (sistemModu == 1) {
    // MOD 1: KRONOMETRE
    lcd.setCursor(0, 0);
    lcd.print("Kronometre      ");

    // Buton ile kronometreyi başlat / durdur
    if (digitalRead(BUTTON) == LOW && (simdikiZaman - sonBtnBasma) > debounceGecikmesi) {
      sonBtnBasma = simdikiZaman;
      kronometreCalisiyor = !kronometreCalisiyor;
      if (kronometreCalisiyor) {
        kronometreBaslangic = millis() - gecenToplamSure;
      }
      tone(PIEZO, 1500, 80);
    }

    if (kronometreCalisiyor) {
      gecenToplamSure = millis() - kronometreBaslangic;
    }

    unsigned long toplamSaniye = gecenToplamSure / 1000;
    int dakika = (toplamSaniye / 60) % 60;
    int saniye = toplamSaniye % 60;

    // 2 saat çalışma uyarısı
    if (toplamSaniye >= 7200) {
      lcd.setCursor(0, 1);
      lcd.print("Yorgun! Mola ver");
      tone(PIEZO, 400, 200);
      delay(500);
      return;
    }

    lcd.setCursor(0, 1);
    if (kronometreCalisiyor) {
      lcd.print("Calis: ");
    } else {
      lcd.print("Durdv: ");
    }
    
    if (dakika < 10) lcd.print("0");
    lcd.print(dakika);
    lcd.print(":");
    if (saniye < 10) lcd.print("0");
    lcd.print(saniye);
    lcd.print(" ");
    lcd.write(byte(3));
    delay(100);
  }
}