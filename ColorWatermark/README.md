# 命令行与测试运行说明

更完整的**项目结构、输出路径与编译说明**见仓库根目录 **`README.md`**（`ColorWatermark/README.md` 上一级）。

## 可执行文件位置

从解决方案根目录 `ColorWatermark.sln` 所在文件夹编译 **x64 | Debug** 后，主程序为：

`{解决方案根}\x64\Debug\ColorWatermark.exe`

例如：`E:\Projects\VSProjects\ColorWatermark\x64\Debug\ColorWatermark.exe`。

该路径不在 `PATH` 中，PowerShell 里不能直接写 `ColorWatermark.exe`，需使用完整路径或 `&` 调用。

## 工作目录（相对路径）

`--input`、`--watermark` 等若使用相对路径，**当前工作目录**应为含 `test_images` 的工程目录：

`{仓库}\ColorWatermark\ColorWatermark`

## PowerShell 示例（嵌入）

```powershell
$exe = "E:\Projects\VSProjects\ColorWatermark\x64\Debug\ColorWatermark.exe"
Set-Location "E:\Projects\VSProjects\ColorWatermark\ColorWatermark"

& $exe --mode embed `
  --input test_images\color\512\4.2.07.tiff `
  --watermark test_images\XM_32x32.bmp `
  --output output\watermarked.png `
  --alpha 0.05
```

## 单元测试

工程默认编译 **CLI**（`src\main.cpp`）。跑 `tests\unit_test_watermark.cpp` 时：

1. 在 Visual Studio 中打开 `ColorWatermark.vcxproj`。
2. 将 `src\main.cpp` 设为 **在生成中排除**。
3. 将 `tests\unit_test_watermark.cpp` 的 **在生成中排除** 取消。
4. 重新生成；仍输出为 `x64\Debug\ColorWatermark.exe`，在工程目录下运行即可。

集成测试同理，改用 `tests\integration_test_watermark.cpp`。

## 第六章实验可执行文件

各 `experiments\ch6_exp*.cpp` 在 vcxproj 中默认排除；跑某一实验时排除 `main.cpp`、只启用对应 `ch6_exp*.cpp` 后单独编译。CSV 默认写入 `experiments\output\`（文件名 `ch6_*`）。
