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
#include "sensors/device_accelerometer_sensor.h"

#include <android/looper.h>
#include <android/sensor.h>
#include <stddef.h>

#include <memory>
#include <mutex>  // NOLINT

#include "util/logging.h"
#include "hid_sensor.h"
#include "../accelerometer_data.h"

// Workaround to avoid the inclusion of "android_native_app_glue.h.
#ifndef LOOPER_ID_USER
#define LOOPER_ID_USER 3
#endif


namespace cardboard {
HidSensor *asensor = nullptr;
// This struct holds android specific sensor information.
struct DeviceAccelerometerSensor::SensorInfo {
    SensorInfo() {
    }
};

DeviceAccelerometerSensor::DeviceAccelerometerSensor()
    : sensor_info_(new SensorInfo()) {
}

DeviceAccelerometerSensor::~DeviceAccelerometerSensor() {}

static int a_num = 0;
void DeviceAccelerometerSensor::PollForSensorData(
    int timeout_ms, std::vector<AccelerometerData>* results) const {

  unsigned char USB_BUF[32];
  float usb_accel_data[3]={0};
  int ret;
  results->clear();

   do {
    ret = asensor->startReading(USB_BUF, 32, 100);
    AccelerometerData sample;
    usb_accel_data[0] =
        (int16_t)(((USB_BUF[13] << 8) & 0xFF00) | USB_BUF[14]) * ICM206XX_ACCEL_RESOLUTION_16G*G/1000;
    usb_accel_data[1] =
        (int16_t)(((USB_BUF[15] << 8) & 0xFF00) | USB_BUF[16]) * ICM206XX_ACCEL_RESOLUTION_16G*G/1000;
    usb_accel_data[2] =
        (int16_t)(((USB_BUF[17] << 8) & 0xFF00) | USB_BUF[18]) * ICM206XX_ACCEL_RESOLUTION_16G*G/1000;

    sample.sensor_timestamp_ns = asensor->getTimeNano();
    sample.system_timestamp = sample.sensor_timestamp_ns;
    sample.data = {(1.0)*usb_accel_data[2], (-1.0)*usb_accel_data[0], (-1.0)*usb_accel_data[1]};
    if (a_num%2000 == 0) {
        CARDBOARD_LOGI("a push_back sample: %f, %f, %f", sample.data[0], sample.data[1], sample.data[2]);
    }
    if (a_num == INT32_MAX) {
        a_num = 0;
    }
    a_num ++;
    results->push_back(sample);
  } while (ret > 0);
}

bool DeviceAccelerometerSensor::Start() {
  asensor = HidSensor::GetInstance();
  if (asensor != nullptr) {
      if(asensor->open()) {
          CARDBOARD_LOGI("asensor is %p, open", asensor);
          return true;
      } else {
          CARDBOARD_LOGE("asensor open failed");
          return false;
      }
  } else {
      CARDBOARD_LOGE("asensor can't get hid instance");
      return false;
  }
}

void DeviceAccelerometerSensor::Stop() {
  if (asensor != nullptr) {
      CARDBOARD_LOGI("asensor is %p, close", asensor);
      asensor->close();
  }
}

}  // namespace cardboard
