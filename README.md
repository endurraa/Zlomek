# 🚗 Złomek - Autonomous Tricycle Robot

![C++](https://img.shields.io/badge/Language-C++-blue)
![Platform](https://img.shields.io/badge/Platform-Arduino-00979D)
![Status](https://img.shields.io/badge/Status-Completed-success)

## 📌 Abstract
**Złomek** is a fully autonomous mobile platform featuring a unique **front-wheel drive and steering module**, inspired by aircraft landing gear. Developed as a team engineering project at the **Bialystok University of Technology**, this robot navigates unknown environments by actively scanning for obstacles and calculating optimal paths in real-time.

Unlike traditional differential-drive robots, Złomek utilizes a stepper motor for precise mechanical steering (0.088° per step) combined with a DC motor for propulsion, eliminating high-frequency vibrations typical of servo-based steering.

---

## 🧑‍💻 My Role in the Project
As part of the engineering team, my primary responsibility focused on **Quality Assurance (QA) and Technical Documentation**:
* **System Validation & Testing:** Designed and executed functional tests to verify the robot's emergency braking distance (< 45 cm) and steering precision (±3 steps error margin).
* **Technical Documentation:** Authored the comprehensive technical engineering report, ensuring all FSM logic, hardware architectures, and CE (Circular Economy) standards were accurately documented.
* **Project Cohesion:** Managed the repository structure and ensured the consistency of the final project presentation.

---

## 🛠️ Hardware Architecture & BOM

The chassis and steering column were custom-designed in SolidWorks and 3D printed using PLA and PETG plastics.

**Key Electronic Components:**
* **Microcontroller:** Arduino Uno R3
* **Steering Actuator:** Stepper Motor with ULN2003 Driver (for high-precision angular positioning)
* **Propulsion:** DC Motor with L298N H-Bridge Driver
* **Vision / Scanning:** HC-SR04 Ultrasonic Sensor mounted on an SG90 Servo (180° scanning radius)
* **Power Supply:** 2x Lithium Cells (7.9V)
<img width="825" height="733" alt="image" src="https://github.com/user-attachments/assets/115119dd-9384-4777-a712-5a9f6125870d" />
<img width="711" height="992" alt="image" src="https://github.com/user-attachments/assets/52ef4ed5-a77b-49f1-a873-099d908a9600" />

---

## 💻 Software & Control Logic

The robot operates fully autonomously using a **Finite State Machine (FSM)** written in C++. 

### 1. Sector Scanning & Path Calculation
When an obstacle is detected within the critical distance (< 45 cm), the robot triggers an emergency brake and reverses (`COFANIE` state) to create mechanical clearance. The SG90 servo sweeps the ultrasonic sensor from 30° to 150°, taking 13 distinct measurements. The algorithm calculates the "best window" (clearance > 60 cm), prioritizing the path closest to the center axis.

### 2. Counter-Steering Logic (Logika Kontry)
To prevent the robot from driving in "S" or "8" shapes after avoiding an obstacle, a custom geometric recovery algorithm is implemented. After the initial arc maneuver, the stepper motor counter-steers by exactly `-2 * alpha` to perfectly align the robot parallel to its original trajectory.

### 3. State Machine Flow
* `KALIBRACJA` -> Auto-homes the steering axis on startup.
* `JAZDA_SWOBODNA` -> Forward cruising at optimized PWM while continuously pinging forward.
* `HAMOWANIE_AWARYJNE` -> Instant stop upon detecting a threat.
* `SKANOWANIE_SEKTOROWE` -> Executes the 180° environmental sweep.

FSM flow charts:
<img width="1117" height="481" alt="obraz" src="https://github.com/user-attachments/assets/3d64cd1e-41fb-41d9-9353-64c44cafa2b9" />
<img width="622" height="981" alt="image" src="https://github.com/user-attachments/assets/7f6c48e9-dbad-4c54-beab-d5a8aae51101" />


---

## 🚀 Getting Started

1. Assemble the hardware and ensure the 7.9V battery pack is charged.
2. Upload the source code to the Arduino Uno.
3. Place the robot on the floor.
4. Turn on the power. The robot will automatically run the `KALIBRACJA` sequence (finding the center axis for the stepper motor).
5. Once calibrated, it will switch to `JAZDA_SWOBODNA` and start exploring autonomously!

---

## 👥 Team & Credits
This project was successfully built by a team of engineering students at Bialystok University of Technology:
* **Gabriel Kozłowski (Me)** - QA, Technical Documentation & System Validation
* **Szymon Kozłowski** - Team Leader, FSM Programming, 3D CAD Modeling & Integration
* **Paweł Krawczuk** - Hardware Interfaces, Electrical Wiring & Presentation
