#include "event_manager.h"
#include "error_handler.h"
#include "esp_log.h"

auto constexpr *TAG = "EventManager";

EventManager::EventManager() : _nextSubscriptionId(1) {
  _mutex = xSemaphoreCreateMutex();
  if (_mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create mutex");
    restart();
  }
  _eventQueue = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(EventType));
  if (_eventQueue == NULL) {
    ESP_LOGE(TAG, "Failed to create event queue");
    restart();
  }
}

EventManager::~EventManager() {
  if (_mutex != NULL) {
    vSemaphoreDelete(_mutex);
    _mutex = NULL;
  }
  if (_eventQueue != NULL) {
    vQueueDelete(_eventQueue);
    _eventQueue = NULL;
  }
}

EventManager &EventManager::getInstance() {
  static EventManager instance;
  return instance;
}

int EventManager::subscribe(EventType type, EventCallback callback) {
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) {
    ESP_LOGE(TAG, "Failed to take mutex in subscribe");
    return -1;
  }

  int id = _nextSubscriptionId++;
  _subscribers[type].push_back({id, callback});

  xSemaphoreGive(_mutex);
  ESP_LOGD(TAG, "Subscribed to event type %d with ID %d",
           static_cast<int>(type), id);
  return id;
}

bool EventManager::unsubscribe(EventType type, int subscriptionId) {
  if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) {
    ESP_LOGE(TAG, "Failed to take mutex in unsubscribe");
    return false;
  }

  bool success = false;
  auto it = _subscribers.find(type);
  if (it != _subscribers.end()) {
    auto &subs = it->second;
    for (auto subIt = subs.begin(); subIt != subs.end(); ++subIt) {
      if (subIt->id == subscriptionId) {
        subs.erase(subIt);
        success = true;
        ESP_LOGD(TAG, "Unsubscribed from event type %d with ID %d",
                 static_cast<int>(type), subscriptionId);
        break;
      }
    }
  }

  xSemaphoreGive(_mutex);
  return success;
}

void EventManager::publish(EventType type) {
  if (_eventQueue == NULL) {
    ESP_LOGE(TAG, "Event queue not initialized");
    return;
  }

  BaseType_t result = xQueueSend(_eventQueue, &type, pdMS_TO_TICKS(100));
  if (result != pdTRUE) {
    ESP_LOGW(TAG, "Failed to queue event %d", static_cast<int>(type));
  } else {
    ESP_LOGI(TAG, "Event %d queued successfully", static_cast<int>(type));
  }
}

bool EventManager::process_event_queue() {
  if (_eventQueue == NULL) {
    ESP_LOGE(TAG, "Event queue not initialized");
    return false;
  }

  EventType event;
  BaseType_t result = xQueueReceive(_eventQueue, &event, pdMS_TO_TICKS(1000));
  if (result != pdTRUE) {
    return false;
  }

  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    ESP_LOGE(TAG, "Failed to take mutex in process_event_queue");
    return false;
  }

  auto it = _subscribers.find(event);
  if (it != _subscribers.end()) {
    for (auto &sub : it->second) {
      sub.callback(event);
    }
  }

  xSemaphoreGive(_mutex);
  return true;
}