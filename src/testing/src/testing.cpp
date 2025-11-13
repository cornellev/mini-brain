#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <chrono>
using namespace std::chrono_literals;

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("testing_node");
    RCLCPP_INFO(node->get_logger(), "Testing node has started.");
    
    auto pub = node->create_publisher<std_msgs::msg::String>("test_topic", 10);
    
    auto timer = node->create_wall_timer(1s, [pub]() {
        std_msgs::msg::String msg;
        msg.data = "Hello, world!";
        pub->publish(msg);
    });
    
    RCLCPP_INFO(node->get_logger(), "Publishing messages to test_topic.");
    rclcpp::spin(node);
    rclcpp::shutdown();
}