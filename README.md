# ROS2 Turtlesim + Raspberry Pi GPIO Control

Control real hardware (servo + LEDs) from ROS2 turtlesim via teleop. Part of the **Road to Physical AI** series by [Back to Engineering](https://youtube.com/@backtoengineering).

---

## What this does

- Runs turtlesim on your laptop
- Subscribes to `/turtle1/cmd_vel` on a Raspberry Pi
- Left/right arrow keys → servo sweeps + LEDs light up
- Shows distributed ROS2 across two machines with zero extra config

---

## Hardware

- Raspberry Pi 5 (Ubuntu Server 24.04)
- 1× servo (SG90 or MG90S)
- 2× LEDs
- 2× 330Ω resistors
- Breadboard + jumper wires

### Wiring

| Component | Pin |
|---|---|
| Servo signal (orange/yellow) | GPIO 18 (physical pin 12) |
| Servo VCC (red) | 5V (pin 2) |
| Servo GND (brown/black) | GND (pin 6) |
| LED left anode → 330Ω resistor | GPIO 23 (physical pin 16) |
| LED right anode → 330Ω resistor | GPIO 24 (physical pin 18) |
| Both LED cathodes (short leg) | GND |

> Always power off the Pi before wiring. Run `pinout` on the Pi to confirm pin numbers.

---

## Laptop setup (Mint / Ubuntu 24.04)

### Install Pixi

```bash
curl -fsSL https://pixi.sh/install.sh | bash
source ~/.bashrc
```

### Clone and install

```bash
git clone https://github.com/iuliaferoli/ros2-raspberry-turtlesim.git
cd ros2-raspberry-turtlesim
pixi install
```

The `pixi.toml` already includes `ros-jazzy-desktop`, `ros-jazzy-turtlesim`, and sets `ROS_DOMAIN_ID=42` automatically.

### Build the hexagon node (optional)

```bash
pixi run build
```

This runs `colcon build --symlink-install` to compile `hexagon_turtles`.

---

## Raspberry Pi setup (Ubuntu Server 24.04)

### Flash Ubuntu Server 24.04 (64-bit) using Raspberry Pi Imager
Enable SSH and WiFi in the imager settings before flashing.

### SSH into the Pi

```bash
ssh ubuntu@<pi-ip-address>
```

Find the Pi's IP with `hostname -I` on the Pi.

### Add ROS2 repository

```bash
sudo apt install software-properties-common curl -y
sudo add-apt-repository universe
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
  http://packages.ros.org/ros2/ubuntu noble main" \
  | sudo tee /etc/apt/sources.list.d/ros2.list
sudo apt update
```

### Fix held packages (Pi 5 / Ubuntu 24.04 issue)

```bash
sudo apt full-upgrade -y
sudo reboot
```

### Install ROS2

```bash
sudo apt install ros-jazzy-ros-base -y
source /opt/ros/jazzy/setup.bash
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
```

### Install lgpio

```bash
sudo apt install python3-lgpio liblgpio-dev swig -y
```

### Fix GPIO permissions

```bash
sudo chmod a+rw /dev/gpiochip4
```

To make permanent across reboots:
```bash
sudo groupadd -f gpio
sudo usermod -a -G gpio $USER
echo 'SUBSYSTEM=="gpio", GROUP="gpio", MODE="0660"' | sudo tee /etc/udev/rules.d/99-gpio.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

> **Pi 5 note:** The GPIO header is on `gpiochip4` (pinctrl-rp1), not `gpiochip0`. Run `gpiodetect` to confirm.

### Set ROS domain ID

```bash
export ROS_DOMAIN_ID=42
echo "export ROS_DOMAIN_ID=42" >> ~/.bashrc
```

### Copy the Pi script

From your laptop:
```bash
scp pi/pi_controller.py ubuntu@<pi-ip-address>:~/
```

Optionally copy the test scripts too:
```bash
scp pi/test_servo.py pi/test_leds.py ubuntu@<pi-ip-address>:~/
```

---

## Running

### Terminal 1 — laptop (turtlesim window)

```bash
cd ros2-raspberry-turtlesim
pixi run sim
```

### Terminal 2 — laptop (keyboard teleop)

```bash
cd ros2-raspberry-turtlesim
pixi run teleop
```

Use arrow keys to drive the turtle. The terminal running teleop must stay focused.

### Terminal 3 — Pi (GPIO controller)

```bash
source /opt/ros/jazzy/setup.bash
export ROS_DOMAIN_ID=42
python3 ~/pi_controller.py
```

Press left/right arrows — servo sweeps, LEDs light up in sync with the turtle turning.

### Hexagon demo (optional, laptop only)

```bash
pixi run hexagon
```

Spawns 6 turtles in a hexagon and moves them sequentially along the edges.

---

## Testing hardware on the Pi

Before running the full ROS2 node, verify your wiring:

```bash
python3 ~/test_servo.py    # sweeps servo left → center → right → center
python3 ~/test_leds.py     # blinks LEDs alternately 3 times
```

---

## Multiple terminals on the Pi (tmux)

```bash
sudo apt install tmux -y
tmux
```

| Shortcut | Action |
|---|---|
| `Ctrl+B %` | Split vertically |
| `Ctrl+B "` | Split horizontally |
| `Ctrl+B →` | Switch pane right |
| `Ctrl+B ←` | Switch pane left |

---

## Files

| File | Description |
|---|---|
| `src/hexagon_turtles.cpp` | C++ node — spawns 6 turtles in a hexagon pattern |
| `pi/pi_controller.py` | Python ROS2 node — subscribes to cmd_vel, drives GPIO |
| `pi/test_servo.py` | Standalone servo test (no ROS2 needed) |
| `pi/test_leds.py` | Standalone LED test (no ROS2 needed) |
| `pixi.toml` | Pixi workspace config (ROS2 Jazzy via robostack-staging) |
| `package.xml` | ROS2 package manifest |
| `CMakeLists.txt` | CMake build config for the C++ node |

---

## Troubleshooting

**`could not connect to display`** — turtlesim needs a screen. Run it on your laptop, not the Pi.

**`No module named rclpy`** — ROS isn't sourced. Run `source /opt/ros/jazzy/setup.bash`.

**`can not open gpiochip`** — permissions issue. Run `sudo chmod a+rw /dev/gpiochip4`.

**LEDs/servo don't respond** — check `ros2 topic list` on the Pi shows `/turtle1/cmd_vel`. If not, both machines need the same `ROS_DOMAIN_ID` on the same network.

**Held packages error on apt install** — run `sudo apt full-upgrade -y && sudo reboot` before installing ROS.
