/**
 * @file main.c
 * @brief Bare-metal GPIO Interrupt Configuration for ESP32-C3
 * @details Demonstrates direct peripheral register manipulation for GPIO 9 interrupts,
 *          bypassing high-level abstractions for clock gates, pin configuration (including 
 *          the critical CPU Interrupt Enable bit 13), and routing via Interrupt Matrix, 
 *          coupled with standard ESP-IDF ISR vector allocation.
 */

#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp_rom_sys.h"

// Peripheral Base Addresses
#define GPIO_BASE                   0x60004000
#define INTERRUPT_MATRIX_BASE       0x600C2000

// GPIO Registers
#define GPIO_ENABLE_REG             (*(volatile uint32_t *)(GPIO_BASE + 0x0020))
#define GPIO_IN_REG                 (*(volatile uint32_t *)(GPIO_BASE + 0x003C))
#define GPIO_PIN9_REG               (*(volatile uint32_t *)(GPIO_BASE + 0x0098)) 
#define GPIO_STATUS_REG             (*(volatile uint32_t *)(GPIO_BASE + 0x0044))
#define GPIO_STATUS_W1TC_REG        (*(volatile uint32_t *)(GPIO_BASE + 0x004C))

// Interrupt Matrix Registers
#define INTERRUPT_CORE0_CLOCK_GATE  (*(volatile uint32_t *)(INTERRUPT_MATRIX_BASE + 0x0100))

// Handle for interrupt allocation
static intr_handle_t gpio_handle = NULL;

// ISR Callback Function placed in IRAM for fast execution
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    if (GPIO_STATUS_REG & (1 << 9)) {
        esp_rom_printf("interupt function handled\n");
        
        // Clear the GPIO interrupt status (Write 1 to Clear)
        GPIO_STATUS_W1TC_REG = (1 << 9);
    }
}

void app_main(void) {
    printf("=== ESP32-C3 BARE-METAL GPIO INTERRUPT PIPELINE ===\n");

    // 1. Enable Interrupt Matrix Clock Gate
    INTERRUPT_CORE0_CLOCK_GATE = 0x1;

    // 2. Configure GPIO 9 as Input (clear enable bit)
    GPIO_ENABLE_REG &= ~(1 << 9);

    // 3. Configure GPIO Pin 9 Register:
    //    - Bit 3: Pull-up enabled
    //    - Bits [5:4]: Falling edge trigger (2 << 4)
    //    - Bit 7: Input enable
    //    - Bit 13: CPU Interrupt Enable per-pin (Crucial for propagating to PCPU_INT)
    GPIO_PIN9_REG = (1 << 3) | (2 << 4) | (1 << 7) | (1 << 13); 

    // 4. Allocate and map the interrupt using ESP-IDF helper 
    //    (Source 16 corresponds to GPIO interrupt source on ESP32-C3)
    esp_err_t err = esp_intr_alloc(16, ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM, gpio_isr_handler, NULL, &gpio_handle);
    if (err != ESP_OK) {
        printf("Failed to allocate interrupt: %d\n", err);
    }

    printf("Setup complete! Waiting for GPIO 9 (BOOT button) press...\n");

    while (1) {
        printf("main loop running\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
