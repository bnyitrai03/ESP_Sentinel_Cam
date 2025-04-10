#include "event_manager.h"
#include "unity.h"

static bool callback_invoked = false;
static void test_callback(EventType type) { callback_invoked = true; }

TEST_CASE("Event manager subscribe and publish", "[event]") {
  EventManager &eventManager = EventManager::getInstance();

  callback_invoked = false;
  int subscriptionId =
      eventManager.subscribe(EventType::BUTTON_PRESSED, test_callback);
  eventManager.publish(EventType::BUTTON_PRESSED);
  bool processed = eventManager.process_event_queue();

  TEST_ASSERT_GREATER_THAN(0, subscriptionId);
  TEST_ASSERT_TRUE(processed);
  TEST_ASSERT_TRUE(callback_invoked);
}