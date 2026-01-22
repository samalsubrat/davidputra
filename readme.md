# Smart Wireless Robotic Vehicle 🚗🤖

A smart Arduino-based robotic vehicle that integrates **gesture control**, **mobile control**, **obstacle detection**, and **automotive-style LED feedback** using wireless communication and embedded systems.

---

## 📌 Project Overview

This project demonstrates a **smart wireless robotic vehicle** controlled using:
- A **gesture-based transmitter**
- **Bluetooth mobile control**
- Real-time **obstacle-aware safety logic**
- Dynamic **NeoPixel LED lighting effects**

The system is inspired by modern automotive behavior, focusing on **safety, responsiveness, and visual feedback**.

---

## 🧠 System Architecture

The project consists of two main units:

### 1️⃣ Transmitter Unit
- Arduino Nano
- MPU6050 (gesture sensing)
- NRF24L01 (wireless transmission)

### 2️⃣ Receiver Unit
- Arduino Uno
- NRF24L01 (wireless reception)
- HC-05 Bluetooth module (mobile control)
- TB6612FNG motor driver
- Ultrasonic sensor (HC-SR04)
- NeoPixel RGB LED strip

---

## ⚙️ Key Features

- Gesture-based control using MPU6050
- Mobile phone control via Bluetooth
- Dual wireless input (NRF + Bluetooth)
- Obstacle-aware forward movement
- Free backward and turning motion
- Automotive-style LED indicators
- RGB flowing effect during reverse
- Non-blocking real-time operation

---

## 🚦 Control Logic

| Command | Action |
|-------|-------|
| `W` | Move Forward |
| `S` | Move Backward |
| `A` | Turn Left |
| `D` | Turn Right |
| `X` | Stop |

- Obstacle detection is applied **only during forward movement**
- Backward motion is always allowed

---

## 💡 Lighting Behavior (NeoPixel)

- **White center LEDs** → Normal operation
- **Orange sliding indicators** → Left / Right turns
- **RGB flowing effect** → Backward movement
- **All LEDs OFF** → Stop state

---

## 🛡️ Safety Mechanism

- Front-mounted ultrasonic sensor continuously measures distance
- Automatically stops the vehicle when an obstacle is detected in front
- Prevents collision without blocking reverse or turning actions

---

## 🔧 Technologies Used

- Arduino Uno & Nano
- C / C++ (Arduino framework)
- NRF24L01 Wireless Module
- HC-05 Bluetooth Module
- MPU6050 Sensor
- HC-SR04 Ultrasonic Sensor
- TB6612FNG Motor Driver
- Adafruit NeoPixel Library

---

## 🚀 Applications

- Smart robotics
- Gesture-controlled systems
- Autonomous vehicle prototypes
- Embedded systems learning
- Automotive electronics simulation

---

## 🔮 Future Enhancements

- Rear obstacle detection
- Mobile app-based UI
- Brake and hazard lighting logic
- Speed-based LED animations
- ESP32-based implementation

---

## 🎥 Project Demonstration Video

📽️ **Watch the complete working demo here:**  
👉 *[Click Here](https://youtu.be/7BjSHVF1QDA)*

Figma Link: [Click Here](https://www.figma.com/design/feEBxf5zE0kvkO5WWOrKDI/David-Putra-Presentation?node-id=406-7&t=f6Fo8opWjvfI0Ozm-1)

---

## 🙌 Acknowledgements

This project was developed as part of an academic embedded systems and robotics initiative, demonstrating real-time control, sensing, and wireless communication.

---

## 📬 Contact

For questions or collaboration, feel free to reach out.

---
