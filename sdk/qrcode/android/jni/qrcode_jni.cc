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
#include <android/log.h>
#include <jni.h>

#include <vector>

#include "qrcode/cardboard_v1/cardboard_v1.h"
#include "qr_code.h"
#include "util/logging.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_google_cardboard_qrcode_CardboardParamsUtils_##method_name

extern "C" {

JNI_METHOD(jbyteArray, nativeGetCardboardV1DeviceParams)
(JNIEnv* env, jobject obj) {
  std::vector<uint8_t> cardboard_v1_params =
      cardboard::qrcode::getCardboardV1DeviceParams();
  jbyteArray result = env->NewByteArray(cardboard_v1_params.size());
  env->SetByteArrayRegion(result, 0, cardboard_v1_params.size(),
                          (jbyte*)cardboard_v1_params.data());
  return result;
}

#ifdef HID_SENSOR
JNI_METHOD(void, nativeSetUsbFileDescriptor)
(JNIEnv* env, jobject obj, jint vid, jint pid, jint fd, jint busnum, jint devaddr, jstring usbfs_str) {
  CARDBOARD_LOGI("native set Usb file descriptor");
  const char *c_usbfs = env->GetStringUTFChars(usbfs_str, JNI_FALSE);
  cardboard::qrcode::setUsbParams(vid, pid, fd, busnum, devaddr, c_usbfs);
}

JNI_METHOD(void, nativeSetUsbClose)
(JNIEnv* env, jobject obj) {
  cardboard::qrcode::setUsbClose();
}

JNI_METHOD(void, nativeSetUsbExit)
(JNIEnv* env, jobject obj) {
  cardboard::qrcode::setUsbExit();
}
#endif
}  // extern "C"
