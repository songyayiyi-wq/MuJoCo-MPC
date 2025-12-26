# MuJoCo MPC 汽车仪表盘可视化系统

## 📌 项目信息
**姓名**：王凌  
**学号**：232011171  
**班级**：计科2305班  
**完成日期**：2025年12月

## 🎯 项目简介
本项目将现代化汽车仪表盘集成到mujoco mpc物理仿真环境中，实时显示车辆速度、转速、油量、温度等数据。仪表盘具有玻璃效果、平滑动画、黑暗/明亮双主题切换等功能，为车辆仿真提供了直观的可视化界面。

## ✨ 功能特性
- ✅ **实时数据监控**：速度、转速、位置、油量、温度
- ✅ **现代化ui设计**：玻璃效果、平滑动画、红区警告
- ✅ **完整仪表组件**：速度表、转速表、数字显示、电池指示器
- ✅ **主题系统**：黑暗/明亮双主题一键切换
- ✅ **优化性能**：顶点预计算、批处理渲染、数据平滑
- ✅ **集成良好**：与mujoco框架无缝集成

## 🖥️ 开发环境
| 环境项 | 配置信息 |
|--------|----------|
| 操作系统 | Ubuntu 22.04 lts |
| 编译器 | gcc 11.3.0 |
| 物理引擎 | MuJoCo MPC (Google Deepmind版本) |
| 图形api | OpenGL / MuJoCo原生渲染 |
| 开发工具 | vscode + cmake + git |
| 项目路径 | `~/mujoco_projects/mujoco_mpc` |

## 🚀 快速开始

### 1. 环境配置

```bash
# 更新系统
sudo apt update && sudo apt upgrade -y

# 安装必要依赖
sudo apt install -y build-essential cmake git \
    libgl1-mesa-dev libglfw3-dev libglew-dev \
    libeigen3-dev libopenblas-dev
```

### 2. 克隆并编译项目
```bash
# 克隆代码（如果你已有项目，跳过此步）
git clone https://github.com/google-deepmind/mujoco_mpc.git
cd mujoco_mpc

# 编译项目
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

3. 运行程序

```bash
# 运行带仪表盘的汽车仿真（simplecar任务）
./bin/mjpc --task=SimpleCar

# 或者指定自定义场景文件
./bin/mjpc --mjcf=../mjpc/tasks/car/car_simple.xml
```

## 🎮 操作说明

## 基本操作

  - 鼠标拖动：旋转视角

  - 鼠标右键拖动：平移视角

  - 滚轮：缩放视角

  - wasd键：控制车辆移动

  - 空格键：重置仿真

## 仪表盘控制

  - t键：切换黑暗/明亮主题

  - d键：显示/隐藏仪表盘

  - r键：重置车辆位置

## 🗂️ 项目结构

```text 
mujoco_projects/mujoco_mpc/
├── build/                 # 编译输出目录
│   └── bin/
│       └── mjpc         # 可执行文件
├── mjpc/
│   ├── dashboard.cc      # 仪表盘模块
│   ├── dashboard_data.h
│   ├── dashboard.h
│   ├── tasks/
│   │   └── simple_car/          # 汽车场景文件
│   │       ├── car_model.xml
│   │       └── task.xml
│   ├── app.cc           # 主程序入口（已集成仪表盘）
│   ├── simulate.cc      # 仿真逻辑（已集成仪表盘）
│   └── render.cc        # 渲染逻辑
└── docs/
    ├── report.md        # 详细技术报告
    ├── screenshots/     # 运行截图
    └── demo.mp4         # 演示视频
```

## 📊 仪表盘组件说明
1. 速度表

    位置：屏幕左下角

    范围：0-200 km/h

    颜色指示：绿(<80) / 黄(<150) / 红(≥150)

2. 转速表

    位置：屏幕左侧中部

    范围：0-8000 rpm

    红区警告：6000-8000 rpm闪烁警告

3. 数字显示面板

    位置：屏幕右下角

    显示内容：

      - 当前速度（数字）

      - 油量百分比

      - 发动机温度

      - 电池电量

      - 档位信息

4. 警告系统

    低油量警告：油量<20%时闪烁

    高温警告：温度>100°c时闪烁

    红区警告：转速>6000 rpm时闪烁

## 🔧 自定义配置
## 修改仪表盘位置

在 simulate.h 中修改 dashboard_settings：

```cpp
struct {
    int x = 20;           // x位置
    int y = 20;           // y位置
    float opacity = 0.9f; // 透明度
    bool enabled = true;  // 启用状态
} dashboard_settings;
```

## 添加新数据源

在 dashboard_data.h 中扩展 dashboarddata 结构体，并在 update() 函数中添加数据提取逻辑。
## 🎬 效果展示

演示视频：
bilibili视频链接:https://www.bilibili.com/video/BV14zBeBMEJ7/


## ⚡ 性能优化

   帧率提升：从30fps优化至58fps（复杂场景）

   内存占用：仅增加约5mb内存

   优化技术：

   1、顶点预计算

   2、批处理渲染

   3、数据平滑算法

## 🐛 常见问题
1. 编译错误：找不到mujoco库

```bash
# 在cmakelists.txt中添加路径
set(mujoco_dir "/home/username/mujoco_projects/mujoco_mpc")
```
2. 仪表盘不显示

确保在渲染函数中正确设置2d模式：

```cpp
gldisable(gl_depth_test);  // 关键：禁用深度测试
glenable(gl_blend);        // 启用透明混合
```
3. 帧率过低

    减少圆形绘制段数

    启用顶点缓存（vbo）

    预计算静态几何体

4. simplecar任务不存在

如果 --task=simplecar 报错，可以使用默认任务：

```bash
./bin/mjpc --task=particle  # 使用粒子任务
# 或加载自定义汽车场景
./bin/mjpc --mjcf=../mjpc/tasks/car/car_simple.xml
```

## 📈 性能测试结果

### 帧率对比
| 测试场景 | 无仪表盘 | 有仪表盘(优化前) | 有仪表盘(优化后) |
|----------|----------|------------------|------------------|
| 简单场景 | 120 fps  | 90 fps           | 110 fps          |
| 复杂场景 | 60 fps   | 45 fps           | 58 fps           |

### 内存占用
| 组件 | 内存占用 | 说明 |
|------|----------|------|
| dashboard对象 | ~5 MB | 顶点缓存、动画数据 |
| 总增加内存 | ~5 MB | 相对基础内存增加约3% |

## 📚 学习收获

通过本项目，我掌握了：

    ✅ 大型c++项目的二次开发能力

    ✅ mujoco物理引擎与opengl集成

    ✅ 实时数据可视化实现

    ✅ 性能优化与调试技巧

    ✅ 现代化ui设计与实现

