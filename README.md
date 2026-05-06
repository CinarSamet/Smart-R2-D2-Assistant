# 🤖 Smart R2-D2 Assistant: Edge-to-Cloud AI Robotics

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Hardware License](https://img.shields.io/badge/Hardware-CC%20BY--NC%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc/4.0/)
[![C++](https://img.shields.io/badge/C++-ESP32-00599C?logo=c%2B%2B)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-Flask-3776AB?logo=python)](https://www.python.org/)

This project is an end-to-end robotics project that combines a custom 3D-printed design of the iconic R2-D2 character with a real-time, AI-based voice assistant. While the system handles hardware and sensor management on the edge device (ESP32), it offloads the AI workload—which requires heavy processing power—to a local Python server.

## 🌟 Highlighted Features

*   **Real-Time Communication (Edge-to-Cloud):** Transmission of RAW PCM audio data received from the microphone via I2S protocol on the ESP32 to the local server over HTTP.
*   **Speech-to-Text (STT):** High-accuracy voice recognition using the OpenAI Whisper (Turbo) model.
*   **Artificial Intelligence (LLM):** Context-aware conversational response generation fitting the R2-D2 character, integrated with the Google Gemini API.
*   **Text-to-Speech (TTS):** Fluent and natural voice synthesis using the Edge-TTS infrastructure.
*   **Dynamic UI:** State-based (Listening, Thinking, Speaking, etc.) dynamic facial animations on an OLED display using the `RobotFace` library.
*   **Wireless Management:** Network configuration with WiFiManager and remote over-the-air code update support with ArduinoOTA.

## 🏗️ System Architecture

The system consists of two asynchronous units: **Edge** and **Server**:

1.  **ESP32 (Edge Device):** Woken up by a touch sensor. It records audio from the I2S microphone and transmits it to the server in RAW PCM format. It manages the animations and plays the response received from the server through the I2S amplifier.
2.  **Flask Server (Python):** 
    *   Converts the audio to a processable WAV format using `FFmpeg`.
    *   Converts speech to text using `Whisper`.
    *   Analyzes the text and generates a response using `Gemini LLM`.
    *   Converts the response to speech using `Edge-TTS`, converts it back to RAW PCM, and sends it back to the device.

## 🛠️ Hardware and 3D Design

R2-D2's outer case and inner chassis were designed from scratch, fully optimized for 3D printers.

👉 **[3D Model and STL Files (Thingiverse)](https://www.thingiverse.com/thing:7346557)**

**Main Electronic Components Used:**
*   **Microcontroller:** ESP32 Development Board
*   **Microphone:** INMP441 (I2S MEMS)
*   **Audio Output:** MAX98357A (I2S Class D Amplifier) + Speaker
*   **Display:** I2C OLED Display
*   **Sensors:** Capacitive Touch Sensor (For wake-up)

## 📁 Project Structure
```text
├── server/
│   ├── app.py                 # Main Flask server application
│   ├── config/
│   │   └── settings.json      # API, TTS, and Server settings
│   └── requirements.txt       # Python dependencies
├── esp32/
│   ├── main.ino               # Main ESP32 source code
│   ├── RobotFace.h            # OLED animation library
│   └── r2d2_ses.h             # System sounds on PROGMEM
├── .gitignore
└── README.md
```

## 🚀 Setup and Usage

### 1. Server (Python Backend) Setup
For the project to work, **FFmpeg** must be installed on your computer and added to your system's PATH.

1. Clone the repository to your computer and enter the directory:
   ```bash
   git clone [https://github.com/CinarSamet/Smart-R2-D2-Assistant.git](https://github.com/CinarSamet/Smart-R2-D2-Assistant.git)
   cd Smart-R2-D2-Assistant/server
   ```

2. Install the required Python libraries:
   ```bash
   pip install -r requirements.txt
   ```

3. Add your Gemini API key as an environment variable:
   ```bash
   export GEMINI_API_KEY="your_api_key"
   ```

4. Start the server:
   ```bash
   python app.py
   ```

### 2. Hardware (ESP32) Setup
1. Open the Arduino IDE and install the necessary libraries (`WiFiManager`, `ArduinoOTA`, etc.).
2. Update the `serverUrl` variable in the `esp32/main.ino` file with the local IP address of the computer running the server (e.g., `http://192.168.1.X:5001/upload`).
3. Upload the code to the ESP32. 
4. On its first boot, the device will create a Wi-Fi network named `R2D2_Kurulum`. Connect to this network to configure your local internet settings on the device.

## 🗺️ Roadmap
- [x] Establishing the I2S audio pipeline on the ESP32.
- [x] Ensuring closed-loop communication with Whisper, Gemini, and TTS integration.
- [x] Creating the hardware State Machine structure and OLED interface.
- [ ] **Dockerization:** Containerizing the entire Flask/AI server infrastructure using Docker to make it environment-independent and deployable.
- [ ] **Sensor Fusion:** Adding autonomous movement capabilities by integrating IMU data and distance sensors.

## 📄 License
The software codes in this repository (ESP32 and Python) are licensed under the **MIT License**. See the `LICENSE` file for more details.

The 3D hardware designs (STL files) of the project are subject to the **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)** license. Reproduction and sale for commercial purposes are prohibited. You are free to use and develop them in your personal projects.
