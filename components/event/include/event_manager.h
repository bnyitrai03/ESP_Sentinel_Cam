#pragma once

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Macro definitions for simplifying event handling
 */
#define SUBSCRIBE(type, code)                                                  \
  EventManager::getInstance().subscribe(type, [&](EventType) { code; })
#define PUBLISH(type) EventManager::getInstance().publish(type)

/**
 * @brief Enum of event types in the system
 */
enum class EventType {
  BUTTON_PRESSED,
  STOP_BUTTON,
  RESET,
  SLEEP_UNTIL_BUTTON_PRESS,
  SLEEP_UNTIL_NEXT_PERIOD,
  SLEEP_UNTIL_NEXT_TIMING,
};

/**
 * @brief Callback function type for event handlers
 */
using EventCallback = std::function<void(EventType)>;

/**
 * @brief Singleton class for managing events in the system
 */
class EventManager {
public:
  /**
   * @brief Get the singleton instance of EventManager
   * @return Reference to the EventManager instance
   */
  static EventManager &getInstance();

  /**
   * @brief Subscribe to an event type
   * @param type The event type to subscribe to
   * @param callback The callback function to be called when the event occurs
   * @return Subscription ID that can be used to unsubscribe
   */
  int subscribe(EventType type, EventCallback callback);

  /**
   * @brief Helper method to publish an event by type
   * @param type The event type to publish
   */
  void publish(EventType type);

  /**
   * @brief Wait for and process the next event from the queue
   * @return true if an event was processed, false if timeout occurred
   */
  bool process_event_queue();

private:
  EventManager();
  ~EventManager();

  // Prevent copying
  EventManager(const EventManager &) = delete;
  EventManager &operator=(const EventManager &) = delete;

  struct Subscription {
    int id;                 /*!< ID for the subscription */
    EventCallback callback; /*!< Function that gets registered */
  };

  std::map<EventType, std::vector<Subscription>> _subscribers;
  SemaphoreHandle_t _mutex;
  int _nextSubscriptionId;
  QueueHandle_t _eventQueue;
  static constexpr size_t EVENT_QUEUE_SIZE = 10;
};
