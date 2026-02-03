# MyEngine - Step 0 构建与验证指南

## Step 0 完成状态

✅ 已生成所有必要文件：
- CMakeLists.txt (根目录)
- .gitignore
- Engine/CMakeLists.txt
- Engine/Platform/CMakeLists.txt
- Engine/Core/CMakeLists.txt
- Engine/Rendering/CMakeLists.txt
- Runtime/CMakeLists.txt
- Runtime/main.cpp
- ThirdParty/README.md

## 下一步操作

### 1. 配置第三方库

在继续之前，需要配置第三方库。有两种方式：

#### 方式 A: 使用 Git Submodule (推荐)

在项目根目录执行：

```powershell
# Windows PowerShell
git submodule add https://github.com/glfw/glfw.git ThirdParty/GLFW
git submodule add https://github.com/g-truc/glm.git ThirdParty/glm
git submodule add https://github.com/gabime/spdlog.git ThirdParty/spdlog
git submodule update --init --recursive
```

#### 方式 B: 暂时跳过第三方库（仅验证编译）

如果暂时不想配置第三方库，可以直接编译验证项目结构。第三方库的 CMake 配置使用了条件判断，不会报错。

### 2. 生成构建系统

```powershell
# 创建 build 目录
mkdir build
cd build

# 生成 Visual Studio 项目 (Windows)
cmake ..

# 或者指定生成器
cmake -G "Visual Studio 17 2022" -A x64 ..
```

### 3. 编译项目

```powershell
# 方式 1: 使用 CMake 编译
cmake --build . --config Debug

# 方式 2: 打开 Visual Studio 解决方案
# 双击 build/MyEngine.sln，然后按 F5 运行
```

### 4. 运行程序

```powershell
# 运行可执行文件
.\bin\Debug\MyEngine.exe
```

**预期输出**：
```
========================================
   MyEngine - Hello World!
   Version: 0.1.0
   Step 0: Environment Setup Complete
========================================

Press Enter to exit...
```

## 验证清单

- [ ] 所有 CMakeLists.txt 文件已生成
- [ ] main.cpp 已生成
- [ ] .gitignore 已配置
- [ ] CMake 配置成功（无错误）
- [ ] 项目可以编译（无编译错误）
- [ ] 程序可以运行并输出正确信息
- [ ] 目录结构符合设计

## 目录结构

当前项目结构应该如下：

```
t2/
├── CMakeLists.txt
├── .gitignore
├── Engine/
│   ├── CMakeLists.txt
│   ├── Platform/
│   │   └── CMakeLists.txt
│   ├── Core/
│   │   └── CMakeLists.txt
│   └── Rendering/
│       └── CMakeLists.txt
├── Runtime/
│   ├── CMakeLists.txt
│   └── main.cpp
├── ThirdParty/
│   └── README.md
└── docs/
    └── (现有文档)
```

## 常见问题

### Q1: CMake 找不到编译器
**解决**: 确保安装了 Visual Studio 2019/2022 和 C++ 开发工具

### Q2: 第三方库找不到
**解决**: 
- 检查 ThirdParty/ 目录下是否有对应的库
- 或者暂时跳过，先验证项目结构编译通过

### Q3: 编译报错找不到头文件
**解决**: 
- 目前 Engine 模块还是空的，没有源文件，这是正常的
- 只要 main.cpp 能编译通过即可

## 下一步

Step 0 完成后，可以继续执行：
- **Step 1**: 基础工具类（UUID/Hash/String）
- **Step 2**: 平台抽象层（Window/Timer/FileSystem）

等待您的确认后继续！
