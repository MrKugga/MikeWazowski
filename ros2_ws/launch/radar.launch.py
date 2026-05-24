# launch/radar.launch.py
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([

        # Static transform: map → base_link (map --> car)
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='radar_tf',
            arguments=['0', '0', '0',   # x y z
                       '0', '0', '0',   # yaw pitch roll
                       'map', 'base_link']
        ),
        
        # Static transform: base_link → radar_link (car --> radar)
        # Values from SensorPose_USK.txt
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='radar_tf',
            arguments=['4.017290', '0.159447', '0.674017',   # x y z
                       '0.009928', '0.013534', '3.090410',   # yaw pitch roll
                       'base_link', 'radar_link']
        ),

        # Your radar node
        Node(
            package='playground_package',
            executable='playground_node',
            name='playground_node',
        ),
    ])