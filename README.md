# STM32-Snake-Game
A handheld arcade game developed on the STM32F103C8T6 (Blue Pill). This project demonstrates low-level hardware control, real-time logic, and optimized display interfacing.

## Tech Stack
MCU: STM32F103 (ARM Cortex-M3, 72MHz).

Display: 1.8" TFT LCD (ST7735 Driver).

Protocols: SPI (Display), ADC (Joystick), PWM (Audio), EXTI (Interrupts).

## Key Features
Real-time UI: Dedicated screens for Start Menu, Scoreboard, and Game Over.

True Random Generation: Uses ADC noise as a seed for `srand()` to ensure unpredictable food placement.

Smart Collision Logic: Implements a rigorous check to prevent food from spawning on the snake's head or body segments.

Control: Using a Joystick to control the snacke and four buttons for others functions (Select, Pause, Continue, Play Again)

Dynamic Audio: PWM-controlled buzzer for real-time sound feedback during gameplay.

## How to Play
The game is controlled via a hardware interface consisting of a Joystick and push buttons.
### Controls
Joystick (Analog): Move the Joystick up, down, left, or right to change the snake's direction. The system uses ADC Polling to translate analog voltages into directional commands.

RED Button (X): Slow select (Menu), Pause (Score board)

YELLOW Button (Y): Fast select (Menu), Continue (Score board)

GREEN Button (B): Enter

BLUE Button (A): Reset (Play Again)

### Game Rules
Objective: Eat the red food blocks to grow longer and increase your score.

Scoring: Each food item consumed adds 1 points. The current score is displayed on the top status bar.

Lose Conditions: The snake's head collides with its own body (Self-collision).

Audio Feedback: A "beep" sound will trigger via the PWM buzzer whenever the snake eats or crashes.
