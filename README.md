🔋 BMS State Estimation in EV Routing Using Kalman Filter and Robust Communication

A real-time simulation prototype for estimating the State of Charge (SOC) of an Electric Vehicle (EV) battery using a Kalman Filter under sensor noise and communication disturbances.

📌 Project Overview

Battery Management Systems (BMS) play an important role in monitoring battery conditions and ensuring the safe and efficient operation of electric vehicles.

However, inaccurate sensor measurements and communication failures can affect battery state estimation, potentially leading to incorrect driving-range predictions and routing decisions.

This project presents an interactive web-based simulation that demonstrates how a Kalman Filter estimates battery State of Charge (SOC) from noisy measurements while simulating communication disturbances during an EV journey.

The prototype visualizes the vehicle's route, actual battery SOC, noisy sensor readings, and filtered SOC estimates in real time.

🎯 Objectives

- Estimate battery State of Charge (SOC) using a Kalman Filter.
- Reduce the effect of sensor measurement noise.
- Simulate communication disturbances, packet loss, and interference.
- Compare actual SOC, raw measurements, and filtered estimates.
- Visualize battery SOC changes during a simulated EV journey.
- Explore the potential use of battery state estimation in EV routing and range prediction.

✨ Key Features

1. 🔋 Battery SOC Estimation

Displays the actual simulated battery SOC and the Kalman Filter's estimated SOC.

2. 📊 Real-Time Data Visualization

Plots three data streams:

- Actual SOC: The simulated battery state.
- Raw SOC: Sensor measurements affected by noise.
- Estimated SOC: The filtered value calculated by the Kalman Filter.

3. ⚡ Sensor Noise Simulation

Introduces Gaussian noise into sensor measurements.

Users can enable high sensor noise to observe how measurement uncertainty affects the estimation process.

4. 📡 Communication Disturbance Simulation

Simulates communication problems such as:

- Packet loss represented by zero-valued measurements.
- Incorrect measurements caused by interference.
- Unstable transmitted battery data.

This feature helps demonstrate the challenges faced by a BMS when communication becomes unreliable.

5. 🗺️ EV Route Visualization

Displays a simulated EV journey through multiple waypoints, including a hill-climb section and a city section.

6. 🖥️ System Monitoring Console

Shows recent simulation readings, estimated SOC, and estimation error.

7. ▶️ Interactive Controls

- Start or pause the simulation.
- Reset the simulation.
- Enable or disable high sensor noise.
- Enable or disable communication disturbances.

🛠️ Technology Stack

Component| Technology
Frontend| React.js
Programming Language| JavaScript
User Interface| JSX
Styling| Tailwind CSS utility classes
Icons| Lucide React
State Management| React Hooks
Data Visualization| SVG
Estimation Algorithm| 1D Kalman Filter
Noise Model| Gaussian Random Noise
Simulation| JavaScript-based EV and battery model

⚙️ System Architecture

        EV Battery Simulation
                 |
                 v
       Actual Battery SOC
                 |
                 v
        Simulated Sensors
                 |
                 v
          Sensor Noise
                 |
                 v
       Communication Channel
                 |
       Packet Loss / Interference
                 |
                 v
        Received SOC Measurement
                 |
                 v
           Kalman Filter
                 |
                 v
       Estimated Battery SOC
                 |
        +--------+---------+
        |                  |
        v                  v
   SOC Comparison     Real-Time Graph
        |                  |
        +--------+---------+
                 |
                 v
       EV Routing Visualization

Note: This architecture represents the simulation workflow. The current prototype does not implement a physical BMS, a real CAN/LIN network, or an operational EV routing algorithm.

🧮 Kalman Filter Implementation

The project uses a one-dimensional Kalman Filter to estimate the battery SOC from noisy measurements.

The filter consists of two main stages.

1. Prediction

The filter predicts the next state using the previous estimate.

[
\hat{x}{k}^{-}=A\hat{x}{k-1}+Bu_k
]

The predicted estimation uncertainty is:

[
P_k^{-}=AP_{k-1}A^T+Q
]

2. Update

The Kalman Gain is calculated as:

[
K_k=\frac{P_k^{-}C^T}{CP_k^{-}C^T+R}
]

The estimated state is updated using the new measurement:

[
\hat{x}_k=\hat{x}_k^{-}+K_k(z_k-C\hat{x}_k^{-})
]

The estimation covariance is updated as:

[
P_k=P_k^{-}-K_kCP_k^{-}
]

Where:

- x = Estimated battery SOC.
- z = Noisy SOC measurement.
- P = Estimation error covariance.
- Q = Process noise covariance.
- R = Measurement noise covariance.
- K = Kalman Gain.
- A = State transition coefficient.
- B = Control input coefficient.
- C = Measurement coefficient.
- u = Control input.

In the current simulation, the filter uses the initial parameters R=0.1 and Q=0.1.

The Kalman Filter reduces measurement fluctuations by balancing the predicted state against the received measurement.

📡 Robust Communication Concept

Reliable communication is important for transmitting battery measurements and status information in an EV.

This prototype simulates communication disturbances to demonstrate their potential effect on SOC estimation.

When communication instability is enabled, the simulation introduces occasional zero-valued measurements and abnormal readings.

Important limitation: The current implementation does not include a complete robust communication protocol. It does not implement CAN, LIN, message authentication, retransmission, packet sequence checking, or a dedicated packet-loss recovery algorithm.

These can be incorporated in future hardware-integrated versions.

🚀 Getting Started

Prerequisites

Install the following software:

- Node.js
- npm
- A modern web browser
- A code editor such as Visual Studio Code

Installation

Step 1: Clone the repository

git clone https://github.com/YOUR_USERNAME/YOUR_REPOSITORY.git

Replace "YOUR_USERNAME" and "YOUR_REPOSITORY" with your GitHub username and repository name.

Step 2: Navigate to the project folder

cd YOUR_REPOSITORY

Step 3: Install dependencies

If your project uses Vite:

npm install
npm install lucide-react

Step 4: Start the development server

For a Vite-based React project:

npm run dev

Open the local address displayed in your terminal.

Note: The supplied code is a React component. To run it, place it inside a React application, such as a Vite project, and configure Tailwind CSS because the interface uses Tailwind utility classes.

📈 Expected Results

When the simulation runs, users can observe:

- The simulated battery SOC decreasing as the journey progresses.
- Raw sensor readings fluctuating because of measurement noise.
- The Kalman Filter producing a smoother SOC estimate.
- Larger deviations when communication disturbances introduce abnormal measurements.
- Changes in estimation accuracy under different noise conditions.

The results depend on the simulation parameters and randomly generated measurements.

🌍 Applications

- Electric vehicle battery monitoring research.
- Battery State of Charge estimation.
- EV range prediction research.
- Battery data visualization.
- Sensor noise analysis.
- Communication reliability studies.
- Educational demonstrations of Kalman Filters.
- Future integration with EV routing and charging systems.

🔮 Future Enhancements

The following improvements are planned for future development:

1. Hardware Integration: Connect an ESP32 or another microcontroller to real battery sensors.
2. Real Battery Data: Measure voltage, current, and temperature using appropriate sensors.
3. Advanced SOC Estimation: Develop a battery model using current measurements and Coulomb counting alongside Kalman filtering.
4. Robust Communication: Integrate CAN communication and investigate LIN where appropriate.
5. Packet Validation: Add message timeouts, sequence numbers, plausibility checks, and stale-data detection.
6. Fault Handling: Detect sensor faults and reject or down-weight abnormal measurements.
7. EV Range Prediction: Estimate the remaining driving range using SOC and energy consumption.
8. Intelligent Routing: Investigate route selection based on battery SOC, road conditions, energy consumption, and charging-station availability.
9. Hardware-in-the-Loop Testing: Evaluate the estimation algorithm with real sensor data and communication hardware.

⚠️ Limitations

- This is a software simulation, not a production-ready BMS.
- Battery SOC is simulated rather than measured from a real battery.
- The Kalman Filter uses a simplified one-dimensional model.
- Battery voltage, current, temperature, and electrochemical behavior are not modeled in detail.
- Communication disturbances are simulated rather than transmitted through a physical network.
- The interface does not calculate an optimal EV route or locate real charging stations.
- The current implementation does not establish safety certification or validate battery protection functions.

🎓 Project Significance

This project demonstrates how Kalman Filtering can support battery SOC estimation when sensor readings are noisy and communication disturbances occur.

It provides a foundation for further research into reliable battery monitoring, EV energy management, remaining-range prediction, and intelligent routing.

The long-term objective is to develop a hardware-integrated system that combines battery state estimation with reliable communication and energy-aware EV routing.

🌱 Sustainable Development Goals (SDGs)

This project is relevant to the following United Nations Sustainable Development Goals:

- SDG 7 – Affordable and Clean Energy: Supports research into efficient battery utilization and electric mobility.
- SDG 9 – Industry, Innovation and Infrastructure: Encourages innovation in battery monitoring, estimation algorithms, and EV technology.
- SDG 13 – Climate Action: Supports the development of technologies that can contribute to the adoption of electric vehicles and reduced transport emissions.

👨‍💻 Contributors

Add your project team members here.

- Project Team: Add your team name.
- Institution: Sri Sairam Engineering College.
- Department: Electrical and Electronics Engineering (EEE).

📄 License

This project can be released under the MIT License. Add a "LICENSE" file to the repository if you choose this license.

⭐ Support

If you find this project useful for learning about battery SOC estimation, Kalman Filtering, and EV energy management, consider giving the repository a star.

---

Project Title: State Estimation of BMS in EV Routing Using Kalman Filter and Robust Communication

Project Type: Electric Vehicle Battery Management System Simulation

Status: Simulation Prototype
