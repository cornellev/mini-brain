#include <pigpiod_if2.h>   // daemon version of the API (important!)
#include "rclcpp/rclcpp.hpp"
#include "ackermann_msgs/msg/ackermann_drive.hpp"

using std::placeholders::_1;

class VelocityTranslator : public rclcpp::Node
{
public:
    VelocityTranslator()
    : Node("velTranslator")
    {
        // Connect to pigpio daemon (no sudo needed)
        pi_ = pigpio_start(NULL, NULL);

        if (pi_ < 0) {
            RCLCPP_FATAL(this->get_logger(), "Failed to connect to pigpiod daemon");
            throw std::runtime_error("pigpio_start failed");
        }

        // Open UART once
        handle_ = serial_open(pi_, "/dev/serial0", 115200, 0);
        if (handle_ < 0) {
            RCLCPP_FATAL(this->get_logger(), "Failed to open UART");
            pigpio_stop(pi_);
            throw std::runtime_error("serial_open failed");
        }

        subscription_ = this->create_subscription<ackermann_msgs::msg::AckermannDrive>(
            "ackDrive", 10,
            std::bind(&VelocityTranslator::topic_callback, this, _1)
        );

        RCLCPP_INFO(this->get_logger(), "VelocityTranslator initialized.");
    }

    ~VelocityTranslator()
    {
        if (handle_ >= 0) serial_close(pi_, handle_);
        if (pi_ >= 0) pigpio_stop(pi_);
    }

private:

    void topic_callback(const ackermann_msgs::msg::AckermannDrive & msg)
    {
        std::string txt = "Velocity: " + std::to_string(msg.speed) + "\n";
        RCLCPP_INFO(this->get_logger(), txt.c_str());
        uint16_t speed = static_cast<uint16_t>(floorf(msg.speed));
        RCLCPP_INFO(this->get_logger(), "Speed: %d", speed);
        char* bytes = new char[3];
        bytes[0] = 0b11110000;
        bytes[1] = speed / (1 << 8);
        bytes[2] = speed % (1 << 8);
        RCLCPP_INFO(this->get_logger(), "B1: %d", bytes[1]);
        RCLCPP_INFO(this->get_logger(), "B2: %d", bytes[2]);
        int rc = serial_write(pi_, handle_, bytes, txt.size());
        if (rc < 0) {
            RCLCPP_ERROR(get_logger(), "serial_write failed (rc=%d)", rc);
        }
    }

    int pi_ = -1;
    int handle_ = -1;
    rclcpp::Subscription<ackermann_msgs::msg::AckermannDrive>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    try {
        auto node = std::make_shared<VelocityTranslator>();
        rclcpp::spin(node);
    }
    catch (const std::exception &e) {
        std::cerr << "Node failed: " << e.what() << std::endl;
    }

    rclcpp::shutdown();
    return 0;
}