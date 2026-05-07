# 测试程序（表 6-2 / §6.3.4）

| 文件 | 内容 | 默认结果 CSV（相对工程目录） |
|------|------|-------------------------------|
| `unit_test_watermark.cpp` | UT-DWT-01、UT-SVD-01、UT-CLAHE-01、UT-PSF-01 | `tests/output/unit_test_results.csv` |
| `integration_test_watermark.cpp` | IT-EM-01：10 幅图无退化 embed→extract | `tests/output/integration_test_it_em01.csv` |
| `perf_test_benchmark.cpp` | §6.3.4 表 6-14：10 幅图平均耗时 | `tests/output/perf_test_results.csv` |

可选参数：

- **单元**：`ColorWatermark.exe [输出.csv]`，缺省为 `tests/output/unit_test_results.csv`。
- **集成**：`ColorWatermark.exe [ch6_images.csv] [watermark.bmp] [输出.csv]`，第三参数为输出 CSV 路径。
- **性能**：`ColorWatermark.exe [--warmup N] [list.csv] [watermark.bmp] [输出.csv]`，与 `experiments/ch6_exp*` 相同占位规则；默认输出 **`tests/output/perf_test_results.csv`**。

公共头文件 **`test_output_csv.h`**：创建 `tests/output` 并打开 UTF-8 CSV（表头与各程序约定列名见上）。

## Visual Studio

1. **排除** `src\main.cpp` 以及所有不参与当前入口的 `experiments\*.cpp`。
2. **仅取消排除** `tests\unit_test_watermark.cpp`（或集成 / 性能三选一）之一。
3. 重新生成；工作目录设为 **ColorWatermark 工程根**（含 `test_images/`、`experiments/ch6_images.csv`）。

集成测试默认读取 `experiments/ch6_images.csv` 与 `test_images/XM_32x32.bmp`。

**说明**：`tests/output/` 下生成的 `.csv` 可由 Git 忽略或纳入版本管理；仓库内保留 `tests/output/.gitkeep` 以占位目录。
