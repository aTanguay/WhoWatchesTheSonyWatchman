/**
 * Display Test Patterns Implementation
 * Hardware verification tests for ST7789 display
 */

#include "test_patterns.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "TEST";

/**
 * Test 1: Solid Colors
 * Cycles through primary colors to test all pixels
 */
void test_solid_colors(void)
{
    ESP_LOGI(TAG, "Test 1: Solid Colors");

    const uint16_t colors[] = {
        COLOR_RED,
        COLOR_GREEN,
        COLOR_BLUE,
        COLOR_YELLOW,
        COLOR_CYAN,
        COLOR_MAGENTA,
        COLOR_WHITE,
        COLOR_BLACK
    };

    const char *color_names[] = {
        "RED", "GREEN", "BLUE", "YELLOW",
        "CYAN", "MAGENTA", "WHITE", "BLACK"
    };

    for (int i = 0; i < 8; i++) {
        ESP_LOGI(TAG, "  %s", color_names[i]);
        display_clear(colors[i]);
        vTaskDelay(pdMS_TO_TICKS(2000));  // 2 seconds per color
    }
}

/**
 * Test 2: Color Bars
 * Classic TV test pattern
 */
void test_color_bars(void)
{
    ESP_LOGI(TAG, "Test 2: Color Bars");

    const uint16_t bars[] = {
        COLOR_WHITE,
        COLOR_YELLOW,
        COLOR_CYAN,
        COLOR_GREEN,
        COLOR_MAGENTA,
        COLOR_RED,
        COLOR_BLUE,
        COLOR_BLACK
    };

    int bar_width = DISPLAY_WIDTH / 8;

    for (int i = 0; i < 8; i++) {
        display_fill_rect(i * bar_width, 0, bar_width, DISPLAY_HEIGHT, bars[i]);
    }

    vTaskDelay(pdMS_TO_TICKS(5000));  // Show for 5 seconds
}

/**
 * Test 3: Bouncing Box
 * Animated square to test display refresh
 */
void test_bouncing_box(void)
{
    ESP_LOGI(TAG, "Test 3: Bouncing Box");

    const int box_size = 40;
    int x = 0, y = 0;
    int dx = 4, dy = 3;  // Velocity

    // Run for 10 seconds
    uint32_t start_time = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(10000)) {
        // Clear screen
        display_clear(COLOR_BLACK);

        // Draw box
        display_fill_rect(x, y, box_size, box_size, COLOR_CYAN);

        // Update position
        x += dx;
        y += dy;

        // Bounce off edges
        if (x <= 0 || x >= DISPLAY_WIDTH - box_size) {
            dx = -dx;
            x = (x <= 0) ? 0 : DISPLAY_WIDTH - box_size;
        }
        if (y <= 0 || y >= DISPLAY_HEIGHT - box_size) {
            dy = -dy;
            y = (y <= 0) ? 0 : DISPLAY_HEIGHT - box_size;
        }

        vTaskDelay(pdMS_TO_TICKS(33));  // ~30 FPS
    }
}

/**
 * Test 4: Gradients
 * RGB color gradients to test color depth
 */
void test_gradients(void)
{
    ESP_LOGI(TAG, "Test 4: RGB Gradients");

    // Red gradient
    ESP_LOGI(TAG, "  Red Gradient");
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        uint8_t intensity = (x * 255) / DISPLAY_WIDTH;
        uint16_t color = RGB565(intensity, 0, 0);
        display_fill_rect(x, 0, 1, DISPLAY_HEIGHT / 3, color);
    }

    // Green gradient
    ESP_LOGI(TAG, "  Green Gradient");
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        uint8_t intensity = (x * 255) / DISPLAY_WIDTH;
        uint16_t color = RGB565(0, intensity, 0);
        display_fill_rect(x, DISPLAY_HEIGHT / 3, 1, DISPLAY_HEIGHT / 3, color);
    }

    // Blue gradient
    ESP_LOGI(TAG, "  Blue Gradient");
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
        uint8_t intensity = (x * 255) / DISPLAY_WIDTH;
        uint16_t color = RGB565(0, 0, intensity);
        display_fill_rect(x, 2 * DISPLAY_HEIGHT / 3, 1, DISPLAY_HEIGHT / 3, color);
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
}

/**
 * Test 5: Checkerboard
 * Tests pixel accuracy and alignment
 */
void test_checkerboard(void)
{
    ESP_LOGI(TAG, "Test 5: Checkerboard");

    const int square_size = 20;

    for (int y = 0; y < DISPLAY_HEIGHT; y += square_size) {
        for (int x = 0; x < DISPLAY_WIDTH; x += square_size) {
            // Alternate between black and white
            uint16_t color = ((x / square_size) + (y / square_size)) % 2 ? COLOR_WHITE : COLOR_BLACK;

            int w = (x + square_size > DISPLAY_WIDTH) ? DISPLAY_WIDTH - x : square_size;
            int h = (y + square_size > DISPLAY_HEIGHT) ? DISPLAY_HEIGHT - y : square_size;

            display_fill_rect(x, y, w, h, color);
        }
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
}

/**
 * Test 6: Moving Lines
 * Tests refresh rate and motion
 */
void test_moving_lines(void)
{
    ESP_LOGI(TAG, "Test 6: Moving Lines");

    // Run for 10 seconds
    uint32_t start_time = xTaskGetTickCount();
    int offset = 0;

    while ((xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(10000)) {
        display_clear(COLOR_BLACK);

        // Draw vertical lines
        for (int x = offset; x < DISPLAY_WIDTH; x += 10) {
            display_fill_rect(x, 0, 2, DISPLAY_HEIGHT, COLOR_GREEN);
        }

        // Draw horizontal lines
        for (int y = offset; y < DISPLAY_HEIGHT; y += 10) {
            display_fill_rect(0, y, DISPLAY_WIDTH, 2, COLOR_BLUE);
        }

        offset = (offset + 1) % 10;
        vTaskDelay(pdMS_TO_TICKS(50));  // 20 FPS
    }
}

/**
 * Run all tests in sequence
 */
void run_display_tests(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Display Hardware Test Suite");
    ESP_LOGI(TAG, "  Testing ST7789 %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
    ESP_LOGI(TAG, "========================================");

    while (1) {
        test_solid_colors();
        test_color_bars();
        test_bouncing_box();
        test_gradients();
        test_checkerboard();
        test_moving_lines();

        ESP_LOGI(TAG, "Test cycle complete. Restarting...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
