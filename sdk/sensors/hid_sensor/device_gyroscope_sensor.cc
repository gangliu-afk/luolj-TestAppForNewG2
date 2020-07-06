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
#include "sensors/device_gyroscope_sensor.h"

#include <android/looper.h>
#include <android/sensor.h>
#include <stddef.h>

#include <memory>
#include <mutex>  // NOLINT

#include "sensors/accelerometer_data.h"
#include "sensors/gyroscope_data.h"
#include "util/logging.h"
#include "hid_sensor.h"

// Workaround to avoid the inclusion of "android_native_app_glue.h.
#ifndef LOOPER_ID_USER
#define LOOPER_ID_USER 3
#endif

namespace cardboard {
    HidSensor *gsensor;
   // This struct holds android gyroscope specific sensor information.
   struct DeviceGyroscopeSensor::SensorInfo {
        SensorInfo()
                : first_gyro_value(true) {}
        // In the first frame gyro system calibration will get written to the
        // initial_system_gyro_bias_ member variable.
        bool first_gyro_value;

        // The initial System gyro bias values provided when gyro is set to
        // ASENSOR_TYPE_GYRO_UNCALIBRATED.
        static Vector3 initial_system_gyro_bias;
        static std::mutex gyro_bias_mutex;
    };

// Defines the static variable.
Vector3 DeviceGyroscopeSensor::SensorInfo::initial_system_gyro_bias = {0, 0, 0};
std::mutex DeviceGyroscopeSensor::SensorInfo::gyro_bias_mutex;

// This function returns gyroscope initial system bias
Vector3 DeviceGyroscopeSensor::GetInitialSystemBias() {
  std::lock_guard<std::mutex> lock(SensorInfo::gyro_bias_mutex);
  return SensorInfo::initial_system_gyro_bias;
}

DeviceGyroscopeSensor::DeviceGyroscopeSensor()
    : sensor_info_(new SensorInfo()) {}

DeviceGyroscopeSensor::~DeviceGyroscopeSensor() {}

static int g_num = 0;
void DeviceGyroscopeSensor::PollForSensorData(
    int timeout_ms, std::vector<GyroscopeData>* results) const {
  unsigned char USB_BUF[32];
  float usb_gyro_data[3]={0};
  int ret;
  results->clear();

  do {
    ret = gsensor->startReading2(USB_BUF, 32, 100);
    GyroscopeData sample;
    usb_gyro_data[0] =
        (int16_t)(((USB_BUF[21] << 8) & 0xFF00) | USB_BUF[22]) * ICM206XX_GYRO_SSTVT_2000DPS*PI/180;
    usb_gyro_data[1] =
        (int16_t)(((USB_BUF[23] << 8) & 0xFF00) | USB_BUF[24]) * ICM206XX_GYRO_SSTVT_2000DPS*PI/180;
    usb_gyro_data[2] =
        (int16_t)(((USB_BUF[25] << 8) & 0xFF00) | USB_BUF[26]) * ICM206XX_GYRO_SSTVT_2000DPS*PI/180;

    sample.sensor_timestamp_ns = gsensor->getTimeNano();
    sample.system_timestamp = sample.sensor_timestamp_ns;
    sample.data = {(1.0)*usb_gyro_data[2], (-1.0)*usb_gyro_data[0], (-1.0)*usb_gyro_data[1]};
    if (g_num%2000 == 0) {
        CARDBOARD_LOGI("g push_back sample: %f, %f, %f", sample.data[0], sample.data[1], sample.data[2]);
    }
    if (g_num < INT16_MAX) {
        g_num ++;
    } else {
        g_num = 0;
    }
    results->push_back(sample);
  } while (ret > 0);
}

bool DeviceGyroscopeSensor::Start() {
  gsensor = HidSensor::GetInstance();
  if (gsensor != nullptr) {
      if(gsensor->open()) {
          CARDBOARD_LOGI("gsensor is %p opened", gsensor);
          return true;
      } else {
          CARDBOARD_LOGE("gsensor open failed");
          return false;
      }
  } else {
      CARDBOARD_LOGE("gsensor can't get hid instance");
      return false;
  }
}

void DeviceGyroscopeSensor::Stop() {
  if (gsensor != nullptr) {
      CARDBOARD_LOGI("gsensor is %p close", gsensor);
      gsensor->close();
  }
}

}  // namespace cardboard