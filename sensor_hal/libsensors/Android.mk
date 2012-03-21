# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
ifeq ($(SENSORS_MODULE_VARIANT),)
  LOCAL_PATH := $(call my-dir)
  # HAL module implemenation, not prelinked
  include $(CLEAR_VARS)
  LOCAL_PRELINK_MODULE := false
  LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
  LOCAL_SHARED_LIBRARIES := liblog libcutils
  LOCAL_SRC_FILES := sensors.c  HMSHardIronCal.c HMSHeading.c
  LOCAL_MODULE := sensors.default
  include $(BUILD_SHARED_LIBRARY)

  include $(CLEAR_VARS)
  LOCAL_SRC_FILES:= HMSHardIronCal.c HMSHeading.c sensor-daemon.cpp
  LOCAL_MODULE:= sensor-daemon
  LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils
  LOCAL_SHARED_LIBRARIES += libdl libstlport
  include external/stlport/libstlport.mk
  include $(BUILD_EXECUTABLE)

endif
