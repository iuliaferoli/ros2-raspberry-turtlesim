# test_servo.py
import lgpio, time

h = lgpio.gpiochip_open(4)
SERVO_PIN = 18

lgpio.gpio_claim_output(h, SERVO_PIN)

# Sweep left → center → right → center
for pulse_us in [1000, 1500, 2000, 1500]:
    lgpio.tx_servo(h, SERVO_PIN, pulse_us)
    time.sleep(1)

lgpio.tx_servo(h, SERVO_PIN, 0)  # turn off
lgpio.gpiochip_close(h)