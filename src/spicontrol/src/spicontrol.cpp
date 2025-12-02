#include "rclcpp/rclcpp.hpp"
#include "ackermann_msgs/msg/ackermann_drive.hpp"

class SPINode : public rclcpp::Node {
    public:
        SPINode() : Node("spi_node") {
            subscription_ = this->create_subscription<ackermann_msgs::msg::AckermannDrive>("ackermann", 10, std::bind(&SPINode::handle_ackermann_update, this, std::placeholders::_1));
        }

    private:

    void handle_ackermann_update(const ackermann_msgs::msg::AckermannDrive::SharedPtr msg) const {
        RCLCPP_INFO(this->get_logger(), "Received Ackermann Drive - Speed: '%f', Steering Angle: '%f'", msg->speed, msg->steering_angle);
        double speed = msg->speed;
        double steering_angle = msg->steering_angle;
        
    }

    rclcpp::Subscription<ackermann_msgs::msg::AckermannDrive>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SPINode>());
    rclcpp::shutdown();
}