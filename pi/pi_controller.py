# pi_controller.py
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
import lgpio


class PiController(Node):
    def __init__(self):
        super().__init__('pi_controller')

        self.h = lgpio.gpiochip_open(4)

        self.SERVO_PIN = 18
        self.LED_LEFT = 23
        self.LED_RIGHT = 24

        lgpio.gpio_claim_output(self.h, self.SERVO_PIN)
        lgpio.gpio_claim_output(self.h, self.LED_LEFT)
        lgpio.gpio_claim_output(self.h, self.LED_RIGHT)

        self.subscription = self.create_subscription(
            Twist,
            '/turtle1/cmd_vel',
            self.cmd_vel_callback,
            10)

        self.get_logger().info('Pi controller ready — waiting for cmd_vel')

    def cmd_vel_callback(self, msg):
        angular = msg.angular.z

        pulse = int(1500 + angular * 250)
        pulse = max(1000, min(2000, pulse))
        lgpio.tx_servo(self.h, self.SERVO_PIN, pulse)

        lgpio.gpio_write(self.h, self.LED_LEFT, 1 if angular > 0.1 else 0)
        lgpio.gpio_write(self.h, self.LED_RIGHT, 1 if angular < -0.1 else 0)

        self.get_logger().info(
            f'linear={msg.linear.x:.2f} angular={angular:.2f} servo={pulse}')

    def destroy_node(self):
        lgpio.tx_servo(self.h, self.SERVO_PIN, 0)
        lgpio.gpio_write(self.h, self.LED_LEFT, 0)
        lgpio.gpio_write(self.h, self.LED_RIGHT, 0)
        lgpio.gpiochip_close(self.h)
        super().destroy_node()


def main():
    rclpy.init()
    node = PiController()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
