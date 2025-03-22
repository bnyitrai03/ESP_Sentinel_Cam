#pragma once

#include <cstdint>

typedef void (*DeinitCallback)();

/**
 * @brief Sets the callback function for WiFi deinitialization
 *
 * @param callback Function to be called for WiFi deinitialization
 */
void set_wifi_deinit_callback(DeinitCallback callback);

/**
 * @brief Sets the callback function for MQTT deinitialization
 *
 * @param callback Function to be called for MQTT deinitialization
 */
void set_mqtt_deinit_callback(DeinitCallback callback);

/**
 * @brief Sets the callback function for camera deinitialization
 *
 * @param callback Function to be called for camera deinitialization
 */
void set_camera_deinit_callback(DeinitCallback callback);

/**
 * @brief Handles an error by restarting the device
 *
 * This function blinks the error LED pattern, then deinitializes the camera,
 * MQTT, and Wi-Fi drivers. If the error_count is 15, the device will sleep for
 * 10 minutes before restarting. Otherwise, the device will restart immediately.
 *
 */
void restart();

/**
 * @brief Resets the device
 *
 * This function erases the NVS and restarts the device.
 */
void reset_device();

/**
 * @brief Deinitializes all components
 *
 * This function deinitializes the camera, MQTT, and Wi-Fi drivers.
 */
void deinit_components();

/**
 * @brief This function increases and returns the error count stored in the NVS.
 *
 * @return The error count
 */
uint32_t get_error_count();