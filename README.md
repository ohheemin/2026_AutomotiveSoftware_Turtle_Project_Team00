# 2026 Automotive Software — Simple ROS Project (Team 00)

ROS2 Humble과 Turtlesim으로 구성한 간단한 ROS 프로젝트입니다. 프로젝트는 세 가지 기능을 구현합니다.

- 거북이를 직진과 제자리 회전을 반복하도록 움직입니다.
- 거북이의 heading에 따라 배경색을 바꿉니다.
- 거북이의 위치와 색 정보를 RViz Marker로 시각화합니다.

## Team

| 이름 | 역할 | 담당 |
|---|---|---|
| 오희민 | 팀장 | `moving_turtle` 노드, `hmi` 노드 |
| 정동명 | 팀원 | `color_changer` 노드 |

## System Architecture

```
                 /turtle1/cmd_vel (geometry_msgs/Twist)
 [moving_turtle] ─────────────────────────────────────▶ [turtlesim]
                                                            │
                         /turtle1/pose (turtlesim/Pose)     │
             ┌──────────────────────────────────────────────┤
             ▼                                              ▼
     [color_changer] ── background_r/g/b (parameter) ──▶ [turtlesim]
             │
             │ TurtleColor (custom msg)
             ▼
          [hmi] ── visualization_msgs/Marker ──▶ [rviz2]
```

| 노드 | 주기 | 입력 | 출력 |
|---|---|---|---|
| `moving_turtle` | 100 Hz | – | `/turtle1/cmd_vel` |
| `color_changer` | 10 Hz | `/turtle1/pose` | turtlesim 배경 파라미터, `TurtleColor` |
| `hmi` | 60 Hz | `/turtle1/pose`, `TurtleColor` | `Marker` |

## Nodes

### `moving_turtle` (오희민)
- 이 노드는 `/turtle1/cmd_vel`에 `geometry_msgs/msg/Twist`를 100 Hz로 publish합니다.
- 처음 100 loop 동안은 `linear.x = 2.0`, `angular.z = 0.0`으로 직진합니다.
- 다음 100 loop 동안은 `linear.x = 0.0`, `angular.z = 1.0`으로 제자리 회전합니다.
- 이 두 동작을 계속 반복합니다.

### `color_changer` (정동명)
- 이 노드는 `/turtle1/pose`를 subscribe하고, `theta`를 radian에서 degree로 변환합니다.
- 변환한 값은 `[-180°, 180°)` 범위로 정규화합니다. `Pose.theta`가 float32이기 때문입니다.
- `AsyncParametersClient`로 turtlesim의 `background_r/g/b` 파라미터를 설정합니다. 이 파라미터의 값 범위는 0–255 정수입니다.
- 같은 색을 `TurtleColor` 메시지로 HMI에 publish합니다. 이 메시지의 값 범위는 0.0–1.0 float입니다.

| theta 범위 | 배경 (R, G, B) | TurtleColor |
|---|---|---|
| 0° ≤ θ < 90° | (255, 255, 255) | (1.0, 1.0, 1.0) |
| 90° ≤ θ < 180° | (255, 0, 0) | (1.0, 0.0, 0.0) |
| -180° ≤ θ < -90° | (0, 255, 0) | (0.0, 1.0, 0.0) |
| -90° ≤ θ < 0° | (0, 0, 255) | (0.0, 0.0, 1.0) |

### `hmi` (오희민)
- 이 노드는 `/turtle1/pose`와 `TurtleColor`를 subscribe합니다.
- 두 메시지로 `visualization_msgs/msg/Marker`를 만들어 60 Hz로 publish합니다. Marker의 각 필드는 다음과 같습니다.
  - `header.frame_id`는 `"map"`이고, `header.stamp`는 현재 ROS time입니다.
  - `type`은 `CUBE`이고, `action`은 `ADD`입니다.
  - `pose.position`은 거북이의 (x, y, 0.0)이고, `orientation`은 (0, 0, 0, 1)입니다.
  - `scale`은 (1.0, 1.0, 1.0)입니다.
  - `color.r/g/b`에는 `TurtleColor` 값을 넣고, `color.a`는 1.0입니다.

## Custom Message

`TurtleColor.msg`
```
float32 red
float32 green
float32 blue
```

## Build & Run

```bash
# workspace 루트에서
colcon build
source install/setup.bash

# turtlesim, 3개 노드, rviz2를 한 번에 실행 (XML launch)
ros2 launch <launch_package> <launch_file>.launch.xml
```

노드 연결은 `rqt_graph`로, 메시지 내용은 `ros2 topic echo`로 확인합니다.

```bash
rqt_graph
ros2 topic echo /turtle1/cmd_vel
ros2 topic echo <marker_topic>
```

## Directory Structure

```
src/
├── <msg_package>/        # TurtleColor.msg
├── moving_turtle/        # 오희민
├── color_changer/        # 정동명
├── hmi/                  # 오희민
└── <launch_package>/     # launch.xml, rviz config
```

> `build/`, `install/`, `log/`는 `.gitignore`로 제외합니다.
