# ESP32 micro-ROS Differential Drive Motor Controller

A simple ESP32-based differential drive motor controller using **micro-ROS** and the **L298N motor driver**. The ESP32 connects to a ROS 2 network over Wi-Fi, subscribes to the `/cmd_vel` topic, and drives two DC motors based on velocity commands.

## Features

* micro-ROS communication over Wi-Fi
* ROS 2 `/cmd_vel` subscriber
* Differential drive kinematics
* PWM motor speed control
* Bidirectional motor control using L298N
* Control using `teleop_twist_keyboard`
* Docker-based ROS 2 environment

## Hardware

* ESP32 Development Board
* L298N Motor Driver
* Two DC Motors
* Power Supply
* Wi-Fi Network

## Software Requirements

* Docker
* Arduino IDE
* `micro_ros_arduino` library

## Pin Configuration

| Signal    | GPIO |
| --------- | ---- |
| IN1       | 25   |
| IN2       | 26   |
| IN3       | 27   |
| IN4       | 14   |
| ENA (PWM) | 32   |
| ENB (PWM) | 33   |

## ROS 2 Interface

**Subscribed Topic:** `/cmd_vel`

**Message Type:** `geometry_msgs/msg/Twist`

## Setup

### 1. Configure the ESP32

Update the following values in the Arduino sketch:

```cpp
char ssid_c[] = "YOUR_WIFI";
char password_c[] = "YOUR_PASSWORD";
char agent_ip_c[] = "YOUR_PC_IP";
uint32_t agent_port = 8888;
```

Upload the sketch to the ESP32.

---

### 2. Pull the Docker Image

```bash
docker pull microros/micro-ros-agent:jazzy
```

---

### 3. Run the micro-ROS Agent

```bash
docker run --rm -it --net=host microros/micro-ros-agent:jazzy udp4 --port 8888
```

---

### 4. Start a ROS 2 Container

Run a ROS 2 jazzy container:

```bash
docker run --rm -it --net=host osrf/ros:jazzy-desktop
```

Inside the container:

```bash
apt update
apt install -y ros-jazzy-teleop-twist-keyboard
source /opt/ros/jazzy/setup.bash
```

---

### 5. Launch Keyboard Teleoperation

```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

Keyboard controls:

```text
Moving around:
   u    i    o
   j    k    l
   m    ,    .

i : Forward
, : Backward
j : Rotate Left
l : Rotate Right
k : Stop
```

The `teleop_twist_keyboard` node publishes `geometry_msgs/msg/Twist` messages on `/cmd_vel`, which the ESP32 receives and converts into motor commands.

## Repository Structure

```text
.
├── esp32_motor_controller.ino
└── README.md
```

## Future Improvements

* Wheel encoder integration
* PID speed control
* Odometry publishing
* TF support
* Battery monitoring
* Integration with Nav2 and SLAM

## License

This project is licensed under the MIT License.
# ESP32 micro-ROS Differential Drive Motor Controller

A simple ESP32-based differential drive motor controller using **micro-ROS** and the **L298N motor driver**. The ESP32 connects to a ROS 2 network over Wi-Fi, subscribes to the `/cmd_vel` topic, and drives two DC motors based on velocity commands.

## Features

* micro-ROS communication over Wi-Fi
* ROS 2 `/cmd_vel` subscriber
* Differential drive kinematics
* PWM-based motor speed control
* Bidirectional motor control using L298N
* Compatible with **teleop_twist_keyboard**

## Hardware

* ESP32 Development Board
* L298N Motor Driver
* Two DC Motors
* Power Supply
* Wi-Fi Network

## Software Requirements

* ROS 2
* Arduino IDE
* micro_ros_arduino library
* micro-ROS Agent
* teleop_twist_keyboard

## Pin Configuration

| Signal    | GPIO |
| --------- | ---- |
| IN1       | 25   |
| IN2       | 26   |
| IN3       | 27   |
| IN4       | 14   |
| ENA (PWM) | 32   |
| ENB (PWM) | 33   |

## ROS 2 Interface

**Subscribed Topic**

```text
/cmd_vel
```

**Message Type**

```text
geometry_msgs/msg/Twist
```

## How It Works

1. ESP32 connects to the configured Wi-Fi network.
2. Connects to the micro-ROS Agent.
3. Subscribes to the `/cmd_vel` topic.
4. Converts linear and angular velocities into left and right wheel speeds.
5. Generates PWM signals to control the motors through the L298N driver.

## Setup

### 1. Configure Wi-Fi and Agent

Update the following values in the Arduino sketch:

```cpp
char ssid_c[] = "YOUR_WIFI";
char password_c[] = "YOUR_PASSWORD";
char agent_ip_c[] = "YOUR_AGENT_IP";
uint32_t agent_port = 8888;
```

### 2. Upload the Code

Flash the sketch to the ESP32 using the Arduino IDE.

### 3. Start the micro-ROS Agent

On your ROS 2 machine, run:

```bash
ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
```

### 4. Start Keyboard Teleoperation

In a new terminal, run:

```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

Use the keyboard controls to publish velocity commands to `/cmd_vel`.

```
Moving around:
   u    i    o
   j    k    l
   m    ,    .

i : Forward
, : Backward
j : Rotate Left
l : Rotate Right
k : Stop
```

As the `teleop_twist_keyboard` node publishes `geometry_msgs/Twist` messages, the ESP32 receives them and drives the motors accordingly.

## Repository Structure

```text
.
├── esp32_motor_controller.ino
└── README.md
```

## Future Improvements

* Wheel encoder integration
* PID speed control
* Odometry publishing
* TF support
* Battery monitoring
* ROS 2 parameter support

## License

This project is licensed under the MIT License.
