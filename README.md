# 2023-2027-Mini-Projects
B.E CSE (IOT &amp; CYBER SECURITY INCLUDING BLOCK CHAIN TECHNOLOGY) 2023-2027-Mini Projects

Secure Short-Range Communication Using ESP-NOW AES Encryption on ESP32 SoC
A lightweight, secure, and energy-efficient IoT communication system that combines a custom AES-based encryption mechanism, dynamic key generation, ESP-NOW wireless communication, and ESP32 deep sleep optimization for secure short-range data transmission.

Overview
Internet of Things (IoT) devices often require secure communication while operating under strict resource constraints such as limited power, memory, and processing capability. Traditional communication methods frequently depend on internet connectivity, introducing additional latency, power consumption, and security risks.

This project presents a secure short-range communication framework based on ESP32 microcontrollers and the ESP-NOW protocol. The system implements a lightweight AES-128 encryption algorithm with dynamic key generation, ensuring that every transmitted packet is encrypted using a unique session key. Deep sleep functionality is incorporated to significantly reduce energy consumption during idle periods.

System Architecture
The communication framework consists of three nodes:

Sender
Reads gas sensor data.
Generates a dynamic session key.
Encrypts the sensor data using the modified AES algorithm.
Broadcasts the encrypted packet through ESP-NOW.
Enters deep sleep mode after transmission.
Receiver
Receives encrypted packets.
Reconstructs the session key using the packet counter.
Decrypts the received data.
Displays the original sensor readings.
Invader
Intercepts ESP-NOW packets.
Can only view encrypted ciphertext.
Cannot recover plaintext without the secret base key.
Dynamic Key Generation
Unlike traditional AES implementations that use a fixed key, this system generates a unique key for every packet.

A pre-shared 16-byte base key K₀ is stored on both sender and receiver devices.

For packet number n, the session key is generated as:

Kn[i] = K0[i] + n
where:

K0[i] = i-th byte of the base key
n = packet counter
Kn[i] = i-th byte of the session key
The packet counter is transmitted along with the ciphertext, allowing the receiver to reconstruct the same session key without requiring additional key exchange.

Security Advantage
If a session key is compromised, only a single packet (16 bytes) can be decrypted. All previous and future packets remain protected because each packet uses a different encryption key.

Lightweight AES Implementation
To reduce computational overhead on resource-constrained ESP32 devices, the MixColumns operation was optimized.

Optimized MixColumns
uint8_t t = b0 ^ b1 ^ b2 ^ b3;

s[i]    = b0 ^ t ^ xtime(b0 ^ b1);
s[i+4]  = b1 ^ t ^ xtime(b1 ^ b2);
s[i+8]  = b2 ^ t ^ xtime(b2 ^ b3);
s[i+12] = b3 ^ t ^ xtime(b3 ^ b0);
Benefits
Reduced Galois Field computations
Lower CPU utilization
Faster encryption
Better suitability for IoT devices
Power Optimization
The ESP32 employs a timer-based deep sleep mechanism.

Operating Cycle
Wake Up
   ↓
Read Sensor
   ↓
Encrypt Data
   ↓
Transmit via ESP-NOW
   ↓
Enter Deep Sleep
The device remains asleep for most of its lifetime and only wakes when a transmission is required.

Experimental Results
Throughput Comparison
Metric	Proposed System
Average Throughput Gain	6.50%
End-to-End Latency
Distance	Latency
1–10 m	~12.84 ms
400 m	~89.60 ms
Power Consumption
Sleep Interval	Average Power
Continuous (1 sec)	32.540 mW
30 sec	2.047 mW
60 sec	1.297 mW
120 sec	0.607 mW
300 sec	0.278 mW
Power Saving
At a sleep interval of 300 seconds:

Power Reduction = 99.1%
compared to continuous operation.

Hardware Requirements
2 × ESP32 Development Boards
MQ-2 or MQ-135 Gas Sensor
ACS712 Current Sensor
USB Power Supply
Arduino IDE
Software Requirements
Arduino IDE
ESP32 Arduino Core
ESP-NOW Protocol Support
C/C++
LaTeX (for result visualization)
Limitations
Designed only for short-range communication.
Communication range is limited by ESP-NOW capabilities.
Requires pre-shared base keys between sender and receiver.
Applications
Smart monitoring systems
Environmental sensing
Industrial IoT
Smart agriculture
Battery-powered sensor networks
Home automation
Authors
Dr. Namita Parati
Vanamala Srithan
G. Lokesh Reddy
B. Satyadhoni
Department of CSE-ALLIED MVSR Engineering College Hyderabad, Telangana, India

Citation
If you use this work in your research, please cite the corresponding publication.

License
This project is intended for academic and research purposes.
