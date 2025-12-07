#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "ackermann_msgs/msg/ackermann_drive.hpp"
#include "std_msgs/msg/string.hpp"

/*

class MinimalSubscriber : public rclcpp::Node
{
  public:
    MinimalSubscriber()
    : Node("minimal_subscriber")
    {
      subscription_ = this->create_subscription<std_msgs::msg::String>(
      "topic", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
    }

  private:
    void topic_callback(const std_msgs::msg::String & msg) const
    {
      RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg.data.c_str());
    }
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

*/

/*
public:
  MinimalPublisher()
  : Node("minimal_publisher"), count_(0)
  {
    publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
    timer_ = this->create_wall_timer(
    500ms, std::bind(&MinimalPublisher::timer_callback, this));
  }

private:
  void timer_callback()
  {
    auto message = std_msgs::msg::String();
    message.data = "Hello, world! " + std::to_string(count_++);
    RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
    publisher_->publish(message);
  }

*/

class JoyInterpreter : public rclcpp::Node {
    public:
        JoyInterpreter() : Node("JoyInterpreter") {
            publisher_ = this->create_publisher<ackermann_msgs::msg::AckermannDrive>("ackermann", 10);
            subscription_ = this->create_subscription<sensor_msgs::msg::Joy>("joy", 10, std::bind(&JoyInterpreter::topic_callback, this, std::placeholders::_1));
        }

    private:
    
    rclcpp::Publisher<ackermann_msgs::msg::AckermannDrive>::SharedPtr publisher_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;

        void topic_callback(const sensor_msgs::msg::Joy::SharedPtr msg) const
        {
            //RCLCPP_INFO(this->get_logger(), "Recieved!");
            
            double left_stick_vertical = msg->axes[1];
            double right_stick_horizontal = msg->axes[3];
            
            auto drive_msg = ackermann_msgs::msg::AckermannDrive();
            drive_msg.speed = left_stick_vertical * 2.0; // scaled between -2.0 and 2.0 m/s
            drive_msg.steering_angle = right_stick_horizontal * 0.5; // scaled between -0.5 and 0.5 radians

            publisher_->publish(drive_msg);
        }
};


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyInterpreter>());
    rclcpp::shutdown();
}