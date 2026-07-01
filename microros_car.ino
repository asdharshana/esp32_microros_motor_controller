#include <micro_ros_arduino.h>
#include <WiFi.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <geometry_msgs/msg/twist.h>

// Macro to catch micro-ROS initialization errors and flash on serial
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){Serial.print("Failed status on line ");Serial.print(__LINE__);Serial.print(": ");Serial.println((int)temp_rc); while(1){delay(500);}}}

//==================== WiFi ====================

char ssid_c[] = "my_wifi";
char password_c[] = "123456798";

char agent_ip_c[] = "10.165.240.56";
uint32_t agent_port = 8888;

//==================== L298N Pins ====================

// Left Motor
#define IN1 25
#define IN2 26

// Right Motor
#define IN3 27
#define IN4 14

// PWM Enable Pins
#define ENA 32
#define ENB 33

//==================== PWM ====================

#define PWM_FREQ 1000
#define PWM_RES 8

//==================== micro-ROS ====================

rcl_subscription_t subscriber;
rclc_executor_t executor;
rcl_node_t node;
rcl_allocator_t allocator;
rclc_support_t support;

geometry_msgs__msg__Twist twist_msg;

//====================================================

void stopMotors()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}

//====================================================

void setMotorLeft(int pwm)
{
  pwm = constrain(pwm, -255, 255);

  if (pwm > 0)
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    ledcWrite(ENA, pwm);
  }
  else if (pwm < 0)
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    ledcWrite(ENA, -pwm);
  }
  else
  {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    ledcWrite(ENA, 0);
  }
}

//====================================================

void setMotorRight(int pwm)
{
  pwm = constrain(pwm, -255, 255);

  if (pwm > 0)
  {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    ledcWrite(ENB, pwm);
  }
  else if (pwm < 0)
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    ledcWrite(ENB, -pwm);
  }
  else
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    ledcWrite(ENB, 0);
  }
}

//====================================================

void cmd_vel_callback(const void *msgin)
{
  const geometry_msgs__msg__Twist *msg =
      (const geometry_msgs__msg__Twist *)msgin;

  float linear = msg->linear.x;
  float angular = msg->angular.z;

  //--------------------------------------------------
  // Differential Drive Kinematics
  //--------------------------------------------------

  float leftWheel = linear - angular;
  float rightWheel = linear + angular;

  //--------------------------------------------------
  // Normalize
  //--------------------------------------------------

  float maxValue = max(abs(leftWheel), abs(rightWheel));

  if (maxValue > 1.0)
  {
    leftWheel /= maxValue;
    rightWheel /= maxValue;
  }

  //--------------------------------------------------
  // Convert to PWM
  //--------------------------------------------------

  int leftPWM = (int)(leftWheel * 255.0);
  int rightPWM = (int)(rightWheel * 255.0);

  //--------------------------------------------------
  // Drive Motors
  //--------------------------------------------------

  setMotorLeft(leftPWM);
  setMotorRight(rightPWM);

  //--------------------------------------------------
  // Debug
  //--------------------------------------------------

  Serial.print("[CALLBACK] Linear X: ");
  Serial.print(linear);
  Serial.print(" | Angular Z: ");
  Serial.print(angular);
  Serial.print(" | PWM L/R: ");
  Serial.print(leftPWM);
  Serial.print("/");
  Serial.println(rightPWM);
}

//====================================================

void setup()
{
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to catch up

  Serial.println("\n--- BOOTING ESP32 MOTOR CONTROLLER ---");

  //---------------- Motor Pins ----------------

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  //---------------- PWM ----------------

  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);

  stopMotors();
  Serial.println("[INFO] Motors initialized and stopped.");

  //---------------- Wi-Fi Status ----------------

  Serial.print("[WIFI] Connecting to SSID: ");
  Serial.println(ssid_c);
  
  WiFi.begin(ssid_c, password_c);
  int wifi_timeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_timeout < 20) {
    delay(500);
    Serial.print(".");
    wifi_timeout++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected successfully!");
    Serial.print("[WIFI] ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] CRITICAL ERROR: Failed to connect to Wi-Fi. Check your router/credentials!");
  }

  //---------------- micro-ROS ----------------

  Serial.print("[uROS] Target Agent IP: ");
  Serial.print(agent_ip_c);
  Serial.print(" | Port: ");
  Serial.println(agent_port);

  set_microros_wifi_transports(
      ssid_c,
      password_c,
      agent_ip_c,
      agent_port);

  Serial.println("[uROS] Wi-Fi transport layer assigned. Waiting 2 seconds...");
  delay(2000);

  allocator = rcl_get_default_allocator();

  Serial.println("[uROS] Initializing support context...");
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  Serial.println("[uROS] Initializing node...");
  RCCHECK(rclc_node_init_default(&node, "esp32_motor_controller", "", &support));

  Serial.println("[uROS] Initializing subscription to '/cmd_vel'...");
  RCCHECK(rclc_subscription_init_default(
      &subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
      "cmd_vel"));

  Serial.println("[uROS] Creating executor...");
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));

  Serial.println("[uROS] Attaching callback function...");
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &twist_msg, &cmd_vel_callback, ON_NEW_DATA));

  Serial.println("\n==============================================");
  Serial.println("[SUCCESS] micro-ROS stack successfully running!");
  Serial.println("Listening for incoming /cmd_vel messages...");
  Serial.println("==============================================\n");
}

//====================================================

void loop()
{
  rcl_ret_t ret = rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
  
  // Optional warning: Alert on serial monitor if the loop spin routine fails
  if (ret != RCL_RET_OK) {
    Serial.print("[WARN] Executor spin failed with status code: ");
    Serial.println((int)ret);
  }
  
  delay(10);
}