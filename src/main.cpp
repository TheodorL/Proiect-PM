#include <Arduino.h>
#include "notes.h"
#include "analogWave.h"
#include "utils.h"
#include <includes/ra/fsp/src/bsp/cmsis/Device/RENESAS/Include/R7FA4M1AB.h>
#include "FspTimer.h"

const uint32_t RESOLUTION = 32;
const uint32_t ARP_DIV = 8;
FspTimer timer;
uint8_t arpCount = 0;
uint64_t count = 0;
char currentNote = NO_NOTE;
char currentRecordNote = 0;
char oldNote = NO_NOTE;
uint64_t start_time = 0;
uint32_t BPM = 240;
uint32_t freq = (BPM/60) * RESOLUTION;
int8_t click_duration = RESOLUTION/4;
int8_t click = click_duration;
analogWave wave(DAC);
const uint16_t recordSize = RESOLUTION * 4;
int16_t recordCount = 0;
bool record = false;
bool recorded = false;

char recordedNotes[recordSize];


void timer_callback(timer_callback_args_t __attribute((unused)) *p_args) {
  if (arpCount >= arpLen) {
    arpCount = 0;
  } else {
    currentNote = arp[arpCount];
  }
  if (recordCount >= recordSize) {
    if (record) {
      recorded = true;
    }
    record = 0;
    recordCount = 0;
  }
  count++;
  click--;
  if (count % ARP_DIV == 0){
    arpCount++;
    recordCount++;

    if(record && !recorded) {
    recordedNotes[recordCount] = currentNote;
  }
  } 
  if (count >= RESOLUTION) {
    count = 0;
    click = click_duration;
  }
}


bool beginTimer(float rate) {
  uint8_t timer_type = GPT_TIMER;
  int8_t tindex = FspTimer::get_available_timer(timer_type);
  if (tindex < 0){
    tindex = FspTimer::get_available_timer(timer_type, true);
  }
  if (tindex < 0){
    return false;
  }

  FspTimer::force_use_of_pwm_reserved_timer();

  if(!timer.begin(TIMER_MODE_PERIODIC, timer_type, tindex, rate, 0.0f, timer_callback)){
    return false;
  }

  if (!timer.setup_overflow_irq()){
    return false;
  }

  if (!timer.open()){
    return false;
  }

  if (!timer.start()){
    return false;
  }
  return true;
}

void cleanRecord() {
  Serial.println("clean");
  for (int i = 0; i < recordSize; i++) {
    recordedNotes[i] = NO_NOTE;
  }
}

void setup() {
  memset(arpPos, NO_ARP_POS, NUM_NOTES);
  beginTimer(freq);
  start_time = millis();
  wave.square(400);
  wave.stop();
  //setting pins D2, D3, D4, D5 as OUTPUT pins
  R_PORT1->PCNTR1 |= (1 << PDR(MP_S0)) | (1 << PDR(MP_S1)) |
                     (1 << PDR(MP_S2)) | (1 << PDR(MP_S3));
  //initializing D2, D3, D4, D5 with LOW
  R_PORT1->PCNTR1 = MULTIBIT_SET(R_PORT1->PCNTR1, MP_CH_MASK, MP_CHANNELS[0]);
  //setting pin D7 as INPUT pin
  R_PORT1->PCNTR1 &= ~(1 << PDR(MP_SIG));
  pinMode(D12, OUTPUT);
}

bool sig = false;
bool ch12_oldsig = false;
void loop() {
  
  
  for (int i = 0; i < 14; i++) {
    R_PORT1->PCNTR1 = MULTIBIT_SET(R_PORT1->PCNTR1, MP_CH_MASK, MP_CHANNELS[i]);
    delayMicroseconds(1); //multiplexor takes about 500ns to switch channels
    int sig = IS_SET(R_PORT1->PCNTR2, PIDR(MP_SIG));
    if (i < 12) {
      if(sig) {
        addArpNote(i);
      } else {
        delArpNote(i);
      }
    } else if (i == 12) {
      if (sig & !ch12_oldsig) {
        if(record == false) {
          if (recorded) {
            cleanRecord();
            recorded = false;
          } else {
          record = true;
          recordCount = 0;
          }
        }
      }
      ch12_oldsig = sig;
    }
  }
  if (arpLen <= 0) {
    wave.stop();
    currentNote = NO_NOTE;
    oldNote = NO_NOTE;
  } else if (currentNote != oldNote) {
    wave.freq(KEY_NOTES[currentNote]);

    oldNote = currentNote;
  }
  if (recorded && recordedNotes[recordCount] != NO_NOTE) {
    tone(D12, KEY_NOTES[recordedNotes[recordCount]]);
  } else if (!recorded){
    if (click > 0) {
    tone(D12, NOTE_C4);
  } else {
    noTone(D12);
  }
  } else {
    noTone(D12);
  }

  return;
}
