#include "RF24.h"
#include <SPI.h>
#include <ezButton.h>
#include "esp_bt.h"
#include "esp_wifi.h"

SPIClass *sp = nullptr;
SPIClass *hp = nullptr;

// Настройка пинов для HSPI и VSPI (оставлено без изменений)
RF24 radio(16, 15, 16000000);   // HSPI
RF24 radio1(22, 21, 16000000);  // VSPI

unsigned int flag = 0;   // HSPI
unsigned int flagv = 0;  // VSPI
int ch = 45;
int ch1 = 45;

ezButton toggleSwitch(33);

// Вспомогательная функция для плавного изменения (оставлена для справки)
void two() {
  if (flagv == 0) { ch1 += 4; } else { ch1 -= 4; }
  if (flag == 0) { ch += 2; } else { ch -= 2; }

  if ((ch1 > 79) && (flagv == 0)) { flagv = 1; } else if ((ch1 < 2) && (flagv == 1)) { flagv = 0; }
  if ((ch > 79) && (flag == 0)) { flag = 1; } else if ((ch < 2) && (flag == 1)) { flag = 0; }

  radio.setChannel(ch);
  radio1.setChannel(ch1);
}

// Основная функция для создания помех: быстрый случайный скачок по каналам
void one() {
  // Устанавливаем случайные каналы для обоих модулей по всему диапазону
  radio.setChannel(random(80));
  radio1.setChannel(random(80));
  
  // Небольшая задержка для стабилизации синтезатора частоты (PLL)
  delayMicroseconds(130); 
}

void setup() {
  Serial.begin(115200);
  // Отключаем Wi-Fi и Bluetooth на ESP32, чтобы они не создавали лишних помех
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();
  
  toggleSwitch.setDebounceTime(50);

  initHP();
  initSP();
}

void initSP() {
  sp = new SPIClass(VSPI);
  sp->begin();
  if (radio1.begin(sp)) {
    Serial.println("SP Started !!!");
    radio1.setAutoAck(false);
    radio1.stopListening();
    radio1.setRetries(0, 0);
    
    // === ПРАВКА 1: Понижен уровень мощности с MAX до HIGH ===
    radio1.setPALevel(RF24_PA_HIGH, true); 
    
    // === ПРАВКА 3: Попробуйте RF24_1MBPS для более широкого сигнала ===
    // Если помехи станут хуже, верните RF24_2MBPS
    radio1.setDataRate(RF24_1MBPS); 
    
    radio1.setCRCLength(RF24_CRC_DISABLED);
    radio1.printPrettyDetails();
    
    // Используем RF24_PA_HIGH и для постоянной несущей
    radio1.startConstCarrier(RF24_PA_HIGH, ch1);
  } else {
    Serial.println("SP couldn't start !!!");
  }
}

void initHP() {
  hp = new SPIClass(HSPI);
  hp->begin();
  if (radio.begin(hp)) {
    Serial.println("HP Started !!!");
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setRetries(0, 0);
    
    // === ПРАВКА 1: Понижен уровень мощности с MAX до HIGH ===
    radio.setPALevel(RF24_PA_HIGH, true); 
    
    // === ПРАВКА 3: Попробуйте RF24_1MBPS для более широкого сигнала ===
    radio.setDataRate(RF24_1MBPS); 
    
    radio.setCRCLength(RF24_CRC_DISABLED);
    radio.printPrettyDetails();
    
    // Используем RF24_PA_HIGH и для постоянной несущей
    radio.startConstCarrier(RF24_PA_HIGH, ch);
  } else {
    Serial.println("HP couldn't start !!!");
  }
}

void loop() {
  // === ПРАВКА 2: Убрана зависимость от переключателя, всегда используется one() ===
  // toggleSwitch.loop(); 
  
  // Всегда вызываем функцию для случайного скачка по каналам
  one();
}
