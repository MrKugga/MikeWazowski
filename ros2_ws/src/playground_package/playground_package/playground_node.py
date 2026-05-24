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
        self.publisherNear_ = self.create_publisher(PointCloud2, 'radar/locations/near', 10)
        self.publisherFar_ = self.create_publisher(PointCloud2, 'radar/locations/far', 10)
        
        # Parameters
        self.declare_parameter('host', '127.0.0.1')
        self.declare_parameter('port', 5000)
        self.declare_parameter('buffer_size', 4096)
        self.declare_parameter('frame_id', 'radar_link')
        
        self.host = self.get_parameter('host').value
        self.port = self.get_parameter('port').value
        
        self.frame_id = self.get_parameter('frame_id').value
        self.buffer_size = self.get_parameter('buffer_size').value
        
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        
        self.get_logger().info(f'Listening for radar scans on {self.host}:{self.port}')
        
        self.thread = threading.Thread(target=self._recv_loop, daemon=True)
        
        self.thread.start()

    def _recv_loop(self):
       
        while rclpy.ok():
            try:
                data, _ = self.sock.recvfrom(self.buffer_size)
                msgNear, msgFar = self._parse(data)
                
                if msgNear:
                    self.publisherNear_.publish(msgNear)
                if msgFar:
                    self.publisherFar_.publish(msgFar)
            
            except Exception as e:
                self.get_logger().error(f'Error: {e}')
    
    
    def _parse(self, data):
    
    
        # Data structure:
        # | n. of near points (uint32) | n. of far points (uint32) | data | 
        num_points_near, num_points_far = struct.unpack_from('2I', data, 0)

        
        # Expect each point as 3 packed float32: range, azimuth, elevation
        point_size = 12  # 3 * float32
        num_points_total = len(data) // point_size
        
        if num_points_total != num_points_near + num_points_far:
            self.get_logger().warning("Size mismatch")
            return None, None
        
        xyz_bytes_near = bytearray()
        xyz_bytes_far = bytearray()
        
        for i in range(num_points_total):
            
            offset = i * point_size + 8 # Header length
            r, az, el = struct.unpack_from('3f', data, offset)
            x = r * math.cos(el) * math.cos(az)
            y = r * math.cos(el) * math.sin(az)
            z = r * math.sin(el)
            
            if i < num_points_near:
                xyz_bytes_near += struct.pack('3f', x, y, z)
            else:
                xyz_bytes_far += struct.pack('3f', x, y, z) 
        
        msg = PointCloud2()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = self.frame_id
        msg.height = 1
        msg.is_dense = True
        msg.is_bigendian = False
        msg.point_step = 12           # x, y, z as float32
        msg.fields = [
                    PointField(name='x', offset=0,  datatype=PointField.FLOAT32, count=1),
                    PointField(name='y', offset=4,  datatype=PointField.FLOAT32, count=1),
                    PointField(name='z', offset=8,  datatype=PointField.FLOAT32, count=1),
                ]
        
        msgNear = msg
        msgFar = msg
        
        msgNear.width = num_points_near
        msgFar.width = num_points_far
        
        msgNear.row_step = msg.point_step * num_points_near
        msgFar.row_step = msg.point_step * num_points_far
        
        msgNear.data = bytes(xyz_bytes_near)
        msgFar.data = bytes(xyz_bytes_far)
        
        return msgNear, msgFar
    
def main(args=None):
    rclpy.init(args=args)
    node = RadarPointCloudNode()
    rclpy.spin(node)
    rclpy.shutdown()