# 测试程序（表 6-2）

| 文件 | 内容 |
|------|------|
| `unit_test_watermark.cpp` | UT-DWT-01、UT-SVD-01、UT-CLAHE-01、UT-PSF-01 |
| `integration_test_watermark.cpp` | IT-EM-01：10 幅图无退化 embed→extract，要求 100% |

## Visual Studio

1. 在项目中**排除** `src\main.cpp` 及所有 `experiments\*.cpp`、`demos\*.cpp`。
2. 仅**包含** `tests\unit_test_watermark.cpp`（或 `integration_test_watermark.cpp`）一项为参与生成。
3. 包含路径已含 `include`、`experiments`（见 `.vcxproj`）。

集成测试默认读取 `experiments/ch6_images.csv` 与 `test_images/XM_32x32.bmp`。
