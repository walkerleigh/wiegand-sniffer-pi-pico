# Raspberry Pi Pico Wiegand Sniffer & Offline Calculator

This guide details how to build a high-speed Wiegand-to-USB Keyboard sniffer that simultaneously acts as a USB Flash Drive (MSC) to host an offline HTML calculator. It uses C++ and hardware interrupts to guarantee zero data loss during rapid 50µs Wiegand pulses.

## 1. Hardware Requirements
*   Raspberry Pi Pico [Link](https://www.jaycar.com.au/universal-usb-12v-step-up-power-cable/p/PP1978)
*   HID Signo 20 Reader (configured to output standard Wiegand, not OSDP). Ideally you need to find one with the prox antenna installed, also make sure all antennas are switched on using the HID Reader Manager app.
*   Jaycar XC4486 Level Shifter (or similar 5V to 3.3V logic level converter) [Link](https://www.jaycar.com.au/duinotech-arduino-compatible-logic-level-converter-module/p/XC4486)
*   12V DC Power Supply (for the reader) [Link](https://www.jaycar.com.au/universal-usb-12v-step-up-power-cable/p/PP1978)
*   Momentary Push Button (for clearing the text box) [Link](https://www.jaycar.com.au/ip67-rated-dome-pushbutton-switch-black/p/SP0656)
*   PCB Prototyping Board [Link](https://www.jaycar.com.au/small-breadboard-layout-prototyping-board/p/HP9570)
*   Enclosure [Link](https://www.jaycar.com.au/plastic-molded-enclosure-dark-grey-abs-190-x-100-x-40mm/p/HB6036)


## 2. Wiring Connections

**Power Connections**
*   **12V DC Supply (+)**  → HID Signo 20 Red Wire (12VDC)
*   **Pico VBUS (Pin 40)** → Level Shifter 5V Side `HV`
*   **Pico 3V3(OUT) (Pin 36)** → Level Shifter 3.3V Side `LV`

**Ground Network (Common Ground)**
*   Connect all of the following together: 12V DC Supply (-), HID Signo 20 Black Wire, Pico GND (Pin 18), Level Shifter 5V Side `GND`, and Level Shifter 3.3V Side `GND`.
*   Connect one leg of the **Push Button** to this common ground.

**Data Signal Paths (5V to 3.3V)**
*   **Data 0:** HID Signo 20 Green Wire → Shifter 5V `Channel 1 RXI`
*   **Data 0:** Shifter 3.3V `Channel 1 RXO` → Pico GP14 (Pin 19)
*   **Data 1:** HID Signo 20 White Wire → Shifter 5V `Channel 2 RXI`
*   **Data 1:** Shifter 3.3V `Channel 2 RXO` → Pico GP15 (Pin 20)

[See fritzing schematic](Wiegand%20Sniffer_bb.pdf)

**Button Connection**
*   Connect the remaining leg of the **Push Button** → Pico GP16 (Pin 21)

## 3. Arduino IDE Setup

1.  **Install the Pico Core:** 
    *   Go to **File > Preferences**. 
    *   Add `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json` to the *Additional Boards Manager URLs*.
    *   Go to **Tools > Board > Boards Manager**, search for "pico", and install **Raspberry Pi Pico/RP2040 by Earle F. Philhower, III**.
2.  **Install Libraries:**
    *   Go to **Sketch > Include Library > Manage Libraries**.
    *   Install **Adafruit SPIFlash**.
    *   Install **SdFat - Adafruit Fork**.
3.  **Configure Board Settings (Tools Menu):**
    *   **Board:** Raspberry Pi Pico
    *   **USB Stack:** Adafruit TinyUSB
    *   **Flash Size:** 2MB (Sketch: 1MB, FS: 1MB) *<-- CRITICAL for the flash drive to work*

## 4. C++ Source Code

Copy and paste the following code into your Arduino IDE. This code utilizes raw HID reports for maximum compatibility and uses hardware interrupts for instantaneous Wiegand pulse detection.
[Pi_Pico_Wiegand_Sniffer.ino](Pi_Pico_Wiegand_Sniffer.ino)

## 5. Deployment and Formatting

1.  Click **Upload** in the Arduino IDE to compile and flash the code to the Pico.
2.  Once uploaded, the Pico will reboot and connect to Windows. Windows will display a popup saying, "You need to format the disk in drive X: before you can use it."
3.  Click **Format disk**. Leave the File System set to **FAT (Default)** and click Start. 
4.  Once the format is complete, open the newly mounted 1MB flash drive.
5.  Drag and drop Ben Hyde's `Access Card Format Calculator v3.html` file into the root of this drive.

## 6. Usage

*   Plug the device into any computer. It mounts instantly as a flash drive.
*   Open the HTML file directly from the drive.
*   Swipe a card on the HID Signo 20. The hardware interrupts will capture the Wiegand pulses and type the binary string into your HTML calculator, followed by the "Enter" key.
*   Press the physical push button to send a `Ctrl+A` and `Backspace` command to clear the calculator's input box for the next card.
