import rclpy
from rclpy.node import Node
from sensor_msgs.msg import PointCloud2, PointField
import socket
import struct
import threading
import math


class RadarPointCloudNode(Node):
    def __init__(self):
        super().__init__('radar_pointcloud')

        self.publisher_ = self.create_publisher(PointCloud2, 'radar/locations', 10)
        
        # Parameters
        self.declare_parameter('host', '127.0.0.1')
        self.declare_parameter('port', 5005)
        self.declare_parameter('buffer_size', 4096)

        self.host = self.get_parameter('host').value
        self.port = self.get_parameter('port').value

        self.buffer_size = self.get_parameter(self.buffer_size).value
        
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        self.get_logger().warning(f'Listening for radar data on {self.host}:{self.port}')

        self.thread = threading.Thread(target=self._recv_loop, daemon=True)
        self.thread.start()

    def _recv_loop(self):
        while rclpy.ok():
            try:
                data, _ = self.sock.recvfrom(self.buffer_size)
                msg = self._parse(data)
                if msg:
                    self.publisher_.publish(msg)
            except Exception as e:
                self.get_logger().error(f'Error: {e}')

    def _parse(self, data):
        
        # Expect each point as 3 packed float32: range, azimuth, elevation
        point_size = 12  # 3 * float32
        num_points = len(data) // point_size

        if num_points == 0:
            return None

        xyz_bytes = bytearray()

        for i in range(num_points):
            offset = i * point_size
            r, az, el = struct.unpack_from('3f', data, offset)

            # Angles in radians — convert here if yours are in degrees
            # az = math.radians(az)
            # el = math.radians(el)

            x = r * math.cos(el) * math.cos(az)
            y = r * math.cos(el) * math.sin(az)
            z = r * math.sin(el)

            xyz_bytes += struct.pack('3f', x, y, z)

        msg = PointCloud2()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'radar'

        msg.height = 1
        msg.width = num_points
        msg.is_dense = True
        msg.is_bigendian = False
        msg.point_step = 12           # x, y, z as float32
        msg.row_step = msg.point_step * num_points

        msg.fields = [
            PointField(name='x', offset=0,  datatype=PointField.FLOAT32, count=1),
            PointField(name='y', offset=4,  datatype=PointField.FLOAT32, count=1),
            PointField(name='z', offset=8,  datatype=PointField.FLOAT32, count=1),
        ]

        msg.data = bytes(xyz_bytes)
        return msg


def main(args=None):
    rclpy.init(args=args)
    node = RadarPointCloudNode()
    rclpy.spin(node)
    rclpy.shutdown()