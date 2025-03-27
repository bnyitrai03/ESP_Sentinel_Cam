#include "storage.h"
#include "unity.h"

TEST_CASE("Storage write and read", "[storage]") {
  Storage storage;
  char value[32];

  TEST_ASSERT(storage.write("test_str", "test") == ESP_OK);
  TEST_ASSERT(storage.read("test_str", value, sizeof(value)) == ESP_OK);
  TEST_ASSERT_EQUAL_STRING("test", value);
}