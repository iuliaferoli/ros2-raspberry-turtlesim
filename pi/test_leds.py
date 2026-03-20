# test_leds.py
import lgpio, time

h = lgpio.gpiochip_open(4)

LED_LEFT  = 23
LED_RIGHT = 24

lgpio.gpio_claim_output(h, LED_LEFT)
lgpio.gpio_claim_output(h, LED_RIGHT)

for _ in range(3):
    lgpio.gpio_write(h, LED_LEFT, 1)
    time.sleep(0.3)
    lgpio.gpio_write(h, LED_LEFT, 0)
    lgpio.gpio_write(h, LED_RIGHT, 1)
    time.sleep(0.3)
    lgpio.gpio_write(h, LED_RIGHT, 0)

lgpio.gpiochip_close(h)