This Arduino code manages an IoT Hydroponics Tower, monitoring water and pH levels and automating a pump, with control and feedback provided via the Blynk platform.

---

### **How the Code Works**

At its core, the system continuously reads **water level** and **pH sensors**. These readings are then sent to the **Blynk mobile application** and displayed on a local **LCD screen**. The code is designed for non-blocking operations, meaning it performs tasks (like reading sensors or updating Blynk) at set intervals without pausing the entire program.

---

### **Key Components and Features**

* **ESP32 Microcontroller:** The brains of the operation, handling sensor readings, Wi-Fi communication, and control logic.
* **Water Level Sensor:** Measures the water level in the hydroponics reservoir.
* **pH Sensor:** Measures the acidity or alkalinity of the water.
* **Relay Module:** Controls an external pump (e.g., for refilling water or adjusting pH).
* **I2C LCD Display:** Provides local, real-time feedback on system status and sensor readings.
* **Blynk Platform:** A user-friendly interface for remote monitoring, control, and notifications.

---

### **Blynk Setup and Functionality**

The system integrates seamlessly with Blynk, offering:

* **Dashboard View:** Displays real-time water level and pH readings, as well as the pump's current status.
* **Pump Control (VPIN\_PUMP\_CONTROL):** A button in the Blynk app allows you to manually turn the pump ON or OFF.
* **Auto/Manual Mode Switch (VPIN\_AUTO\_MODE):**
    * **Auto Mode:** When active, the system automatically controls the pump based on the water level threshold. If the water level drops below a predefined `lowWaterThreshold`, the pump turns ON. Otherwise, it stays OFF. This is the **default mode**.
    * **Manual Mode:** When selected, the pump's operation is solely determined by the manual pump control button in the Blynk app, overriding the automatic logic.
* **Water Level Threshold Slider (VPIN\_THRESHOLD):** A slider in the Blynk app lets you set the specific water level value that triggers the pump in auto mode.
* **Notifications:** The system can send a "low\_water" event notification to your Blynk app if the water level falls below the set threshold while in auto mode and the pump is running.
* **Synchronization:** Upon connecting to Blynk, the app automatically syncs the current state of the pump, auto mode setting, and water level threshold, ensuring the app always reflects the actual device status.

---

### **Auto Mode Explained**

The `autoMode` boolean variable determines the system's operational logic.

* When `autoMode` is `true`:
    * The `updatePumpControl()` function is called regularly.
    * Inside `updatePumpControl()`, the current `level` (water level) is compared to `lowWaterThreshold`.
    * If `level < lowWaterThreshold`, the relay is activated (`digitalWrite(RELAY_PIN, LOW)`), turning the pump **ON**.
    * If `level >= lowWaterThreshold`, the relay is deactivated (`digitalWrite(RELAY_PIN, HIGH)`), turning the pump **OFF**.
    * Manual pump control via Blynk is ignored.

* When `autoMode` is `false`:
    * The `updatePumpControl()` function is **not** called.
    * The `digitalWrite(RELAY_PIN, manualPumpState ? LOW : HIGH);` line directly controls the pump based on the `manualPumpState` variable, which is updated by the Blynk manual pump control button.

---

### **Setup and Loop Functions**

* **`setup()`:** Initializes serial communication, I2C for the LCD, sets up sensor and relay pins, connects to Wi-Fi, and establishes the initial connection to Blynk. It also initializes the LCD display.
* **`loop()`:** This is the heart of the program, running continuously. It performs the following at defined intervals:
    * Runs the Blynk client to maintain communication.
    * Reads sensor data (water level and pH).
    * Calls `updatePumpControl()` if `autoMode` is enabled.
    * Updates the Blynk app with current sensor values and pump status.
    * Updates the local LCD display with system information.

This setup allows for a robust and responsive hydroponics monitoring and automation system.