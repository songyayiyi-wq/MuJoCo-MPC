#ifndef MJPC_DASHBOARD_DATA_H_
#define MJPC_DASHBOARD_DATA_H_

#include <string>

namespace mjpc {

struct DashboardData {
    // 车辆数据
    double speed_ms = 0.0;           // 速度 (m/s)
    double speed_kmh = 0.0;          // 速度 (km/h)
    double rpm = 0.0;                // 转速 (转/分钟)
    double fuel = 100.0;             // 油量 (%)
    double temperature = 0.0;        // 温度 (°C)
    int gear = 1;                    // 档位
    double throttle = 0.0;           // 油门 (0-1)
    double brake = 0.0;              // 刹车 (0-1)
    double steering = 0.0;           // 转向 (-1 to 1)
    double acceleration = 0.0;       // 加速度 (m/s²)
    
    // 位置和方向
    double car_x = 0.0, car_y = 0.0, car_z = 0.0;          // 车辆位置
    double car_heading = 0.0;                              // 车辆朝向（弧度）
    
    // 驾驶状态
    bool autopilot = false;           // 自动驾驶状态
    bool warning = false;             // 警告状态
    
    // 其他（保持与旧代码兼容）
    double battery_level = 100.0;     // 电池电量 (%)
    double trip_distance = 0.0;       // 行程距离 (km) - 旧代码中的 distance
    double time_of_day = 0.0;         // 时间（模拟）- 旧代码中的 time
    std::string mode = "MANUAL";      // 驾驶模式 - 旧代码中的 mode
    double max_rpm = 8000.0;          // 最大转速
    // 构造函数 - 可选，因为 C++11 的成员初始化已经足够
    DashboardData() = default;
};

}  // namespace mjpc

#endif  // MJPC_DASHBOARD_DATA_H_
