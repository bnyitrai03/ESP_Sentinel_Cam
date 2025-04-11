#include "unity.h"

static void print_banner(const char* text)
{
    printf("\n#### %s #####\n\n", text);
}

void app_main(void)
{
    print_banner("Unity test runner");
    printf("Total test count: %d\n\n", unity_get_test_count());

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

}