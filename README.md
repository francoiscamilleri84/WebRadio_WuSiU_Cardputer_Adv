# Cardputer Adv WebRadio

An advanced internet radio for **M5Stack Cardputer Adv** with SD card station list support, Wi-Fi memory, and additional audio features.

![Cardputer ADV WebRadio – running](images/WebRadio.jpg)

---

## 📻 Radio Station List

The list of radio stations is stored in a text file: **station_list.txt**

The file must be placed in the **/web_radio_config/ directory of the SD card**.

### Priority and Storage Options

- The station list can also be **stored permanently inside the firmware code** and used without an SD card.
- If an SD card is inserted and the file **station_list.txt** exists, the device will **load the station list from the SD card first**.
- If the file is not found, the internal station list compiled in the firmware will be used.

### File format

Example:

Radio 01 Name, http://radio-stream-link-01

Radio 02 Name, http://radio-stream-link-02


- One station per line  
- Station name and stream URL separated by a comma

---

## ⌨️ Controls (Cardputer Adv Keyboard)

- **Left / Right Arrow** – change radio station  
- **Up / Down Arrow** – adjust volume  
- **R** – reset the server connection (if the radio freezes or fails to start)  
- **M** – toggle mute on / off  
- **F** – toggle FFT audio visualization  
- **B** – adjust screen brightness  
- **L** – enable / disable the station list view  
- **ENTER** – confirm station selection when inside the station list menu  

---

## 📶 Wi-Fi

SSID and Password and stored in wifi.txt in /web_radio_config/ 

### Connection Priority Logic

1. The device first attempts to connect to the **last used network**.
2. If the connection fails, it scans for available networks already stored in memory.
3. If multiple known networks are found, the device connects to the **strongest signal**.
4. If no known networks are available, the device displays the list of currently available networks for selection.

- Wi-Fi settings are saved to wifi.txt


---

## 🧰 Requirements

- M5Stack Cardputer Adv  
- SD card with `station_list.txt` (optional if stations are stored in firmware)  
- Wi-Fi connection

---

## ⚙️ Arduino IDE Configuration

Before compiling and uploading the firmware, make sure the correct board and partition scheme are selected in Arduino IDE.

---

### 🧩 Select Board

Choose the correct board for Cardputer:

**M5Cardputer**

![Board_selection](images/board.jpg)

---

### 💾 Partition Scheme

Set the partition scheme to:

**Huge APP (3MB No OTA / 1MB SPIFFS)**

This is required to fit the web radio application in flash memory.

![Partition scheme](images/partition_scheme.jpg)

---

## 👤 Author

**WuSiU**

## 📄 License

MIT License

## Credits

Based on the original project:  
https://github.com/cyberwisk/M5Cardputer_WebRadio

Original author: cyberwisk

