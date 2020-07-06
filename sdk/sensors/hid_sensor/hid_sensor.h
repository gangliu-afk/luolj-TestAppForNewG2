/*
 * Copyright 2019 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CARDBOARD_SDK_SENSORS_HID_SENSOR_HID_SENSOR_H_
#define CARDBOARD_SDK_SENSORS_HID_SENSOR_HID_SENSOR_H_

#include <mutex>
#include "libs/include/libusb.h"
#include "util/vector.h"

namespace cardboard {
#ifndef G
#define G                              9.81188f
#endif

#ifndef PI
#define PI                             3.14159f
#endif

/**
 * Sensor Temprature resolution in deg Celsius/LSB
 * 1/326.8 deg Celsius per LSB
 */
#define ICM206XX_SENSOR_TEMPERATURE_RESOLUTION  (0.00306f)
#define ICM206XX_ROOM_TEMP_OFFSET                       25//Celsius degree


/**
 * Accelerometer resolutions (mg/LSB)
 */
#define ICM206XX_ACCEL_RESOLUTION_2G    (0.0610f)
#define ICM206XX_ACCEL_RESOLUTION_4G    (0.1221f)
#define ICM206XX_ACCEL_RESOLUTION_8G    (0.2441f)
#define ICM206XX_ACCEL_RESOLUTION_16G   (0.4883f)

/**
 * ICM206XX sensitivity for gyro in dps/LSB
 */
#define ICM206XX_GYRO_SSTVT_250DPS              (0.00763)
#define ICM206XX_GYRO_SSTVT_500DPS              (0.01526)
#define ICM206XX_GYRO_SSTVT_1000DPS             (0.03052)
#define ICM206XX_GYRO_SSTVT_2000DPS             (0.06104)

class HidSensor {
 public:
  static HidSensor *&GetInstance();

  static void DeleteInstance();

  void setParams(int vid, int pid, int fd, int busnum, int devaddr, const char *usbfs);

  bool init();

  bool open();

  void close();

  void exit();

  int startReading(unsigned char *buffer, int length, unsigned int timeout);

  int startReading2(unsigned char *buffer, int length, unsigned int timeout);

  uint64_t getTimeNano();

  private:
  HidSensor();
  ~HidSensor();

  HidSensor(const HidSensor &signal);

  const HidSensor &operator=(const HidSensor &signal);

private:
  static HidSensor *m_SingleInstance;
  static std::mutex m_Mutex;

  struct libusb_context *usb_ctx;
  struct libusb_device *usb_dev;
  struct libusb_device_handle *handle;
  struct libusb_config_descriptor *conf_desc;

  int interface_num;
};
}
#endif //CARDBOARD_SDK_SENSORS_HID_SENSOR_HID_SENSOR_H_
