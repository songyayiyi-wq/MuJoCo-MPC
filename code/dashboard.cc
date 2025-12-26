#include "mjpc/dashboard.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace mjpc {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 辅助函数：转换弧度到角度
inline float RadToDeg(float rad) { return rad * 180.0f / M_PI; }
// 辅助函数：转换角度到弧度
inline float DegToRad(float deg) { return deg * M_PI / 180.0f; }

// ============ 构造函数和析构函数 ============
Dashboard::Dashboard() 
    : last_update_time_(0),
      pulse_phase_(0.0f),
      glow_intensity_(0.0f),
      warning_blink_(0.0f),
      animated_speed_(0.0f),
      animated_rpm_(0.0f),
      window_width_(800),
      window_height_(600),
      dash_x_(80.0f),      // 向左移动
      dash_y_(100.0f),     // 向下移动
      dash_width_(700.0f), // 放大宽度
      dash_height_(400.0f),// 放大高度
      scale_(1.0f),
      follow_car_(true) {
    
    // 初始化数据
    data_ = DashboardData();  // 使用默认构造函数
    data_.max_rpm = 8000.0f;  // 设置最大转速
    
    // 设置现代化深色主题
    SetDarkTheme();
}

Dashboard::~Dashboard() = default;

// ============ 初始化函数 ============
void Dashboard::Initialize(int width, int height) {
    window_width_ = width;
    window_height_ = height;
    viewport_width_ = width;
    viewport_height_ = height;
}

void Dashboard::SetViewport(int x, int y, int width, int height) {
    viewport_x_ = x;
    viewport_y_ = y;
    viewport_width_ = width;
    viewport_height_ = height;
}

// ============ 主题设置 ============
void Dashboard::SetDarkTheme() {
    theme_.primary = Color(0.0f, 0.8f, 1.0f);     // 青色（现代科技色）
    theme_.secondary = Color(0.3f, 0.3f, 0.4f);   // 深灰蓝
    theme_.accent = Color(1.0f, 0.4f, 0.0f);      // 橙色
    theme_.background = Color(0.1f, 0.1f, 0.15f, 0.8f);  // 深蓝黑
    theme_.warning = Color(1.0f, 0.3f, 0.3f);     // 红色
    theme_.success = Color(0.0f, 1.0f, 0.4f);     // 亮绿色
}

void Dashboard::SetLightTheme() {
    theme_.primary = Color(0.0f, 0.5f, 0.8f);     // 蓝色
    theme_.secondary = Color(0.9f, 0.9f, 0.95f);  // 浅灰
    theme_.accent = Color(1.0f, 0.5f, 0.0f);      // 橙色
    theme_.background = Color(1.0f, 1.0f, 1.0f, 0.9f);  // 白色
    theme_.warning = Color(1.0f, 0.2f, 0.2f);     // 红色
    theme_.success = Color(0.2f, 0.8f, 0.2f);     // 绿色
}

// ============ 基础绘制函数 ============
void Dashboard::DrawGradientRect(float x, float y, float width, float height,
                                const Color& c1, const Color& c2, bool horizontal) {
    glBegin(GL_QUADS);
    
    if (horizontal) {
        // 水平渐变
        glColor4f(c1.r, c1.g, c1.b, c1.a);
        glVertex2f(x, y);
        glVertex2f(x, y + height);
        
        glColor4f(c2.r, c2.g, c2.b, c2.a);
        glVertex2f(x + width, y + height);
        glVertex2f(x + width, y);
    } else {
        // 垂直渐变
        glColor4f(c1.r, c1.g, c1.b, c1.a);
        glVertex2f(x, y + height);
        glVertex2f(x + width, y + height);
        
        glColor4f(c2.r, c2.g, c2.b, c2.a);
        glVertex2f(x + width, y);
        glVertex2f(x, y);
    }
    
    glEnd();
}

void Dashboard::DrawRoundedRect(float x, float y, float width, float height,
                               float radius, const Color& color) {
    // 简化实现：绘制矩形加圆角
    glColor4f(color.r, color.g, color.b, color.a);
    
    // 绘制中心矩形
    glBegin(GL_QUADS);
    glVertex2f(x + radius, y);
    glVertex2f(x + width - radius, y);
    glVertex2f(x + width - radius, y + height);
    glVertex2f(x + radius, y + height);
    glEnd();
    
    glBegin(GL_QUADS);
    glVertex2f(x, y + radius);
    glVertex2f(x + width, y + radius);
    glVertex2f(x + width, y + height - radius);
    glVertex2f(x, y + height - radius);
    glEnd();
    
    // 绘制四个圆角
    DrawCircle(x + radius, y + radius, radius, color);
    DrawCircle(x + width - radius, y + radius, radius, color);
    DrawCircle(x + width - radius, y + height - radius, radius, color);
    DrawCircle(x + radius, y + height - radius, radius, color);
}

void Dashboard::DrawCircle(float cx, float cy, float radius, const Color& color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    
    const int segments = 32;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex2f(cx + radius * cosf(angle), cy + radius * sinf(angle));
    }
    glEnd();
}

void Dashboard::DrawRing(float cx, float cy, float inner_radius, float outer_radius,
                        float start_angle, float end_angle, const Color& color) {
    glColor4f(color.r, color.g, color.b, color.a);
    
    const int segments = 32;
    glBegin(GL_TRIANGLE_STRIP);
    
    for (int i = 0; i <= segments; i++) {
        float t = static_cast<float>(i) / segments;
        float angle = start_angle + t * (end_angle - start_angle);
        float cos_angle = cosf(angle);
        float sin_angle = sinf(angle);
        
        glVertex2f(cx + inner_radius * cos_angle, cy + inner_radius * sin_angle);
        glVertex2f(cx + outer_radius * cos_angle, cy + outer_radius * sin_angle);
    }
    glEnd();
}

void Dashboard::DrawArc(float cx, float cy, float radius, float start_angle, 
                       float end_angle, float thickness, const Color& color) {
    float inner_radius = radius - thickness * 0.5f;
    float outer_radius = radius + thickness * 0.5f;
    DrawRing(cx, cy, inner_radius, outer_radius, start_angle, end_angle, color);
}

// ============ 现代化效果函数 ============
void Dashboard::DrawGlassEffect(float x, float y, float width, float height) {
    // 玻璃模糊效果
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 基础玻璃色
    Color glass_color(1.0f, 1.0f, 1.0f, 0.1f);
    DrawRoundedRect(x, y, width, height, 15.0f, glass_color);
    
    // 高光效果
    Color highlight_color(1.0f, 1.0f, 1.0f, 0.2f);
    DrawRoundedRect(x + 5, y + 5, width - 10, 20, 8.0f, highlight_color);
    
    glDisable(GL_BLEND);
}

void Dashboard::DrawNeonGlow(float x, float y, float radius, const Color& color, float intensity) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 多层发光效果
    for (int i = 0; i < 3; i++) {
        float glow_radius = radius * (1.0f + intensity * 0.3f * (3 - i));
        float alpha = color.a * intensity * 0.2f * (3 - i) / 3.0f;
        Color glow_color(color.r, color.g, color.b, alpha);
        DrawCircle(x, y, glow_radius, glow_color);
    }
    
    glDisable(GL_BLEND);
}

// ============ 数字和文本绘制 ============
void Dashboard::DrawDigitSevenSegment(float x, float y, int digit, float size, const Color& color) {
    // 七段数码管数字
    const bool segments[10][7] = {
        {1,1,1,1,1,1,0},  // 0
        {0,1,1,0,0,0,0},  // 1
        {1,1,0,1,1,0,1},  // 2
        {1,1,1,1,0,0,1},  // 3
        {0,1,1,0,0,1,1},  // 4
        {1,0,1,1,0,1,1},  // 5
        {1,0,1,1,1,1,1},  // 6
        {1,1,1,0,0,0,0},  // 7
        {1,1,1,1,1,1,1},  // 8
        {1,1,1,1,0,1,1}   // 9
    };
    
    if (digit < 0 || digit > 9) return;
    
    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(size * 0.2f);
    
    // 段a（上横线）
    if (segments[digit][0]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.2f, y);
        glVertex2f(x + size * 0.8f, y);
        glEnd();
    }
    
    // 段b（右上竖线）
    if (segments[digit][1]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.8f, y);
        glVertex2f(x + size * 0.8f, y + size * 0.5f);
        glEnd();
    }
    
    // 段c（右下竖线）
    if (segments[digit][2]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.8f, y + size * 0.5f);
        glVertex2f(x + size * 0.8f, y + size);
        glEnd();
    }
    
    // 段d（下横线）
    if (segments[digit][3]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.2f, y + size);
        glVertex2f(x + size * 0.8f, y + size);
        glEnd();
    }
    
    // 段e（左下竖线）
    if (segments[digit][4]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.2f, y + size * 0.5f);
        glVertex2f(x + size * 0.2f, y + size);
        glEnd();
    }
    
    // 段f（左上竖线）
    if (segments[digit][5]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.2f, y);
        glVertex2f(x + size * 0.2f, y + size * 0.5f);
        glEnd();
    }
    
    // 段g（中横线）
    if (segments[digit][6]) {
        glBegin(GL_LINES);
        glVertex2f(x + size * 0.2f, y + size * 0.5f);
        glVertex2f(x + size * 0.8f, y + size * 0.5f);
        glEnd();
    }
    
    glLineWidth(1.0f);
}

void Dashboard::DrawDigitalNumber(float x, float y, int number, float size, const Color& color) {
    std::string num_str = std::to_string(number);
    float digit_width = size * 0.6f;
    float spacing = size * 0.1f;
    float total_width = num_str.length() * (digit_width + spacing);
    
    float current_x = x - total_width * 0.5f;
    for (char c : num_str) {
        if (c >= '0' && c <= '9') {
            int digit = c - '0';
            DrawDigitSevenSegment(current_x, y, digit, size, color);
        }
        current_x += digit_width + spacing;
    }
}

void Dashboard::DrawText(float x, float y, const std::string& text, float size, const Color& color) {
    // 简化文本绘制（实际项目中应使用字体库）
    glColor4f(color.r, color.g, color.b, color.a);
    glPointSize(size * 0.5f);
    glBegin(GL_POINTS);
    
    // 模拟字母绘制
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        if (c != ' ') {
            // 简单的位置计算
            for (int j = 0; j < 3; j++) {
                glVertex2f(x + i * size * 0.6f + (j * size * 0.1f), 
                          y + (j * size * 0.1f));
            }
        }
    }
    glEnd();
    glPointSize(1.0f);
}

// ============ 平滑动画函数 ============
float Dashboard::SmoothValue(float current, float target, float smoothing) {
    return current + (target - current) * smoothing;
}

void Dashboard::UpdateAnimation(float delta_time) {
    // 更新脉动相位（用于霓虹灯效果）
    pulse_phase_ += delta_time * 2.0f;
    if (pulse_phase_ > 2.0f * M_PI) {
        pulse_phase_ -= 2.0f * M_PI;
    }
    
    // 更新辉光强度
    glow_intensity_ = 0.5f + 0.3f * sinf(pulse_phase_);
    
    // 更新警告闪烁
    if (data_.warning) {
        warning_blink_ += delta_time * 5.0f;
        if (warning_blink_ > 2.0f * M_PI) {
            warning_blink_ -= 2.0f * M_PI;
        }
    } else {
        warning_blink_ = 0.0f;
    }
    
    // 平滑速度动画
    animated_speed_ = SmoothValue(animated_speed_, data_.speed_kmh, 0.1f);
    animated_rpm_ = SmoothValue(animated_rpm_, data_.rpm, 0.1f);
}

// ============ 数据更新函数 ============
void Dashboard::Update(const mjModel* m, const mjData* d) {
    if (!m || !d) return;
    
    // 获取当前时间
    double current_time = glfwGetTime();
    float delta_time = 0.0f;
    if (last_update_time_ > 0) {
        delta_time = static_cast<float>(current_time - last_update_time_);
    }
    // 定期输出到终端（每2秒一次）
    static double last_print_time = 0.0;
    if (current_time - last_print_time > 2.0) {  // 每2秒输出一次
        PrintDataToConsole();
        last_print_time = current_time;
    }

    last_update_time_ = current_time;

    // ============ 计算车辆位置和方向 ============


    static float default_cam_pos[3] = {5.0f, 0.0f, 3.0f};
    static float default_cam_forward[3] = {-1.0f, 0.0f, -0.3f};
    static float default_cam_up[3] = {0.0f, 0.0f, 1.0f};
    
    UpdateCameraInfo(default_cam_pos, default_cam_forward, default_cam_up);

    CalculateFollowPosition(m, d);
    
    // ============ 更新车辆数据 ============
    
    // 获取车身ID（假设为第一个body）
    int car_body_id = 0;
    if (m->nbody > 1) {
        // 查找名为"car"或"chassis"的body
        for (int i = 0; i < m->nbody; i++) {
            const char* name = mj_id2name(m, mjOBJ_BODY, i);
            if (name && (strstr(name, "car") || strstr(name, "chassis") || strstr(name, "body"))) {
                car_body_id = i;
                break;
            }
        }
    }
    
    // 获取位置
    int qpos_adr = m->body_dofadr[car_body_id];
    if (qpos_adr + 2 < m->nq) {
        data_.car_x = d->qpos[qpos_adr];
        data_.car_y = d->qpos[qpos_adr + 1];
        data_.car_z = d->qpos[qpos_adr + 2];
    }
    
    // 获取速度
    int qvel_adr = m->body_dofadr[car_body_id];
    if (qvel_adr + 2 < m->nv) {
        double vx = d->qvel[qvel_adr];
        double vy = d->qvel[qvel_adr + 1];
        data_.speed_ms = sqrt(vx * vx + vy * vy);
        data_.speed_kmh = data_.speed_ms * 3.6f;
    }
    
    // ============ 模拟其他数据 ============
    
    // 模拟转速（基于速度）
    data_.rpm = 800.0f + data_.speed_kmh * 60.0f;
    if (data_.rpm > data_.max_rpm) data_.rpm = data_.max_rpm;
    if (data_.rpm < 800.0f) data_.rpm = 800.0f;
    
    // 模拟油量消耗
    static double fuel_level = 100.0;
    fuel_level -= 0.001f * (1.0f + data_.speed_kmh / 100.0f);
    if (fuel_level < 0.0f) fuel_level = 100.0f;
    data_.fuel = fuel_level;
    
    // 模拟温度
    data_.temperature = 60.0f + (data_.rpm / data_.max_rpm) * 40.0f;
    
    // 模拟档位
    if (data_.speed_kmh < 5.0f) data_.gear = 1;
    else if (data_.speed_kmh < 15.0f) data_.gear = 2;
    else if (data_.speed_kmh < 30.0f) data_.gear = 3;
    else if (data_.speed_kmh < 50.0f) data_.gear = 4;
    else if (data_.speed_kmh < 80.0f) data_.gear = 5;
    else data_.gear = 6;
    
    // 模拟控制输入
    data_.throttle = 0.5f + 0.3f * sinf(static_cast<float>(d->time));
    data_.brake = 0.2f + 0.1f * cosf(static_cast<float>(d->time));
    data_.steering = 0.1f * sinf(2.0f * static_cast<float>(d->time));
    
    // 模拟自动驾驶状态
    data_.autopilot = (static_cast<int>(d->time) % 10) < 5;
    data_.mode = data_.autopilot ? "AUTO" : "MANUAL";
    
    // 模拟警告（速度过快或温度过高）
    data_.warning = (data_.speed_kmh > 120.0f) || (data_.temperature > 90.0f);
    
    // 模拟电池电量
    static double battery = 95.0;
    battery -= 0.0005f * (1.0f + data_.speed_kmh / 80.0f);
    if (battery < 20.0f) battery = 95.0f;
    data_.battery_level = battery;
    
    // 模拟行程距离
    static double trip = 0.0;
    trip += data_.speed_ms * delta_time / 1000.0;  // 转换为km
    data_.trip_distance = trip;
    
    // 模拟时间
    data_.time_of_day = fmod(d->time / 60.0, 24.0);  // 24小时制
    
    // 更新动画
    UpdateAnimation(delta_time);
}

// ============ 计算跟随位置 ============
void Dashboard::CalculateFollowPosition(const mjModel* m, const mjData* d) {
    if (!follow_car_) return;
    
    dash_width_ = 700.0f * scale_;
    dash_height_ = 400.0f * scale_;
    
    // 向左移动，避免右边被挡住
    float left_margin = 50.0f * scale_;  // 离左边的距离
    dash_x_ = left_margin+500.0f;
    dash_y_ = window_height_ - dash_height_ - 50.0f;  // 向下移动，离底边有距离
    
    // 如果车辆移动，稍微调整位置（模拟HUD跟随效果）
    if (fabs(data_.speed_ms) > 0.1f) {
        // 根据速度轻微移动
        float move_factor = 0.2f * scale_;
        dash_x_ += data_.car_x * move_factor * 10.0f;
        dash_y_ += data_.car_y * move_factor * 10.0f;
        
        // 限制在屏幕内（确保不会移出左边或右边）
        dash_x_ = std::max(30.0f, std::min(dash_x_, window_width_ - dash_width_ - 30.0f));
        dash_y_ = std::max(50.0f, std::min(dash_y_, window_height_ - dash_height_ - 30.0f)); 
    }
}

// ============ 现代化转速表绘制函数（简化版，无刻度点） ============
void Dashboard::DrawModernTachometer(float x, float y, float radius, float rpm, float max_rpm) {
    // 霓虹光环效果
    float glow = 0.5f + 0.3f * sinf(pulse_phase_);
    Color rpm_glow_color(1.0f, 0.3f, 0.1f, 0.7f);  // 橙色/红色辉光
    DrawNeonGlow(x, y, radius * 1.1f, rpm_glow_color, glow);
    
    // 外环
    DrawRing(x, y, radius * 0.9f, radius, 0.0f, 2.0f * M_PI, 
             Color(0.2f, 0.2f, 0.25f, 0.9f));
    
    // 转速弧 - 根据转速变化颜色
    Color rpm_color;
    float rpm_ratio = rpm / max_rpm;
    
    if (rpm_ratio < 0.4f) {
        rpm_color = Color(0.0f, 1.0f, 0.3f, 0.8f);  // 绿色 - 低转速
    } else if (rpm_ratio < 0.7f) {
        rpm_color = Color(1.0f, 0.8f, 0.0f, 0.8f);  // 黄色 - 中等转速
    } else {
        rpm_color = Color(1.0f, 0.2f, 0.1f, 0.9f);  // 红色 - 高转速
    }
    
    // 绘制转速弧（240度范围）
    float rpm_angle = rpm_ratio * 240.0f;  // 最大240度
    DrawArc(x, y, radius * 0.95f, DegToRad(-120.0f), 
            DegToRad(-120.0f + rpm_angle), radius * 0.05f, rpm_color);
    
    // ============ 绘制指针 ============
    float pointer_angle = DegToRad(-120.0f + rpm_angle);  // 指针角度
    
    // 指针主体 - 红色
    glColor4f(1.0f, 0.2f, 0.1f, 0.9f);  // 红色指针
    glLineWidth(3.0f);
    
    float pointer_length = radius * 0.7f;
    float pointer_tip_x = x + pointer_length * cosf(pointer_angle);
    float pointer_tip_y = y + pointer_length * sinf(pointer_angle);
    
    glBegin(GL_LINES);
    glVertex2f(x, y);  // 指针中心
    glVertex2f(pointer_tip_x, pointer_tip_y);  // 指针尖端
    glEnd();
    
    // 指针尖端装饰
    DrawCircle(pointer_tip_x, pointer_tip_y, 3.0f, Color(1.0f, 0.1f, 0.05f, 1.0f));
    
    // 指针中心圆点
    DrawCircle(x, y, 5.0f, Color(0.1f, 0.1f, 0.1f, 0.9f));
    DrawCircle(x, y, 3.0f, Color(0.8f, 0.2f, 0.1f, 0.9f));
    
    glLineWidth(1.0f);
    
    // ============ 中心显示数字转速 ============
    DrawDigitalNumber(x, y - 15, static_cast<int>(rpm), 12.0f, Color::White());
    
    // 单位标签
    DrawText(x - 20, y + 20, "RPM", 8.0f, Color::LightGray(0.8f));
    
    // ============ 红区指示 ============
    if (rpm_ratio > 0.7f) {
        // 红区闪烁效果
        float redline_alpha = 0.5f + 0.5f * sinf(pulse_phase_ * 4.0f);
        Color redline_color(1.0f, 0.0f, 0.0f, redline_alpha);
        
        // 绘制红区弧段
        float redline_start_angle = DegToRad(-120.0f + 0.7f * 240.0f);
        float redline_end_angle = DegToRad(-120.0f + 240.0f);
        DrawArc(x, y, radius * 0.96f, redline_start_angle, redline_end_angle, 
                radius * 0.08f, redline_color);
        
        // 红区文字
        if (redline_alpha > 0.7f) {
            DrawText(x - 15, y - 35, "REDLINE", 6.0f, Color(1.0f, 0.0f, 0.0f, 0.9f));
        }
    }
}

// ============ 现代化速度表组件（带蓝色指针） ============
void Dashboard::DrawModernSpeedometer(float x, float y, float radius, float speed) {
    // 霓虹光环
    float glow = 0.5f + 0.3f * sinf(pulse_phase_);
    DrawNeonGlow(x, y, radius * 1.1f, theme_.primary, glow);
    
    // 外环
    DrawRing(x, y, radius * 0.9f, radius, 0.0f, 2.0f * M_PI, 
             Color(0.2f, 0.2f, 0.25f, 0.9f));
    
    // 速度弧（根据速度变化颜色）
    Color speed_color = theme_.primary;
    if (speed > 120.0f) {
        speed_color = theme_.warning;
    } else if (speed > 80.0f) {
        speed_color = theme_.accent;
    }
    
    float speed_ratio = std::min(speed / 200.0f, 1.0f);
    float speed_angle = speed_ratio * 240.0f;  // 240度范围
    
    // 绘制速度弧
    DrawArc(x, y, radius * 0.95f, DegToRad(-120.0f), 
            DegToRad(-120.0f + speed_angle), radius * 0.05f, speed_color);
    
    // ============ 绘制蓝色指针 ============
    float pointer_angle = DegToRad(-120.0f + speed_angle);  // 指针角度
    
    // 指针主体 - 蓝色
    glColor4f(theme_.primary.r, theme_.primary.g, theme_.primary.b, 0.9f);  // 蓝色指针
    glLineWidth(3.0f);
    
    float pointer_length = radius * 0.7f;
    float pointer_tip_x = x + pointer_length * cosf(pointer_angle);
    float pointer_tip_y = y + pointer_length * sinf(pointer_angle);
    
    glBegin(GL_LINES);
    glVertex2f(x, y);  // 指针中心
    glVertex2f(pointer_tip_x, pointer_tip_y);  // 指针尖端
    glEnd();
    
    // 指针尖端装饰
    DrawCircle(pointer_tip_x, pointer_tip_y, 3.0f, Color(theme_.primary.r, theme_.primary.g, theme_.primary.b, 1.0f));
    
    // 指针中心圆点
    DrawCircle(x, y, 5.0f, Color(0.1f, 0.1f, 0.1f, 0.9f));
    DrawCircle(x, y, 3.0f, Color(theme_.primary.r, theme_.primary.g, theme_.primary.b, 0.9f));
    
    glLineWidth(1.0f);
    
    // 中心数字速度显示
    DrawDigitalNumber(x, y - 10, static_cast<int>(speed), 15.0f, Color::White());
    
    // 单位标签
    DrawText(x - 15, y + 25, "km/h", 8.0f, Color::LightGray(0.8f));
}

void Dashboard::DrawDigitalSpeed(float x, float y, float size, float speed) {
    // 数字速度显示（特斯拉风格）
    Color bg_color(0.0f, 0.0f, 0.0f, 0.7f);
    DrawRoundedRect(x - size * 0.8f, y - size * 0.3f, 
                   size * 1.6f, size * 0.6f, 5.0f, bg_color);
    
    // 霓虹数字
    float pulse = 0.2f * sinf(pulse_phase_);
    Color digit_color = theme_.primary;
    digit_color.r += pulse;
    digit_color.g += pulse;
    
    DrawDigitalNumber(x, y, static_cast<int>(speed), size, digit_color);
}

void Dashboard::DrawBatteryIndicator(float x, float y, float width, float height, float level) {
    // 电池外框
    Color border_color(0.5f, 0.5f, 0.5f, 0.8f);
    DrawRoundedRect(x, y, width, height, 3.0f, border_color);
    
    // 电池正极头
    float terminal_width = width * 0.1f;
    float terminal_height = height * 0.3f;
    DrawRoundedRect(x + width, y + (height - terminal_height) * 0.5f,
                   terminal_width, terminal_height, 2.0f, border_color);
    
    // 电池电量
    Color battery_color = theme_.success;
    if (level < 30.0f) {
        battery_color = theme_.warning;
    } else if (level < 50.0f) {
        battery_color = theme_.accent;
    }
    
    float fill_width = (level / 100.0f) * (width - 4.0f);
    DrawRoundedRect(x + 2, y + 2, fill_width, height - 4, 2.0f, battery_color);
    
    // 电量百分比
    std::string percent = std::to_string(static_cast<int>(level)) + "%";
    DrawText(x + width * 0.5f - 10.0f, y + height * 0.5f - 4.0f, 
             percent, 8.0f, Color::White(0.9f));
}

void Dashboard::DrawEnergyFlow(float x, float y, float size, float throttle, float regen) {
    // 能量流图示（电动/混合动力汽车）
    float center_x = x;
    float center_y = y;
    
    // 外环
    DrawRing(center_x, center_y, size * 0.8f, size, 0.0f, 2.0f * M_PI,
             Color(0.2f, 0.2f, 0.2f, 0.8f));
    
    // 油门（能量输出）
    if (throttle > 0.01f) {
        float throttle_angle = throttle * 180.0f;
        Color throttle_color = theme_.primary;
        throttle_color.a = 0.7f;
        
        DrawArc(center_x, center_y, size * 0.9f, 
                DegToRad(-90.0f), DegToRad(-90.0f + throttle_angle),
                size * 0.05f, throttle_color);
    }
    
    // 能量回收
    if (regen > 0.01f) {
        float regen_angle = regen * 180.0f;
        Color regen_color = theme_.success;
        regen_color.a = 0.7f;
        
        DrawArc(center_x, center_y, size * 0.9f,
                DegToRad(90.0f), DegToRad(90.0f - regen_angle),
                size * 0.05f, regen_color);
    }
    
    // 中心图标
    DrawText(center_x - 5.0f, center_y - 5.0f, "E", 10.0f, Color::White(0.9f));
}

void Dashboard::DrawAutopilotIndicator(float x, float y, float size, bool active) {
    // 自动驾驶指示器
    Color bg_color = active ? theme_.success : Color(0.3f, 0.3f, 0.3f, 0.8f);
    
    if (active) {
        // 激活时的脉动效果
        float pulse_size = size * (1.0f + 0.1f * sinf(pulse_phase_ * 2.0f));
        DrawNeonGlow(x, y, pulse_size * 0.6f, theme_.success, 0.5f);
    }
    
    DrawCircle(x, y, size * 0.5f, bg_color);
    
    // 图标
    std::string icon = active ? "A" : "M";
    Color icon_color = active ? Color::Black() : Color::White(0.8f);
    DrawText(x - size * 0.15f, y - size * 0.2f, icon, size * 0.4f, icon_color);
    
    // 标签
    std::string label = active ? "AUTO" : "MANUAL";
    DrawText(x - size * 0.5f, y + size * 0.6f, label, size * 0.3f, Color::White(0.8f));
}

void Dashboard::DrawNavigationBar(float x, float y, float width, float height, float heading) {
    // 导航方向条
    Color bg_color(0.0f, 0.0f, 0.0f, 0.7f);
    DrawRoundedRect(x, y, width, height, 5.0f, bg_color);
    
    // 方向刻度
    float center_x = x + width * 0.5f;
    float center_y = y + height * 0.5f;
    
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glLineWidth(1.0f);
    
    // 绘制方向刻度
    for (int angle = 0; angle < 360; angle += 30) {
        float rad_angle = DegToRad(angle - heading);
        float cos_angle = cosf(rad_angle);
        float sin_angle = sinf(rad_angle);
        
        float inner_radius = height * 0.3f;
        float outer_radius = height * 0.4f;
        
        if (angle % 90 == 0) {
            // 主要方向（N, E, S, W）
            outer_radius = height * 0.45f;
            
            // 方向标签
            std::string direction;
            switch (angle) {
                case 0: direction = "N"; break;
                case 90: direction = "E"; break;
                case 180: direction = "S"; break;
                case 270: direction = "W"; break;
            }
            
            float label_x = center_x + outer_radius * cos_angle * 1.2f - 3.0f;
            float label_y = center_y + outer_radius * sin_angle * 1.2f - 5.0f;
            DrawText(label_x, label_y, direction, 8.0f, Color::White(0.9f));
        }
        
        glBegin(GL_LINES);
        glVertex2f(center_x + inner_radius * cos_angle, 
                   center_y + inner_radius * sin_angle);
        glVertex2f(center_x + outer_radius * cos_angle,
                   center_y + outer_radius * sin_angle);
        glEnd();
    }
    
    // 当前方向指示器
    glColor4f(theme_.primary.r, theme_.primary.g, theme_.primary.b, 0.8f);
    glLineWidth(2.0f);
    
    glBegin(GL_TRIANGLES);
    glVertex2f(center_x, center_y - height * 0.25f);
    glVertex2f(center_x - 5.0f, center_y - height * 0.4f);
    glVertex2f(center_x + 5.0f, center_y - height * 0.4f);
    glEnd();
    
    glLineWidth(1.0f);
}

void Dashboard::DrawMinimap(float x, float y, float radius, float car_x, float car_y, float heading) {
    // 小地图（简化版）
    Color bg_color(0.0f, 0.0f, 0.0f, 0.7f);
    DrawCircle(x, y, radius, bg_color);
    
    // 地图网格
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    glLineWidth(1.0f);
    
    for (int i = -2; i <= 2; i++) {
        // 水平线
        glBegin(GL_LINES);
        glVertex2f(x - radius, y + i * radius * 0.4f);
        glVertex2f(x + radius, y + i * radius * 0.4f);
        glEnd();
        
        // 垂直线
        glBegin(GL_LINES);
        glVertex2f(x + i * radius * 0.4f, y - radius);
        glVertex2f(x + i * radius * 0.4f, y + radius);
        glEnd();
    }
    
    // 车辆位置（在小地图中）
    float map_scale = radius * 0.05f;
    float car_map_x = x + car_x * map_scale;
    float car_map_y = y + car_y * map_scale;
    
    // 限制在圆圈内
    float dx = car_map_x - x;
    float dy = car_map_y - y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    if (dist > radius * 0.8f) {
        car_map_x = x + dx * radius * 0.8f / dist;
        car_map_y = y + dy * radius * 0.8f / dist;
    }
    
    // 绘制车辆图标
    glPushMatrix();
    glTranslatef(car_map_x, car_map_y, 0.0f);
    glRotatef(RadToDeg(heading), 0.0f, 0.0f, 1.0f);
    
    Color car_color = theme_.primary;
    car_color.a = 0.9f;
    
    // 三角形表示车辆
    glColor4f(car_color.r, car_color.g, car_color.b, car_color.a);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, -radius * 0.1f);
    glVertex2f(-radius * 0.05f, radius * 0.05f);
    glVertex2f(radius * 0.05f, radius * 0.05f);
    glEnd();
    
    glPopMatrix();
    
    // 小地图边界
    glColor4f(theme_.primary.r, theme_.primary.g, theme_.primary.b, 0.5f);
    glLineWidth(2.0f);
    
    glBegin(GL_LINE_LOOP);
    const int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex2f(x + radius * cosf(angle), y + radius * sinf(angle));
    }
    glEnd();
    glLineWidth(1.0f);
}

// ============ 主渲染函数（重新布局，增加间距） ============
void Dashboard::Render(mjrContext* con, int width, int height) {
    // 更新窗口尺寸
    if (width != window_width_ || height != window_height_) {
        window_width_ = width;
        window_height_ = height;
        CalculateFollowPosition(nullptr, nullptr);
    }
    
    // ============ 保存OpenGL状态 ============
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // 启用混合（透明效果）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // ============ 仪表盘背景 ============
    if (follow_car_) {
        // 跟随模式：半透明现代化背景
        DrawGlassEffect(dash_x_, dash_y_, dash_width_, dash_height_);
        
        // 背景渐变
        Color bg_start(0.05f, 0.05f, 0.08f, 0.85f);
        Color bg_end(0.1f, 0.1f, 0.15f, 0.9f);
        DrawGradientRect(dash_x_, dash_y_, dash_width_, dash_height_, bg_start, bg_end);
        
        // 发光边框
        float border_glow = 0.3f + 0.2f * sinf(pulse_phase_);
        Color border_color = theme_.primary;
        border_color.a = 0.3f * border_glow;
        
        glLineWidth(2.0f);
        glColor4f(border_color.r, border_color.g, border_color.b, border_color.a);
        glBegin(GL_LINE_LOOP);
        glVertex2f(dash_x_, dash_y_);
        glVertex2f(dash_x_ + dash_width_, dash_y_);
        glVertex2f(dash_x_ + dash_width_, dash_y_ + dash_height_);
        glVertex2f(dash_x_, dash_y_ + dash_height_);
        glEnd();
        glLineWidth(1.0f);
    } else {
        // 固定位置：更明显的背景
        Color bg_color(0.0f, 0.0f, 0.0f, 0.9f);
        DrawRoundedRect(dash_x_, dash_y_, dash_width_, dash_height_, 15.0f, bg_color);
    }
    
    // ============ 仪表盘布局（重新设计，增加间距） ============
    float padding = 20.0f * scale_;  // 增加内边距
    float content_width = dash_width_ - 2 * padding;
    float content_height = dash_height_ - 2 * padding;
    
    float current_x = dash_x_ + padding;
    float current_y = dash_y_ + padding;
    
    if (follow_car_) {
        // ============ 跟随模式布局（重新布局，增加间距） ============
        
        // 第一行：速度表和转速表
        float top_height = content_height * 0.45f;  // 增加高度
        
        // 速度表（左侧）- 增加间距
        float speed_radius = top_height * 0.35f;
        float speed_x = current_x + content_width * 0.25f;
        DrawModernSpeedometer(speed_x, 
                             current_y + top_height * 0.5f,
                             speed_radius, animated_speed_);
        
        // 转速表（右侧）- 增加间距
        float tach_x = current_x + content_width * 0.75f;
        DrawModernTachometer(tach_x,
                            current_y + top_height * 0.5f,
                            speed_radius, animated_rpm_, data_.max_rpm);
        
        current_y += top_height + 15.0f;  // 增加行间距
        
        // 第二行：信息面板
        float middle_height = content_height * 0.3f;
        
        // 电池指示器（左侧）- 增加间距
        float battery_width = content_width * 0.25f;
        DrawBatteryIndicator(current_x, 
                            current_y + 10.0f,
                            battery_width, middle_height - 20.0f, data_.battery_level);
        
        // 自动驾驶指示器（中间）- 增加间距
        DrawAutopilotIndicator(current_x + content_width * 0.5f,
                              current_y + middle_height * 0.5f, 
                              20.0f, data_.autopilot);
        
        // 小地图（右侧）- 增加间距
        DrawMinimap(current_x + content_width - battery_width * 0.5f,
                   current_y + middle_height * 0.5f,
                   middle_height * 0.4f, data_.car_x, data_.car_y, data_.car_heading);
        
        current_y += middle_height + 15.0f;  // 增加行间距
        
        // 第三行：导航和其他信息
        float bottom_height = content_height * 0.25f;
        
        // 导航方向条（左侧）- 增加间距
        float nav_width = content_width * 0.6f;
        DrawNavigationBar(current_x, current_y, nav_width, bottom_height, data_.car_heading);
        
        // 档位显示（中间右侧）- 增加间距
        std::string gear_text;
        if (data_.gear == -1) gear_text = "R";
        else if (data_.gear == 0) gear_text = "N";
        else gear_text = std::to_string(data_.gear);
        
        Color gear_color = theme_.primary;
        if (data_.gear == -1) gear_color = theme_.warning;
        
        float gear_x = current_x + nav_width + 30.0f;
        DrawText(gear_x, 
                current_y + bottom_height * 0.5f - 5.0f,
                gear_text, 16.0f, gear_color);
        
        // 温度显示（最右侧）- 增加间距
        std::string temp_text = std::to_string(static_cast<int>(data_.temperature)) + "°C";
        Color temp_color = (data_.temperature > 90.0f) ? theme_.warning : Color::White(0.9f);
        DrawText(gear_x + 50.0f, current_y + bottom_height * 0.5f - 5.0f,
                temp_text, 10.0f, temp_color);
        
    } else {
        // ============ 固定位置布局（重新设计，增加间距） ============
        
        // 顶部标题行
        float title_height = 30.0f;
        DrawText(current_x + content_width * 0.5f - 40.0f,
                current_y + title_height * 0.5f - 5.0f,
                "VEHICLE DASHBOARD", 12.0f, theme_.primary);
        
        current_y += title_height + 20.0f;  // 增加间距
        
        // 第一行：两个主表盘
        float gauge_height = content_height * 0.55f;
        
        // 速度表（左侧）- 增加间距
        float left_width = content_width * 0.45f;
        float speedometer_radius = gauge_height * 0.35f;
        DrawModernSpeedometer(current_x + left_width * 0.5f,
                             current_y + gauge_height * 0.5f,
                             speedometer_radius, animated_speed_);
        
        // 速度标签
        DrawText(current_x + left_width * 0.5f - 25.0f,
                current_y + gauge_height * 0.9f,
                "SPEED", 10.0f, Color::LightGray(0.9f));
        
        // 转速表（右侧）- 增加间距
        float right_width = content_width * 0.45f;
        float tachometer_radius = gauge_height * 0.35f;
        DrawModernTachometer(current_x + content_width - right_width * 0.5f,
                            current_y + gauge_height * 0.5f,
                            tachometer_radius, animated_rpm_, data_.max_rpm);
        
        // 转速标签
        DrawText(current_x + content_width - right_width * 0.5f - 20.0f,
                current_y + gauge_height * 0.9f,
                "RPM", 10.0f, Color::LightGray(0.9f));
        
        current_y += gauge_height + 20.0f;  // 增加间距
        
        // 第二行：信息面板
        float info_height = content_height * 0.45f;
        
        // 左侧信息列
        float col_width = content_width * 0.3f;
        
        // 电池指示器
        DrawBatteryIndicator(current_x,
                           current_y + 10.0f,
                           col_width - 10.0f, 35.0f, data_.battery_level);
        
        // 自动驾驶状态
        DrawAutopilotIndicator(current_x + col_width * 0.5f,
                              current_y + 70.0f,
                              18.0f, data_.autopilot);
        
        // 中间信息列
        float center_x = current_x + col_width + 20.0f;
        
        // 档位显示
        std::string gear_text;
        if (data_.gear == -1) gear_text = "REVERSE";
        else if (data_.gear == 0) gear_text = "NEUTRAL";
        else gear_text = "GEAR " + std::to_string(data_.gear);
        
        Color gear_color = theme_.primary;
        if (data_.gear == -1) gear_color = theme_.warning;
        
        DrawText(center_x + col_width * 0.5f - 35.0f,
                current_y + 30.0f, gear_text, 12.0f, gear_color);
        
        // 温度显示
        std::string temp_text = "TEMP: " + std::to_string(static_cast<int>(data_.temperature)) + "°C";
        Color temp_color = (data_.temperature > 90.0f) ? theme_.warning : Color::White(0.9f);
        DrawText(center_x + col_width * 0.5f - 40.0f,
                current_y + 60.0f, temp_text, 10.0f, temp_color);
        
        // 能量流
        DrawEnergyFlow(center_x + col_width * 0.5f,
                      current_y + 90.0f,
                      25.0f, data_.throttle, data_.brake * 0.5f);
        
        // 右侧信息列
        float right_x = center_x + col_width + 20.0f;
        
        // 小地图
        DrawMinimap(right_x + col_width * 0.5f,
                   current_y + info_height * 0.5f,
                   col_width * 0.4f, data_.car_x, data_.car_y, data_.car_heading);
        
        // 导航方向
        DrawNavigationBar(right_x + 5.0f,
                         current_y + 100.0f,
                         col_width - 10.0f, 40.0f, data_.car_heading);
    }
    
    // ============ 警告指示器 ============
    if (data_.warning) {
        float warning_alpha = 0.5f + 0.5f * sinf(warning_blink_);
        Color warning_color = theme_.warning;
        warning_color.a = warning_alpha;
        
        // 警告边框
        glLineWidth(3.0f);
        glColor4f(warning_color.r, warning_color.g, warning_color.b, warning_color.a);
        glBegin(GL_LINE_LOOP);
        glVertex2f(dash_x_, dash_y_);
        glVertex2f(dash_x_ + dash_width_, dash_y_);
        glVertex2f(dash_x_ + dash_width_, dash_y_ + dash_height_);
        glVertex2f(dash_x_, dash_y_ + dash_height_);
        glEnd();
        glLineWidth(1.0f);
        
        // 警告图标
        if (warning_alpha > 0.7f) {
            DrawText(dash_x_ + dash_width_ * 0.5f - 10.0f,
                    dash_y_ + dash_height_ * 0.5f - 5.0f,
                    "!", 15.0f, warning_color);
        }
    }
    
    // ============ 恢复OpenGL状态 ============
    glDisable(GL_BLEND);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}
// ============ 终端输出函数 ============
void Dashboard::PrintDataToConsole() const {
    printf("\n");
    printf("========================================\n");
    printf("        汽车仪表盘 - 实时数据          \n");
    printf("========================================\n");
    
    // 1. 速度信息
    printf("🚗 速度信息:\n");
    printf("   当前速度: %6.1f km/h  | %5.1f m/s\n", 
           data_.speed_kmh, data_.speed_ms);
    printf("   发动机转速: %6.0f RPM\n", data_.rpm);
    printf("   加速度: %6.2f m/s²\n", data_.acceleration);
    
    // 2. 车辆状态
    printf("📊 车辆状态:\n");
    printf("   档位: ");
    if (data_.gear == -1) printf("R (倒车)\n");
    else if (data_.gear == 0) printf("N (空挡)\n");
    else printf("%d档\n", data_.gear);
    
    printf("   燃油量: %5.1f%%\n", data_.fuel);
    printf("   温度: %5.1f°C %s\n", 
           data_.temperature,
           data_.temperature > 90.0f ? "⚠️" : "");
    
    // 3. 控制输入
    printf("🎮 控制输入:\n");
    printf("   油门: %5.1f%%  ", data_.throttle * 100.0f);
    DrawProgressBar(data_.throttle, 20);
    
    printf("   刹车: %5.1f%%  ", data_.brake * 100.0f);
    DrawProgressBar(data_.brake, 20);
    
    printf("   转向: %+6.1f°  ", data_.steering * 90.0f);
    DrawSteeringBar(data_.steering, 20);
    
    // 4. 驾驶模式
    printf("🤖 驾驶模式:\n");
    printf("   模式: %s %s\n", 
           data_.mode.c_str(),
           data_.autopilot ? "🟢" : "🔴");
    printf("   警告状态: %s\n", 
           data_.warning ? "⚠️ 有警告" : "✅ 正常");
    
    // 5. 能源系统
    printf("🔋 能源系统:\n");
    printf("   电池电量: %5.1f%%  ", data_.battery_level);
    DrawProgressBar(data_.battery_level / 100.0f, 20);
    
    // 6. 位置信息
    printf("📍 位置信息:\n");
    printf("   位置: X=%+6.2f, Y=%+6.2f, Z=%+6.2f\n",
           data_.car_x, data_.car_y, data_.car_z);
    printf("   朝向: %6.1f°\n", RadToDeg(data_.car_heading));
    printf("   行驶距离: %6.2f km\n", data_.trip_distance);
    
    // 7. 时间信息
    printf("🕒 时间信息:\n");
    int hour = static_cast<int>(data_.time_of_day);
    int minute = static_cast<int>((data_.time_of_day - hour) * 60.0);
    printf("   当前时间: %02d:%02d\n", hour, minute);
    
    // 8. 仪表盘状态
    printf("📱 仪表盘状态:\n");
    printf("   位置: (%.0f, %.0f) | 尺寸: %.0f×%.0f\n",
           dash_x_, dash_y_, dash_width_, dash_height_);
    printf("   跟随模式: %s | 缩放: %.1fx\n",
           follow_car_ ? "开启" : "关闭", scale_);
    
    printf("========================================\n");
    printf("\n");
}

// ============ 辅助绘制函数 ============
void Dashboard::DrawProgressBar(float value, int width) const {
    printf("[");
    int filled = static_cast<int>(value * width);
    for (int i = 0; i < width; i++) {
        if (i < filled) printf("█");
        else printf(" ");
    }
    printf("]\n");
}

void Dashboard::DrawSteeringBar(float value, int width) const {
    int center = width / 2;
    int pos = center + static_cast<int>(value * center);
    
    printf("[");
    for (int i = 0; i < width; i++) {
        if (i == center) printf("|");
        else if (i == pos) printf("▲");
        else printf(" ");
    }
    printf("]\n");
}
// ============ 更新摄像机信息 ============
void Dashboard::UpdateCameraInfo(const float* cam_pos, const float* cam_forward, const float* cam_up) {
    // 复制摄像机位置
    cam_pos_[0] = cam_pos[0];
    cam_pos_[1] = cam_pos[1];
    cam_pos_[2] = cam_pos[2];
    
    // 复制摄像机前向向量
    cam_forward_[0] = cam_forward[0];
    cam_forward_[1] = cam_forward[1];
    cam_forward_[2] = cam_forward[2];
    
    // 复制摄像机上向量
    cam_up_[0] = cam_up[0];
    cam_up_[1] = cam_up[1];
    cam_up_[2] = cam_up[2];
    
    // 计算右向量（前向×上）
    cam_right_[0] = cam_forward_[1] * cam_up_[2] - cam_forward_[2] * cam_up_[1];
    cam_right_[1] = cam_forward_[2] * cam_up_[0] - cam_forward_[0] * cam_up_[2];
    cam_right_[2] = cam_forward_[0] * cam_up_[1] - cam_forward_[1] * cam_up_[0];
}

// ============ 3D到2D投影 ============
bool Dashboard::Project3DTo2D(float x, float y, float z, float& screen_x, float& screen_y) {
    // 简化投影：将3D点投影到摄像机平面
    
    // 计算点到摄像机的向量
    float dx = x - cam_pos_[0];
    float dy = y - cam_pos_[1];
    float dz = z - cam_pos_[2];
    
    // 计算在摄像机坐标系中的位置
    float dot_forward = dx * cam_forward_[0] + dy * cam_forward_[1] + dz * cam_forward_[2];
    float dot_right = dx * cam_right_[0] + dy * cam_right_[1] + dz * cam_right_[2];
    float dot_up = dx * cam_up_[0] + dy * cam_up_[1] + dz * cam_up_[2];
    
    // 如果点在摄像机后面，不显示
    if (dot_forward < 0.1f) return false;
    
    // 简单的透视投影
    float scale = 500.0f / dot_forward;  // 透视投影因子
    
    // 转换为屏幕坐标（原点在屏幕中心）
    screen_x = window_width_ / 2.0f + dot_right * scale;
    screen_y = window_height_ / 2.0f - dot_up * scale;  // Y轴向下
    
    // 检查是否在屏幕内
    return (screen_x >= 0 && screen_x <= window_width_ && 
            screen_y >= 0 && screen_y <= window_height_);
}

} // namespace mjpc
