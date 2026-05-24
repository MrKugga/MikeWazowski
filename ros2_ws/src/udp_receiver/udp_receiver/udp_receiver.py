import rclpy
from rclpy.node import Node
from std_msgs.msg import UInt8MultiArray

import socket
import threading


class UdpReceiverNode(Node):
    def __init__(self):
        super().__init__('udp_receiver')

        # Parameters
        self.declare_parameter('host', '127.0.0.1')
        self.declare_parameter('port', 5005)
        self.declare_parameter('buffer_size', 4096)

        host = self.get_parameter('host').value
        port = self.get_parameter('port').value
        self.buffer_size = self.get_parameter('buffer_size').value

        # Publisher
        self.publisher_ = self.create_publisher(UInt8MultiArray, '/udp_receiver/raw_SOMEIP_data', 10)

        # UDP socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((host, port))
        self.get_logger().info(f'Listening on UDP {host}:{port}')

        # Receive loop in a background thread
        self.thread = threading.Thread(target=self._recv_loop, daemon=True)
        self.thread.start()

    def _recv_loop(self):
        while rclpy.ok():
            try:
                data, addr = self.sock.recvfrom(self.buffer_size)
                msg = UInt8MultiArray()
                msg.data = list(data)
                self.publisher_.publish(msg)
                self.get_logger().debug(f'Received {len(data)} bytes from {addr}')
            except Exception as e:
                self.get_logger().error(f'Socket error: {e}')
                break

    def destroy_node(self):
        self.sock.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = UdpReceiverNode()
    try:
        rclpy.spin(node)

    except KeyboardInterrupt:
        pass

    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()