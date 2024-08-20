# Fleet Driver Monitoring and Management using IoT

## Introduction

This project was developed as part of the IoT course at Iran University of Science and Technology. It aims to enhance fleet safety and efficiency using IoT technologies, specifically **ESP32**, **SIM800L**, **MPU6050**, and **ThingsBoard**.

## Process

### 1. Problem Identification
- Monitoring driver behavior for safe driving practices.
- Real-time vehicle location tracking.
- Environmental monitoring to protect goods.
- Sending immediate alerts in case of emergencies.

### 2. Solution Development

#### Hardware Components
- **ESP32 Microcontroller:** Central processing unit.
- **SIM800L GSM/GPRS Module:** Data transmission and SMS communication.
- **MPU6050 Accelerometer/Gyroscope:** Monitors vehicle acceleration and orientation.
- **Additional Sensors:** For future enhancements like temperature and humidity monitoring.

#### Software Components
- **Embedded C/C++ Code:** Manages data acquisition and communication.
- **AT Commands:** Controls the SIM800L for SMS and GPRS functions.
- **ThingsBoard Platform:** Visualizes data and provides analysis tools.

### 3. System Integration
- **ESP32** collects data from the **MPU6050**.
- **SIM800L** transmits data to ThingsBoard via GPRS and sends SMS alerts.
- The system monitors real-time data and alerts on ThingsBoard.

### 4. Data Transmission and Monitoring
- Data is sent to ThingsBoard via GPRS for real-time visualization.
- ThingsBoard triggers alerts and logs data for analysis.

## Contributors
-  [SanazMotie](https://github.com/sanazmotie)
-  [MehdiMoosaviun](https://github.com/tsSMehdiM)


