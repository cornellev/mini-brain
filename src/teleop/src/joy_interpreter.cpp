#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ackermann_msgs/msg/ackermann_drive.hpp"
#include <cmath>
#include <vector>

class LogitechController {
    public:
        LogitechController(const std::vector<float>& axes, const std::vector<int>& buttons) : axes_(axes), buttons_(buttons) {}

        float get_left_trigger() const {
            return (axes_.size() > 2) ? (axes_[2] + 1.0f) / 2.0f : 0.0f;
        }

        float get_right_trigger() const {
            return (axes_.size() > 5) ? (axes_[5] + 1.0f) / 2.0f : 0.0f;
        }

    private:
        std::vector<float> axes_;
        std::vector<int> buttons_;
};

class JoyInterpreter : public rclcpp::Node {
    public:
        JoyInterpreter() : Node("joy_interpreter") {
            joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
                "joy", 10, std::bind(&JoyInterpreter::joy_callback, this, std::placeholders::_1));
            drive_pub_ = this->create_publisher<ackermann_msgs::msg::AckermannDrive>("ackermann_cmd", 10);
        }

    private:
        void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg) {
            LogitechController controller(msg->axes, msg->buttons);

            float left_trigger = controller.get_left_trigger();
            float right_trigger = controller.get_right_trigger();

            float speed = right_trigger - left_trigger;
            float steering_angle = (msg->axes.size() > 0) ? msg->axes[0] : 0.0f;

            ackermann_msgs::msg::AckermannDrive drive_msg;
            drive_msg.speed = speed * max_speed_;
            drive_msg.steering_angle = steering_angle * max_steering_angle_;

            drive_pub_->publish(drive_msg);
        }

        rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
        rclcpp::Publisher<ackermann_msgs::msg::AckermannDrive>::SharedPtr drive_pub_;
        const float max_speed_ = 5.0f; // meters per second
        const float max_steering_angle_ = M_PI / 4; // radians
};