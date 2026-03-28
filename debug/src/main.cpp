#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <nrf_lpcomp.h>

#define PIN WB_A1  // AIN1 -> P0.31 -> LPCOMP AIN7
#define DEBOUNCE_MS 500  // ignore events within this window

// --- LPCOMP raw interrupt ---
volatile unsigned long lpcomp_last_irq = 0;
volatile int lpcomp_raw_count = 0;

extern "C" void LPCOMP_IRQHandler(void) {
  if (NRF_LPCOMP->EVENTS_CROSS) {
    NRF_LPCOMP->EVENTS_CROSS = 0;
    lpcomp_last_irq = millis();
    lpcomp_raw_count++;
  }
  if (NRF_LPCOMP->EVENTS_DOWN) NRF_LPCOMP->EVENTS_DOWN = 0;
  if (NRF_LPCOMP->EVENTS_UP) NRF_LPCOMP->EVENTS_UP = 0;
}

// --- Debounced state ---
bool motion_on = false;          // current debounced state
bool prev_motion_on = false;     // for edge detection
unsigned long last_change = 0;   // when debounced state last changed
int motion_events = 0;           // count of debounced ON events

void setup_lpcomp() {
  NRF_LPCOMP->ENABLE = LPCOMP_ENABLE_ENABLE_Disabled;
  NRF_LPCOMP->PSEL = 7;  // AIN7 (P0.31 = WB_A1)
  NRF_LPCOMP->REFSEL = LPCOMP_REFSEL_REFSEL_SupplyTwoEighthsPrescaling;  // VDD*2/8 ~825mV
  NRF_LPCOMP->ANADETECT = LPCOMP_ANADETECT_ANADETECT_Cross;
  NRF_LPCOMP->HYST = LPCOMP_HYST_HYST_Hyst50mV;
  NRF_LPCOMP->INTENSET = LPCOMP_INTENSET_CROSS_Msk;
  NRF_LPCOMP->ENABLE = LPCOMP_ENABLE_ENABLE_Enabled;
  NRF_LPCOMP->TASKS_START = 1;

  NVIC_SetPriority(LPCOMP_IRQn, 6);
  NVIC_ClearPendingIRQ(LPCOMP_IRQn);
  NVIC_EnableIRQ(LPCOMP_IRQn);

  // Read initial state
  NRF_LPCOMP->TASKS_SAMPLE = 1;
  while (!NRF_LPCOMP->EVENTS_READY) {}
  NRF_LPCOMP->EVENTS_READY = 0;
  // RESULT: 0=below ref (LED on), 1=above ref (LED off)
  motion_on = (NRF_LPCOMP->RESULT == 0);
  prev_motion_on = motion_on;

  Serial.printf("LPCOMP: ref=VDD*2/8 (~825mV), hyst=50mV, debounce=%dms\n", DEBOUNCE_MS);
  Serial.printf("LPCOMP: initial=%s\n", motion_on ? "MOTION ON" : "IDLE");
}

void check_debounced_state() {
  // Sample current comparator result
  NRF_LPCOMP->TASKS_SAMPLE = 1;
  delayMicroseconds(10);
  bool current = (NRF_LPCOMP->RESULT == 0);  // 0=below=LED on

  // Only accept state change if stable for DEBOUNCE_MS
  if (current != motion_on) {
    if (millis() - lpcomp_last_irq >= DEBOUNCE_MS) {
      motion_on = current;
      last_change = millis();
      if (motion_on) motion_events++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10);

  Serial.println("\n=== RAK4631 LPCOMP Debounce Test ===");

  analogCalibrateOffset();
  setup_lpcomp();

  Serial.println("Ready.\n");
}

void loop() {
  check_debounced_state();

  // Report state changes
  if (motion_on != prev_motion_on) {
    if (motion_on) {
      Serial.printf(">>> MOTION ON  (event #%d, raw IRQs=%d) <<<\n", motion_events, lpcomp_raw_count);
    } else {
      Serial.printf(">>> MOTION OFF (raw IRQs=%d) <<<\n", lpcomp_raw_count);
    }
    prev_motion_on = motion_on;
  }

  // Periodic status
  static unsigned long last_print = 0;
  if (millis() - last_print >= 2000) {
    last_print = millis();

    // Analog for reference
    analogReadResolution(12);
    analogReference(AR_INTERNAL);
    long asum = 0;
    for (int i = 0; i < 32; i++) asum += analogRead(PIN);
    float v_avg = (asum / 32) * 3600.0 / 4096.0;

    Serial.printf("state=%s  analog=%.0fmV  motion_events=%d  raw_irqs=%d\n",
                  motion_on ? "ON " : "OFF", v_avg, motion_events, lpcomp_raw_count);
  }

  delay(50);  // check ~20 times per second
}
