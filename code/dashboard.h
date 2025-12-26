#ifndef MJPC_DASHBOARD_H_
#define MJPC_DASHBOARD_H_

#include <mujoco/mujoco.h>
#include <vector>
#include <string>
#include <cstring>

// 包含现有的 dashboard_data.h 文件
#include "dashboard_data.h"

namespace mjpc {

// 现代化的颜色结构体
struct Color {
    float r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : 
        r(r_), g(g_), b(b_), a(a_) {}
    
    // 预定义颜色
    static Color Black(float alpha = 1.0f) { return Color(0.0f, 0.0f, 0.0f, alpha); }
    static Color White(float alpha = 1.0f) { return Color(1.0f, 1.0f, 1.0f, alpha); }
    static Color Red(float alpha = 1.0f) { return Color(1.0f, 0.0f, 0.0f, alpha); }
    static Color Green(float alpha = 1.0f) { return Color(0.0f, 1.0f, 0.0f, alpha); }
    static Color Blue(float alpha = 1.0f) { return Color(0.0f, 0.0f, 1.0f, alpha); }
    static Color Cyan(float alpha = 1.0f) { return Color(0.0f, 1.0f, 1.0f, alpha); }
    static Color Yellow(float alpha = 1.0f) { return Color(1.0f, 1.0f, 0.0f, alpha); }
    static Color Orange(float alpha = 1.0f) { return Color(1.0f, 0.5f, 0.0f, alpha); }
    static Color Purple(float alpha = 1.0f) { return Color(0.5f, 0.0f, 0.5f, alpha); }
    static Color DarkGray(float alpha = 1.0f) { return Color(0.1f, 0.1f, 0.1f, alpha); }
    static Color LightGray(float alpha = 1.0f) { return Color(0.8f, 0.8f, 0.8f, alpha); }
};

class Dashboard {
public:
    Dashboard();
    ~Dashboard();
    
    // ============ 初始化函数 ============
    void Initialize(int width, int height);
    void SetViewport(int x, int y, int width, int height);
    
    // ============ 更新函数 ============
    // 新的更新函数
    void Update(const mjModel* m, const mjData* d);
    
    // 向后兼容的更新函数
    void UpdateData(const mjModel* m, const mjData* d, DashboardData* data) {
        Update(m, d);
        if (data) {
            // 复制数据到外部结构体
            *data = data_;
        }
    }
    
    void UpdateAnimation(float delta_time);
    
    // ============ 渲染函数 ============
    // 新的渲染函数
    void Render(mjrContext* con, int width, int height);
    
    // 向后兼容的渲染函数
    void Render(const DashboardData* data, mjrContext* con, int width, int height) {
        // 忽略传入的 data，使用内部数据
        (void)data;  // 避免未使用参数警告
        Render(con, width, height);
    }
    
    // ============ 现代化仪表盘组件 ============
    void DrawModernSpeedometer(float x, float y, float radius, float speed);
    void DrawModernTachometer(float x, float y, float radius, float rpm, float max_rpm);   
    void DrawDigitalSpeed(float x, float y, float size, float speed);
    void DrawBatteryIndicator(float x, float y, float width, float height, float level);
    void DrawEnergyFlow(float x, float y, float size, float throttle, float regen);
    void DrawAutopilotIndicator(float x, float y, float size, bool active);
    void DrawNavigationBar(float x, float y, float width, float height, float heading);
    void DrawMinimap(float x, float y, float radius, float car_x, float car_y, float heading);
    
    // ============ 设置函数 ============
    void SetFollowCar(bool follow) { follow_car_ = follow; }
    void SetDashboardPosition(float x, float y) { dash_x_ = x; dash_y_ = y; }
    void SetScale(float scale) { scale_ = scale; }
    
    // 获取数据（用于向后兼容）
    const DashboardData& GetData() const { return data_; }
    
    // 调试输出函数
    void PrintDataToConsole() const; 

    // 设置跟随模式
    enum FollowMode {
        FIXED_SCREEN,      // 固定在屏幕上（当前模式）
        FOLLOW_CAR_3D      // 跟随小车3D位置
    };
    
    void SetFollowMode(FollowMode mode) { follow_mode_ = mode; }
    void SetOffsetFromCar(float x, float y, float z) { offset_x_ = x; offset_y_ = y; offset_z_ = z; }
    
    // 更新摄像机信息（用于3D投影）
    void UpdateCameraInfo(const float* cam_pos, const float* cam_forward, const float* cam_up);
private:
    // 数据 - 使用 dashboard_data.h 中的定义
    DashboardData data_;
    float last_update_time_;
    
    // 动画参数
    float pulse_phase_;
    float glow_intensity_;
    float warning_blink_;
    
    // 平滑动画值
    float animated_speed_;
    float animated_rpm_;
    
    // 窗口尺寸
    int window_width_;
    int window_height_;
    int viewport_x_, viewport_y_, viewport_width_, viewport_height_;
    
    // 仪表盘位置和大小
    float dash_x_, dash_y_, dash_width_, dash_height_;
    float scale_;
    bool follow_car_;
   
    // 跟随模式
    FollowMode follow_mode_ = FIXED_SCREEN;
    
    // 相对于小车的偏移量
    float offset_x_ = 0.0f;    // 小车前方的偏移
    float offset_y_ = 0.0f;    // 左右偏移
    float offset_z_ = 2.0f;    // 小车上方的高度
    
    // 摄像机信息（用于3D投影）
    float cam_pos_[3] = {0, 0, 0};
    float cam_forward_[3] = {0, 0, -1};
    float cam_up_[3] = {0, 1, 0};
    float cam_right_[3] = {1, 0, 0};
    
    // 3D投影相关
    bool Project3DTo2D(float x, float y, float z, float& screen_x, float& screen_y);
    
    // 颜色主题
    struct Theme {
        Color primary;
        Color secondary;
        Color accent;
        Color background;
        Color warning;
        Color success;
    } theme_;
    
    // ============ 私有辅助函数 ============
    void SetDarkTheme();
    void SetLightTheme();
    
    void DrawGradientRect(float x, float y, float width, float height,
                         const Color& c1, const Color& c2, bool horizontal = true);
    void DrawRoundedRect(float x, float y, float width, float height,
                        float radius, const Color& color);
    void DrawCircle(float cx, float cy, float radius, const Color& color);
    void DrawRing(float cx, float cy, float inner_radius, float outer_radius,
                  float start_angle, float end_angle, const Color& color);
    void DrawArc(float cx, float cy, float radius, float start_angle, float end_angle,
                 float thickness, const Color& color); 
    void DrawGlassEffect(float x, float y, float width, float height);
    void DrawNeonGlow(float x, float y, float radius, const Color& color, float intensity);
    void DrawDigitSevenSegment(float x, float y, int digit, float size, const Color& color);
    void DrawDigitalNumber(float x, float y, int number, float size, const Color& color);
    void DrawText(float x, float y, const std::string& text, float size, const Color& color);
    void DrawProgressBar(float value, int width) const;
    void DrawSteeringBar(float value, int width) const; 
    // 数据平滑函数
    float SmoothValue(float current, float target, float smoothing);
    
    // 计算跟随位置
    void CalculateFollowPosition(const mjModel* m, const mjData* d);
};

// 向前兼容的辅助函数
inline double& battery(DashboardData& data) { return data.battery_level; }
inline double& time(DashboardData& data) { return data.time_of_day; }
inline double& distance(DashboardData& data) { return data.trip_distance; }

}  // namespace mjpc

#endif  // MJPC_DASHBOARD_H_
