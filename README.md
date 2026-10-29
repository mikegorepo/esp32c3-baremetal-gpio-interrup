# ESP32-C3 Bare-Metal GPIO Interrupt Pipeline

An experimental project exploring low-level register manipulation, clock gating, and interrupt routing on the **ESP32-C3 (RISC-V)** architecture using ESP-IDF.

This repository demonstrates how to bypass high-level abstractions to manually configure GPIO pins, trigger settings, and the Interrupt Matrix controller down to the register level.

## 🚀 Key Technical Highlights
- **Direct Peripheral Access**: Manual configuration of GPIO base registers (`0x60004000`) and Interrupt Matrix base registers (`0x600C2000`).
- **Clock Gating**: Explicitly enabling the Interrupt Matrix clock via `INTERRUPT_CORE0_CLOCK_GATE` (`0x0100`).
- **The Critical Bit 13**: Unlocking the CPU interrupt propagation path directly inside `GPIO_PINX_REG`.
- **Hybrid Integration**: Marrying pure bare-metal hardware configuration with ESP-IDF's `esp_intr_alloc` vector dispatcher for safe and reliable ISR execution.

## 🛠️ Register Map Explored
| Register / Offset | Description |
| :--- | :--- |
| `GPIO_BASE + 0x0098` (`GPIO_PIN9_REG`) | Configures pull-up, trigger mode, input enable, and **Bit 13** (CPU Interrupt Enable). |
| `GPIO_BASE + 0x0044` (`GPIO_STATUS_REG`) | Latches raw pin state transitions. |
| `GPIO_BASE + 0x005C` (`PCPU_INT`) | Reflects core-level peripheral interrupt forwarding. |
| `INTERRUPT_MATRIX_BASE + 0x0100` | Clock gate control for the Interrupt Matrix controller. |

## 💻 Hardware Requirements
- ESP32-C3 Development Board (e.g., ESP32-C3-DevKitM-1 or SuperMini).
- Uses **GPIO 9** by default (commonly connected to the onboard **BOOT** button).

## 📋 How to Build & Flash
1. Ensure you have the **ESP-IDF v5.x** environment set up.
2. Clone or place `main.c` into your project's `main/` directory.
3. Build and flash the project using standard IDF tools:
   ```bash
   idf.py build flash monitor
