#include <iostream>
#include <vector>
#include <cstdint>
#include <functional>

#include "rclcpp/rclcpp.hpp"
#include "ackermann_msgs/msg/ackermann_drive.hpp"
// include spicomms.cpp
#include "spicomms.cpp"

const std::string dev = "/dev/spidev0.1";
uint32_t speedHz = 1'000'000;  // 1 MHz

SpiDevice spi(dev, speedHz, SPI_MODE_0, 8);

class SPINode : public rclcpp::Node {
    public:
        SPINode() : Node("spi_node") {
            subscription_ = this->create_subscription<ackermann_msgs::msg::AckermannDrive>("ackermann", 10, std::bind(&SPINode::handle_ackermann_update, this, std::placeholders::_1));
        }

    private:

    void handle_ackermann_update(const ackermann_msgs::msg::AckermannDrive::SharedPtr msg) const {
        //RCLCPP_INFO(this->get_logger(), "Received Ackermann Drive - Speed: '%f', Steering Angle: '%f'", msg->speed, msg->steering_angle);
        double speed = msg->speed;
        double steering_angle = msg->steering_angle;
        // covnvert speed and steering angle to SPI data
        std::vector<uint8_t> tx_data;
        tx_data.reserve(1 + sizeof(speed) + sizeof(steering_angle) + 1);
        // total size: 1 + 8 + 8 + 1 = 18 bytes

        // Start byte (helps the receiver align to frames)
        tx_data.push_back(0xAA);

        auto append_double = [&tx_data](double value) {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
            tx_data.insert(tx_data.end(), p, p + sizeof(value));
        };

        append_double(speed);
        append_double(steering_angle);

        // Simple XOR checksum over everything after the start byte
        uint8_t checksum = 0;
        for (std::size_t i = 1; i < tx_data.size(); ++i) {
            checksum ^= tx_data[i];
        }
        tx_data.push_back(checksum);

        // RCLCPP_INFO(this->get_logger(), "SPI write successful, sent %zu bytes", tx_data.size());
        // RCLCPP_INFO(this->get_logger(), "Data bytes:");
        // for (size_t i = 0; i < tx_data.size(); ++i) {
        //     RCLCPP_INFO(this->get_logger(), "Byte %zu: 0x%02X", i, tx_data[i]);
        // }

        // --- Actually send over SPI ---
        try {
            for (size_t i = 0; i < tx_data.size(); ++i) {
                std::vector<uint8_t> byte_to_send = {tx_data[i]};
                spi.write(byte_to_send);
            }
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "SPI write failed: %s", e.what());
        }
    }

    rclcpp::Subscription<ackermann_msgs::msg::AckermannDrive>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) {
    std::cout << "Initialized spi device!\n";

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SPINode>());
    rclcpp::shutdown();
}