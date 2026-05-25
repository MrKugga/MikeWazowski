import rclpy
from rclpy.node import Node
from sensor_msgs.msg import PointCloud2, PointField
from std_msgs.msg import Header
import socket
import struct
import threading
import array

# ── Wire format constants (must match udp_radar_packet.h) ─────────────────
RADAR_UDP_MAGIC   = 0x41756469 # Audi
HEADER_FORMAT     = '<IIIHH'       # little-endian: magic, cycle, timestamp_us, near_count, far_count
HEADER_SIZE       = struct.calcsize(HEADER_FORMAT)  # = 16 bytes

POINT_FORMAT      = '<ffffffB'     # little-endian: x, y, z, velocity, rcs, snr, pdh0
POINT_SIZE        = struct.calcsize(POINT_FORMAT)   # = 25 bytes

# PointCloud2 field definitions
FIELDS = [
    PointField(name='x',        offset=0,  datatype=PointField.FLOAT32, count=1),
    PointField(name='y',        offset=4,  datatype=PointField.FLOAT32, count=1),
    PointField(name='z',        offset=8,  datatype=PointField.FLOAT32, count=1),
    PointField(name='velocity', offset=12, datatype=PointField.FLOAT32, count=1),
    PointField(name='rcs',      offset=16, datatype=PointField.FLOAT32, count=1),
    PointField(name='snr',      offset=20, datatype=PointField.FLOAT32, count=1),
    PointField(name='pdh0',     offset=24, datatype=PointField.UINT8,   count=1),
]
POINT_STEP = POINT_SIZE  # = 25 bytes per point


class RadarUdpReceiverNode(Node):

    def __init__(self):
        super().__init__('radar_udp_receiver')

        # Parameters
        self.declare_parameter('host',        '127.0.0.1')
        self.declare_parameter('port',        5000)
        self.declare_parameter('buffer_size', 65507)
        self.declare_parameter('frame_id',    'radar_link')
        self.declare_parameter('near_topic',  '/radar/near')
        self.declare_parameter('far_topic',   '/radar/far')

        host        = self.get_parameter('host').value
        port        = self.get_parameter('port').value
        buffer_size = self.get_parameter('buffer_size').value
        self.frame_id    = self.get_parameter('frame_id').value
        near_topic  = self.get_parameter('near_topic').value
        far_topic   = self.get_parameter('far_topic').value

        # Publishers
        self.near_pub = self.create_publisher(PointCloud2, near_topic, 10)
        self.far_pub  = self.create_publisher(PointCloud2, far_topic,  10)

        # UDP socket — bind to all interfaces so ADTF can reach us
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((host, port))
        self.buffer_size = buffer_size

        self.get_logger().info(
            f'Radar UDP receiver listening on {host}:{port}')
        self.get_logger().info(
            f'Publishing near → {near_topic}  far → {far_topic}')

        # Receive loop in background thread
        self.running = True
        self.thread = threading.Thread(
            target=self._recv_loop, daemon=True)
        self.thread.start()

    # ── Receive loop ───────────────────────────────────────────────────────

    def _recv_loop(self):
        while self.running and rclpy.ok():
            try:
                data, addr = self.sock.recvfrom(self.buffer_size)
                self._handle_packet(data, addr)
            except OSError:
                break  # socket closed on shutdown
            except Exception as e:
                self.get_logger().error(f'Receive error: {e}')

    # ── Packet parser ──────────────────────────────────────────────────────

    def _handle_packet(self, data: bytes, addr):
        # Minimum size check
        if len(data) < HEADER_SIZE:
            self.get_logger().warning(
                f'Packet too short: {len(data)} bytes from {addr}')
            return

        # Parse header
        magic, cycle, timestamp_us, near_count, far_count = \
            struct.unpack_from(HEADER_FORMAT, data, 0)
        

        # Validate magic
        if magic != RADAR_UDP_MAGIC:
            self.get_logger().warning(
                f'Bad magic: 0x{magic:08X} — expected 0x{RADAR_UDP_MAGIC:08X}')
            return

        # Validate total size
        expected_size = (HEADER_SIZE
                         + near_count * POINT_SIZE
                         + far_count  * POINT_SIZE)
        if len(data) < expected_size:
            self.get_logger().warning(
                f'Packet too short: got {len(data)} '
                f'expected {expected_size} bytes '
                f'(near={near_count} far={far_count})')
            return

        self.get_logger().debug(
            f'Cycle {cycle}  near={near_count}  far={far_count}  '
            f'ts={timestamp_us}us  from {addr}')

        # Build ROS2 timestamp from radar timestamp
        stamp = self.get_clock().now().to_msg()
        # Optionally use radar hardware timestamp:
        # stamp.sec     = timestamp_us // 1_000_000
        # stamp.nanosec = (timestamp_us % 1_000_000) * 1000

        # Parse and publish near detections
        if near_count > 0:
            near_offset = HEADER_SIZE
            near_bytes  = data[near_offset : near_offset + near_count * POINT_SIZE]
            self.near_pub.publish(
                self._build_pointcloud2(near_bytes, near_count, stamp))

        # Parse and publish far detections
        if far_count > 0:
            far_offset = HEADER_SIZE + near_count * POINT_SIZE
            far_bytes  = data[far_offset : far_offset + far_count * POINT_SIZE]
            self.far_pub.publish(
                self._build_pointcloud2(far_bytes, far_count, stamp))

    # ── PointCloud2 builder ────────────────────────────────────────────────

    def _build_pointcloud2(
            self,
            raw_bytes: bytes,
            point_count: int,
            stamp) -> PointCloud2:

        msg = PointCloud2()
        msg.header          = Header()
        msg.header.stamp    = stamp
        msg.header.frame_id = self.frame_id

        msg.height       = 1
        msg.width        = point_count
        msg.fields       = FIELDS
        msg.is_bigendian = False
        msg.point_step   = POINT_STEP
        msg.row_step     = POINT_STEP * point_count
        msg.is_dense     = False

        # Use list of ints — most compatible with all ROS2/CycloneDDS versions
        # msg.data = list(raw_bytes)
        
        cloud = bytes()
        for i in enumerate(raw_bytes):
            x, y, z, vel, rcs, snr, pdh0 = struct.unpack_from(POINT_FORMAT, raw_bytes, 0)
        

        return msg

    # ── Shutdown ───────────────────────────────────────────────────────────

    def destroy_node(self):
        self.running = False
        self.sock.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = RadarUdpReceiverNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()