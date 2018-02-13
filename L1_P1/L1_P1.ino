#include <wm8731.h>
#include <WM8731_Audio.h>
#include<PowerDue.h>
#include"MIDIParser.h"

MidiCallbacks_t midiCallbacks;
MidiParser *parser;

uint32_t _bpm = 120;
uint16_t _tpm = 96;
unsigned long tof_serial;
unsigned long tof_parser;
unsigned long tof_proc;
unsigned long tof_begin;
float fact;
float freq;

float notes[108];
  
void midi_handleEvent(uint64_t ticks, uint8_t status, uint8_t note, uint8_t velocity){
  tof_parser = micros();
  //SerialUSB.println("Event Detected");
  //SerialUSB.println(note);
  fact = float(pow(2.000,((float) note - 69.0)/12.0));
  freq = 440*fact;
  uint32_t wait_time_ms = TRACK_TIMER_PERIOD_MS(_tpm,_bpm)*ticks;
  delay(wait_time_ms);
  if(status==STATUS_NOTE_ON && velocity>0){
    Codec.playTone(freq);
    //SerialUSB.println(fact);
  }else{
    Codec.stopTone();
  }
  tof_proc = micros();
  SerialUSB.print("Processing from Serial Port required ");
  SerialUSB.println(tof_serial-tof_begin);
  SerialUSB.print("Parsing data from serial port required ");
  SerialUSB.println(tof_parser-tof_serial);
  SerialUSB.print("Processing data required ");
  SerialUSB.println(tof_proc-tof_parser);
  SerialUSB.println();
}

void midi_volumeChanged(uint8_t vol){
   SerialUSB.print("Volume changed to ");  
   SerialUSB.println(vol);
   Codec.setOutputVolume(vol);
   SerialUSB.println();
}

void midi_trackStart(void){
  
}

void midi_trackEnd(void){
  
}

void midi_setTicks(uint16_t tpm){
  _tpm = tpm;
}

void midi_setbpm(uint32_t bpm){
  _bpm = bpm;
}

void setup() {
  PowerDue.LED();
  PowerDue.LED(PD_GREEN);

  midiCallbacks.HandleEvent = midi_handleEvent;
  midiCallbacks.VolumeChanged = midi_volumeChanged;
  midiCallbacks.TrackStart = midi_trackStart;
  midiCallbacks.TrackEnd = midi_trackEnd;
  midiCallbacks.SetTicksPerNote = midi_setTicks;
  midiCallbacks.SetBPM = midi_setbpm;

  parser = new MidiParser(&midiCallbacks);
  
  Codec.begin();
  Codec.setOutputVolume(80);

  SerialUSB.begin(0);
  while(!SerialUSB);
}



void loop() {
  // put your main code here, to run repeatedly:
  tof_begin = micros();
  if(SerialUSB.available()){
   tof_serial = micros();
   parser->feed(SerialUSB.read()); 
  }
}
