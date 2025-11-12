/**
 * Display Test Patterns
 * Simple test patterns for hardware verification
 * No SD card or other components required!
 */

#ifndef TEST_PATTERNS_H
#define TEST_PATTERNS_H

#include <stdint.h>
#include "display.h"

/**
 * Run all display tests in sequence
 * Tests: solid colors, color bars, bouncing box, gradients
 * Loops forever - perfect for initial hardware testing
 */
void run_display_tests(void);

/**
 * Individual test functions
 */
void test_solid_colors(void);      // Cycle through R, G, B, W, Black
void test_color_bars(void);        // Classic TV test pattern
void test_bouncing_box(void);      // Animated bouncing square
void test_gradients(void);         // RGB gradients
void test_checkerboard(void);      // Alternating pattern
void test_moving_lines(void);      // Scrolling lines

#endif // TEST_PATTERNS_H
