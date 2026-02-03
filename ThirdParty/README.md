# MyEngine 第三方库说明

本引擎使用以下第三方库，请按照说明配置：

## 必需库

### 1. GLFW (窗口与输入管理)
- **版本**: 3.3+
- **网址**: https://www.glfw.org/
- **安装方法**:
  ```bash
  # 方式1: Git Submodule
  git submodule add https://github.com/glfw/glfw.git ThirdParty/GLFW
  
  # 方式2: 手动下载
  # 下载并解压到 ThirdParty/GLFW/
  ```

### 2. GLM (数学库)
- **版本**: 0.9.9+
- **网址**: https://github.com/g-truc/glm
- **类型**: Header-Only
- **安装方法**:
  ```bash
  # 方式1: Git Submodule
  git submodule add https://github.com/g-truc/glm.git ThirdParty/glm
  
  # 方式2: 手动下载
  # 下载并解压到 ThirdParty/glm/
  ```

### 3. spdlog (日志库)
- **版本**: 1.11+
- **网址**: https://github.com/gabime/spdlog
- **类型**: Header-Only
- **安装方法**:
  ```bash
  # 方式1: Git Submodule
  git submodule add https://github.com/gabime/spdlog.git ThirdParty/spdlog
  
  # 方式2: 手动下载
  # 下载并解压到 ThirdParty/spdlog/
  ```

## 后续步骤需要的库

### 4. Assimp (模型加载 - Step 11)
- **网址**: https://github.com/assimp/assimp

### 5. stb (图像加载 - Step 11)
- **网址**: https://github.com/nothings/stb

### 6. ImGui (编辑器UI - Step 20)
- **网址**: https://github.com/ocornut/imgui

### 7. Jolt Physics (物理引擎 - Step 25)
- **网址**: https://github.com/jrouwe/JoltPhysics

## 快速配置脚本

### Windows (PowerShell)
```powershell
# 创建 ThirdParty 目录
New-Item -ItemType Directory -Force -Path ThirdParty

# 使用 Git Submodule 添加依赖
git submodule add https://github.com/glfw/glfw.git ThirdParty/GLFW
git submodule add https://github.com/g-truc/glm.git ThirdParty/glm
git submodule add https://github.com/gabime/spdlog.git ThirdParty/spdlog

# 初始化和更新子模块
git submodule update --init --recursive
```

### Linux / macOS (Bash)
```bash
# 创建 ThirdParty 目录
mkdir -p ThirdParty

# 使用 Git Submodule 添加依赖
git submodule add https://github.com/glfw/glfw.git ThirdParty/GLFW
git submodule add https://github.com/g-truc/glm.git ThirdParty/glm
git submodule add https://github.com/gabime/spdlog.git ThirdParty/spdlog

# 初始化和更新子模块
git submodule update --init --recursive
```

## 使用 vcpkg (可选)

如果你使用 vcpkg 包管理器：

```bash
vcpkg install glfw3:x64-windows
vcpkg install glm:x64-windows
vcpkg install spdlog:x64-windows
```

然后在 CMake 配置时添加：
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=[vcpkg根目录]/scripts/buildsystems/vcpkg.cmake ..
```

## 目录结构验证

配置完成后，ThirdParty 目录结构应该如下：

```
ThirdParty/
├── GLFW/
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
├── glm/
│   ├── glm/
│   └── CMakeLists.txt
└── spdlog/
    ├── include/
    └── CMakeLists.txt
```

## 许可证说明

- **GLFW**: zlib/libpng License
- **GLM**: MIT License
- **spdlog**: MIT License

请确保遵守各库的许可证要求。
