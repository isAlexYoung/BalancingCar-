# BalancingCar 嵌入式项目

**平衡车** 是一个基于 STM32F103C8T6 的嵌入式开发项目，主要实现平衡静止，平衡遥控。

## ✨ 功能介绍

- ✅ 功能点 1，微信小程序蓝牙遥控
- ✅ 功能点 2，OLED打印当前位姿

## 🔧 硬件需求

- **主控芯片**：STM32F103C8T6
- **所需外设**：
  - 0.96寸OLED屏幕
  - JGB37-520霍尔编码器直流电机 12V
  - MP1584EN DC-DC降压模块 输出 3.3V
  - HC-04D蓝牙模块 支持SPP2.1+BLE4.0
  - MPU6050姿态传感器 6轴imu
  - TB6612电机驱动模块
  - 锂电池组12V输出  
- **调试器**：ST-Link

## 💻 软件环境

- **IDE**：Keil MDKv5
- **编译器**：Keil ARM Compiler

## 🚀 快速开始（克隆 + 使用）

1. **克隆仓库**
   ```bash
   git clone https://github.com/用户名/仓库名.git
