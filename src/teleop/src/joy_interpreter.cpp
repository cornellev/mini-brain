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
            publishertwo_ = this->create_publisher<std_msgs::msg::String>("test", 10);
            subscription_ = this->create_subscription<sensor_msgs::msg::Joy>("joy", 10, std::bind(&JoyInterpreter::topic_callback, this, std::placeholders::_1));
        }

    private:
    
    rclcpp::Publisher<ackermann_msgs::msg::AckermannDrive>::SharedPtr publisher_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publishertwo_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;

        void topic_callback(const sensor_msgs::msg::Joy::SharedPtr msg) const
        {
            RCLCPP_INFO(this->get_logger(), "Recieved!");
            auto message = std_msgs::msg::String();
            message.data = "Testing";
            publishertwo_->publish(message);
            //publisher_->publish();
        }
};


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyInterpreter>());
    rclcpp::shutdown();
}