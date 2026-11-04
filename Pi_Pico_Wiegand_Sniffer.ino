#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_TinyUSB.h>

// 1. Configure the Pico's Internal Flash to act as a USB Drive
Adafruit_FlashTransport_RP2040 flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
Adafruit_USBD_MSC usb_msc;

// 2. Configure the HID Keyboard Emulation
uint8_t const desc_hid_report[] = { TUD_HID_REPORT_DESC_KEYBOARD() };
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

// 3. Wiegand Hardware Setup
const int PIN_D0 = 14; 
const int PIN_D1 = 15; 
const int PIN_CLEAR = 16;

volatile unsigned long lastPulseTime = 0;
volatile int bitCount = 0;
char wiegandData[100]; 

// Instant Hardware Interrupts for Wiegand Data
void isrD0() {
  if (bitCount < 99) wiegandData[bitCount++] = '0';
  lastPulseTime = millis();
}
void isrD1() {
  if (bitCount < 99) wiegandData[bitCount++] = '1';
  lastPulseTime = millis();
}

// USB Mass Storage Block Read/Write Callbacks
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
  return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
  return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}
void msc_flush_cb(void) {
  flash.syncBlocks();
}

void setup() {
  // Initialize the internal flash memory
  flash.begin();
  
  // Bind the flash memory to the USB Mass Storage Controller
  usb_msc.setID("Raspberry", "Pico Drive", "1.0");
  usb_msc.setCapacity(flash.size() / 512, 512);
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  usb_msc.setUnitReady(true);
  usb_msc.begin();

  // Initialize the HID Keyboard
  usb_hid.begin();

  // Initialize Physical Pins
  pinMode(PIN_D0, INPUT_PULLUP);
  pinMode(PIN_D1, INPUT_PULLUP);
  pinMode(PIN_CLEAR, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(PIN_D0), isrD0, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), isrD1, FALLING);
  
  // Wait for the host PC to mount the device
  while( !TinyUSBDevice.mounted() ) delay(1);
}

void loop() {
  // If we have bits and 50ms have passed, the swipe is done
  if (bitCount > 0 && (millis() - lastPulseTime > 50)) {
    wiegandData[bitCount] = '\0'; 
    
    // Type the binary string one character at a time
    for (int i = 0; i < bitCount; i++) {
      usb_hid.keyboardPress(0, wiegandData[i]);
      delay(5);
      usb_hid.keyboardRelease(0);
      delay(5);
    }
    
    // Send Enter key via raw HID report
    uint8_t keycode[6] = {0};
    keycode[0] = HID_KEY_ENTER;
    usb_hid.keyboardReport(0, 0, keycode);
    delay(10);
    usb_hid.keyboardRelease(0);
    
    bitCount = 0;
    wiegandData[0] = '\0';
  }
  
  // Clear Button Logic (Only triggers if reader is idle)
  if (bitCount == 0 && digitalRead(PIN_CLEAR) == LOW) {
    uint8_t keycode[6] = {0};
    
    // Send Ctrl+A
    keycode[0] = HID_KEY_A;
    usb_hid.keyboardReport(0, KEYBOARD_MODIFIER_LEFTCTRL, keycode);
    delay(10);
    usb_hid.keyboardRelease(0);
    delay(50);
    
    // Send Backspace
    keycode[0] = HID_KEY_BACKSPACE;
    usb_hid.keyboardReport(0, 0, keycode);
    delay(10);
    usb_hid.keyboardRelease(0);
    
    while(digitalRead(PIN_CLEAR) == LOW) delay(10); 
  }
}