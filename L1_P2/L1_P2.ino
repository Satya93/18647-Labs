#include <assert.h>
#include <FreeRTOS_ARM.h>
#include <IPAddress.h>
#include <PowerDueWiFi.h>
#include "WM_synth.h"
#include"MIDIParser.h"

MidiCallbacks_t midiCallbacks;
MidiParser *parser;

uint32_t _bpm = 120;
uint16_t _tpm = 96;
signed long tof_serial;
signed long tof_parser;
signed long tof_proc;
signed long tof_begin;

float fact;
uint16_t freq;

float notes[108];

#define WIFI_SSID "PowerDue"
#define WIFI_PASS "powerdue"

#define SERVER_ADDR "10.230.8.1"
#define SERVER_PORT 3000
#define ANDREW_ID "srwagle\n"
#define BUFSIZE 64

void midi_handleEvent(uint64_t ticks, uint8_t status, uint8_t note, uint8_t velocity){
  tof_serial = micros();
  uint32_t wait_time_ms = TRACK_TIMER_PERIOD_MS(_tpm,_bpm)*ticks;
  vTaskDelay(pdMS_TO_TICKS(wait_time_ms));
  if(status==STATUS_NOTE_ON && velocity>0){
    WMSynth.playToneN(note, velocity, 1);
    //SerialUSB.println(fact);
  }else{
    WMSynth.stopToneN(note);
  }
  SerialUSB.print("Processing Time is ");
  SerialUSB.println(tof_serial-tof_begin);
}

void midi_volumeChanged(uint8_t vol){
   SerialUSB.print("Volume changed to ");  
   SerialUSB.println(vol);
   WMSynth.setMasterVolume(vol);
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


void TcpMIDIStream(void *arg){

  MidiParser parser = MidiParser(&midiCallbacks);
  
  uint8_t buf[BUFSIZE];
  struct sockaddr_in serverAddr;
  socklen_t socklen;
  memset(&serverAddr, 0, sizeof(serverAddr));

  serverAddr.sin_len = sizeof(serverAddr);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_ADDR, &(serverAddr.sin_addr));

  int s = lwip_socket(AF_INET, SOCK_STREAM, 0);
  if(lwip_connect(s, (struct sockaddr *)&serverAddr, sizeof(serverAddr))){
    SerialUSB.println("Failed to connect");
    assert(false);
  }

  lwip_write(s, ANDREW_ID, strlen(ANDREW_ID));
  SerialUSB.println("Connected and Waiting for events");

  while(1){
    memset(&buf, 0, BUFSIZE);
    tof_begin = micros();
    int n = lwip_read(s, buf, BUFSIZE);
    for(int i=0; i<n; i++){
      parser.feed(buf[i]);
    }
  }

  lwip_close(s);
}


void onReady(){
  SerialUSB.println("Device ready");  
  SerialUSB.print("Device IP: ");
  SerialUSB.println(IPAddress(PowerDueWiFi.getDeviceIP()));  
  
  xTaskCreate(TcpMIDIStream, "MIDIStreamer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}

void onError(int errorCode){
  SerialUSB.print("Error received: ");
  SerialUSB.println(errorCode);
}


void setup(){
  SerialUSB.begin(0);
  while(!SerialUSB);

  midiCallbacks.HandleEvent = midi_handleEvent;
  midiCallbacks.VolumeChanged = midi_volumeChanged;
  midiCallbacks.TrackStart = midi_trackStart;
  midiCallbacks.TrackEnd = midi_trackEnd;
  midiCallbacks.SetTicksPerNote = midi_setTicks;
  midiCallbacks.SetBPM = midi_setbpm;

  WMSynth.setMasterVolume(254);
  WMSynth.startSynth(3);
  
  PowerDueWiFi.init(WIFI_SSID, WIFI_PASS);
  PowerDueWiFi.setCallbacks(onReady, onError);
  
  SerialUSB.println("Starting Scheduler");
  vTaskStartScheduler();
  
  SerialUSB.println("Insufficient RAM");
  while(1);
}

void loop(){
  
}

