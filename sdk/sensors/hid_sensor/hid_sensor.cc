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
#include "hid_sensor.h"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "util/logging.h"

namespace cardboard {
HidSensor *HidSensor::m_SingleInstance = nullptr;
std::mutex HidSensor::m_Mutex;

static int mVid, mPid, mFd, mBusnum, mDevaddr;
static const char *mUsbfs;
static bool opened;

HidSensor *&HidSensor::GetInstance()
{
    if (m_SingleInstance == nullptr)
     {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_SingleInstance == nullptr)
        {
            m_SingleInstance = new (std::nothrow) HidSensor;
        }
     }
   return m_SingleInstance;
}

void HidSensor::DeleteInstance()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    if (m_SingleInstance)
    {
        delete m_SingleInstance;
        m_SingleInstance = nullptr;
    }
}

HidSensor::HidSensor()
    :interface_num(-1),
    usb_ctx(nullptr),
    usb_dev(nullptr),
    handle(nullptr),
    conf_desc(nullptr){}

HidSensor::~HidSensor(){
    if (conf_desc != nullptr) {
        conf_desc = nullptr;
    }
    if (handle != nullptr) {
        handle = nullptr;
    }
    if (usb_dev != nullptr) {
        usb_dev = nullptr;
    }
    if (usb_ctx != nullptr) {
        libusb_exit(usb_ctx);
        usb_ctx = nullptr;
    }
    opened = false;
}

void HidSensor::setParams(int vid, int pid, int fd, int busnum, int devaddr, const char *usbfs) {
    mVid = vid;
    mPid = pid;
    mFd = fd;
    mBusnum = busnum;
    mDevaddr = devaddr;
    mUsbfs = strdup(usbfs);
    CARDBOARD_LOGI("setParams: usbfs = %s", mUsbfs);
}

bool HidSensor::init() {
    int res;
    if (mUsbfs && strlen(mUsbfs) > 0) {
        // ret = libusb_init(&usb_ctx);
        res = libusb_init2(&usb_ctx, mUsbfs);
        if (res == LIBUSB_SUCCESS) {
            CARDBOARD_LOGI("libusb_init success");
            return true;
        } else {
            CARDBOARD_LOGE("libusb_init failed");
            return false;
        }
    } else {
        CARDBOARD_LOGE("usbfs incorrect");
        return false;
    }
}

bool HidSensor::open() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    CARDBOARD_LOGI("HidSensor opened ? %d", opened);
    if (opened) {
        CARDBOARD_LOGI("libusb_already opened");
        return true;
    }
    usb_dev = libusb_get_device_with_fd(usb_ctx, mVid, mPid, nullptr, mFd, mBusnum, mDevaddr);
    if ((!opened) && (usb_dev != nullptr)) {
        struct libusb_device_descriptor desc;
        int res;
        res = libusb_open(usb_dev, &handle);
        if (res == LIBUSB_SUCCESS) {
            CARDBOARD_LOGI("libusb_open success");
            libusb_get_device_descriptor(usb_dev, &desc);
            res = libusb_get_active_config_descriptor(usb_dev, &conf_desc);
            if (res < 0)
                res = libusb_get_config_descriptor(usb_dev, 0, &conf_desc);
            if (res == LIBUSB_SUCCESS) {
                CARDBOARD_LOGI("libusb_get_config_descriptor success");
                if (conf_desc) {
                    for (int j = 0; j < conf_desc->bNumInterfaces; j++) {
                        const struct libusb_interface *intf = &conf_desc->interface[j];
                        for (int k = 0; k < intf->num_altsetting; k++) {
                            const struct libusb_interface_descriptor *intf_desc;
                            intf_desc = &intf->altsetting[k];
                            if (intf_desc->bInterfaceClass == LIBUSB_CLASS_HID) {
                                interface_num = intf_desc->bInterfaceNumber;
                                CARDBOARD_LOGI("IMU sensor found, interface_num is %d",
                                             interface_num);
                                unsigned char data[256];
                                int detached = 0;
                                res = libusb_set_auto_detach_kernel_driver(handle, 1);
                                res = libusb_claim_interface(handle, interface_num);
                                if (res >= 0) {
                                    opened = true;
                                } else {
                                    CARDBOARD_LOGE("libusb_claim_interface failed");
                                }
                            } /* find class HID */
                        }
                    }
                }
            } else {
                CARDBOARD_LOGE("libusb_get_config_descriptor failed");
                libusb_free_config_descriptor(conf_desc);
            }
        } else {
            CARDBOARD_LOGE("libusb_open failed");
            libusb_close(handle);
        }
    }
    CARDBOARD_LOGI("HidSensor opened ? %d", opened);
    return opened;
}



void HidSensor::close() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    int res;
    if (!opened) {
        CARDBOARD_LOGI("libusb_already opened");
        return;
    }
    if (opened) {
        if (interface_num > 0) {
            /* Release the interface */
            res = libusb_release_interface(handle, interface_num);
            if (res < 0)
                CARDBOARD_LOGE("Can't release the interface.\n");
            interface_num = -1;
        }
        if (conf_desc != nullptr) {
            libusb_free_config_descriptor(conf_desc);
            conf_desc = nullptr;
        }
        if (handle != nullptr) {
            libusb_close(handle);
            handle = nullptr;
        }
        opened = false;
    }
}

void HidSensor::exit() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    if(usb_ctx != nullptr)
        libusb_exit(usb_ctx);
    usb_ctx = nullptr;
}

int HidSensor::startReading(unsigned char *buffer, int length, unsigned int timeout) {
    int res = -1, size;
    if (opened && (handle != nullptr)) {
        res = libusb_bulk_transfer(handle, 0x89, buffer, length, &size, timeout);
        if (res < 0) {
            CARDBOARD_LOGE("libusb_bulk_transfer failed %d\n", res);
        }
    }
    return res;
}

int HidSensor::startReading2(unsigned char *buffer, int length, unsigned int timeout) {
    int res = -1, size;
    if (opened && (handle != nullptr)) {
        res = libusb_bulk_transfer(handle, 0x89, buffer, length, &size, timeout);
        if (res < 0) {
            CARDBOARD_LOGE("libusb_bulk_transfer failed %d\n", res);
        }
    }
    return res;
}

uint64_t HidSensor::getTimeNano()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint64_t result = t.tv_sec * 1000000000LL + t.tv_nsec;
    return result;
}

}
