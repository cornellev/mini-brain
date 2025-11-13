import rclpy
from rclpy.node import Node
# import string type
from std_msgs.msg import String

def main(args=None):
    rclpy.init(args=args)

    node = Node("test_launch_node")
    # write to topic /test
    publisher = node.create_publisher(String, '/test', 10)
    msg = String()
    msg.data = 'Hello, ROS2!'
    publisher.publish(msg)
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == "__main__":
    main()