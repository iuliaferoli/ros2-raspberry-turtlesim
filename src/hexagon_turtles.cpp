// hexagon_turtles.cpp
// Spawns 6 turtles at hexagon vertices and moves them one by one with a delay.
// Part of the Road to Physical AI series — Back to Engineering
// https://youtube.com/@backtoengineering

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <turtlesim/srv/spawn.hpp>
#include <cmath>

using namespace std::chrono_literals;

class HexagonTurtles : public rclcpp::Node
{
public:
  HexagonTurtles() : Node("hexagon_turtles"), current_turtle_(0), phase_(0)
  {
    spawn_client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");

    while (!spawn_client_->wait_for_service(1s)) {
      RCLCPP_INFO(this->get_logger(), "Waiting for spawn service...");
    }

    spawnAllTurtles();

    // Timer drives the movement state machine at 10Hz
    phase_start_ = this->now();
    timer_ = this->create_wall_timer(
      100ms, std::bind(&HexagonTurtles::movementLoop, this));
  }

private:
  rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_client_;
  std::vector<rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr> cmd_publishers_;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Time phase_start_;
  int current_turtle_;
  int phase_;  // 0 = moving, 1 = delay between turtles

  static constexpr int    NUM_TURTLES    = 6;
  static constexpr double CENTER_X       = 5.5;
  static constexpr double CENTER_Y       = 5.5;
  static constexpr double RADIUS         = 3.0;   // hexagon radius = edge length
  static constexpr double MOVE_DURATION  = 3.0;   // seconds per edge
  static constexpr double DELAY_BETWEEN  = 0.8;   // seconds between turtles
  static constexpr double LINEAR_SPEED   = RADIUS / MOVE_DURATION;

  void spawnAllTurtles()
  {
    for (int i = 0; i < NUM_TURTLES; i++) {
      double alpha = i * M_PI / 3.0;                 // vertex angle (60° steps)
      double x     = CENTER_X + RADIUS * std::cos(alpha);
      double y     = CENTER_Y + RADIUS * std::sin(alpha);
      double theta = alpha + (2.0 * M_PI / 3.0);    // face toward next vertex

      auto req   = std::make_shared<turtlesim::srv::Spawn::Request>();
      req->x     = x;
      req->y     = y;
      req->theta = theta;
      req->name  = "turtle" + std::to_string(i + 2); // turtle1 already exists

      auto future = spawn_client_->async_send_request(req);
      rclcpp::spin_until_future_complete(this->get_node_base_interface(), future);

      RCLCPP_INFO(this->get_logger(), "Spawned turtle%d at (%.2f, %.2f) theta=%.2f",
                  i + 2, x, y, theta);

      auto pub = this->create_publisher<geometry_msgs::msg::Twist>(
        "/turtle" + std::to_string(i + 2) + "/cmd_vel", 10);
      cmd_publishers_.push_back(pub);
    }
  }

  void movementLoop()
  {
    if (current_turtle_ >= NUM_TURTLES) {
      timer_->cancel();
      RCLCPP_INFO(this->get_logger(), "Hexagon complete!");
      return;
    }

    double elapsed = (this->now() - phase_start_).seconds();
    geometry_msgs::msg::Twist twist;

    if (phase_ == 0) {
      // Moving — publish velocity until edge is traversed
      twist.linear.x = LINEAR_SPEED;
      cmd_publishers_[current_turtle_]->publish(twist);

      if (elapsed >= MOVE_DURATION) {
        twist.linear.x = 0.0;                          // stop
        cmd_publishers_[current_turtle_]->publish(twist);
        phase_ = 1;
        phase_start_ = this->now();
        RCLCPP_INFO(this->get_logger(), "Turtle%d done, waiting...", current_turtle_ + 2);
      }
    } else {
      // Delay before next turtle starts
      if (elapsed >= DELAY_BETWEEN) {
        current_turtle_++;
        phase_ = 0;
        phase_start_ = this->now();
        if (current_turtle_ < NUM_TURTLES) {
          RCLCPP_INFO(this->get_logger(), "Starting turtle%d", current_turtle_ + 2);
        }
      }
    }
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HexagonTurtles>());
  rclcpp::shutdown();
  return 0;
}
