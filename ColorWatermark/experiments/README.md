# 第六章实验程序

消融时仅对**待检测图**做预处理、**参考载体不增强**（`extract(CLAHE(I'), I_orig)`）。在这种设定下，差分统计量 \((\Sigma_{det}-\Sigma_{orig})/\alpha\) 的两侧来自**不同光照/对比度域**，与嵌入时“干净域”的奇异值关系不再一致，常出现 **CLAHE 列与无预处理列几乎相同**或**提升为 0** 的现象，属于**方法口径问题**，未必是 CLAHE 未改变像素。

**本目录程序默认已改为域对齐**（与当前 `WatermarkSystem` 一致）：

- `extract(CLAHE(I'), CLAHE(I_orig), α)`，公平比较“是否对退化图做 CLAHE”。

若需**严格复现**旧稿单侧参考，运行实验时加参数 **`--word-protocol`**（见 `ch6_exp2`、`ch6_exp3`）。建议正文或脚注中说明：表 6-4 / 6-5 以域对齐结果为准，或同时报告两种口径。

> `ch6_exp2` 的 CSV 已增加 `mean_abs_clahe_minus_low` 列，用于检查 CLAHE 是否实际改变待检图（该值应明显大于 0）。

## 图像清单

编辑 `ch6_images.csv`（两列：`name,path`），使 10 幅图与表 5-1 名称对应且路径正确。当前文件为占位映射（含不同分辨率条目）；程序会将每幅读入后**统一缩放为 512×512**。

## 固定参数

| 参数 | 值 |
|------|-----|
| α | 0.05 |
| CLAHE | clipLimit=2.0, tileSize=8 |
| 维纳滤波 K | 0.01 |
| 低光照噪声 | 高斯 σ=3 |
| 运动模糊噪声 | 高斯 σ=2 |
| 模糊方向 | 30° |

## 程序与输出 CSV

| 文件 | 对应章节 | 输出 |
|------|----------|------|
| `ch6_exp1_baseline.cpp` | 实验一 | 逐图 PSNR、SSIM、准确率 + MEAN 行 |
| `ch6_exp2_lowlight_clahe.cpp` | 实验二 | β=0.3，无预处理 vs CLAHE + MEAN |
| `ch6_exp3_lowlight_beta_sweep.cpp` | 实验三 | β∈{0.3,0.2,0.1} 的 10 幅均值 |
| `ch6_exp5_motion_wiener.cpp` | 实验五 | L=15，无预处理 vs 维纳 + MEAN |
| `ch6_exp6_motion_length_sweep.cpp` | 实验六 | L∈{10,15,20} 的 10 幅均值 |

### 第六章（`works.md` 表 6-6～6-13 对应）

| 文件 | 对应论文表 | 说明 |
|------|------------|------|
| `ch6_exp4_cliplimit_sweep.cpp` | 表 6-9 | β=0.3，扫描 clipLimit，域对齐 CLAHE，10 幅均值 |
| `ch6_exp7_noise_robustness.cpp` | 表 6-12 | L=15 模糊，退化噪声 σ 扫描，无预处理 vs 维纳 |
| `ch6_exp8_extreme_degradation.cpp` | 表 6-13 | β=0.05 低光照 + CLAHE；L=25 模糊 + 维纳 |

一键合并 CSV：工程目录下运行 `python run_all_experiments.py`（见脚本内说明）。

**§6.3.4 性能测试**已移至 **`tests/perf_test_benchmark.cpp`**，结果 CSV 默认 **`tests/output/perf_test_results.csv`**，见 **`tests/README.md`**。

### Visual Studio：多入口程序

`.vcxproj` 已将各 `ch6_exp*.cpp`、`tests\*.cpp` 及 `fig2_5/fig2_6` demo 设为 **ExcludedFromBuild**。跑某一实验或测试时：

1. 将目标 `.cpp` 的 **ExcludedFromBuild** 改为 `false`；
2. 将 `src\main.cpp` 设为 **ExcludedFromBuild** `true`；
3. 重新生成，得到单一 `ColorWatermark.exe` 即当前入口。

默认配置为 **`src\main.cpp` 参与生成**、其余实验/测试排除。

实验程序默认命令行：

```text
程序.exe [ch6_images.csv] [watermark.bmp] [输出.csv]
```

工作目录设为 **`ColorWatermark` 工程目录**（与 `test_images/` 相对路径一致）。VS 中每次只编译**一个**含 `main` 的 `ch6_exp*.cpp` 或 `tests\*.cpp`。

## 与 Word 中数值的差异说明

文档里的 PSNR/准确率示例来自**旧版实现**（近似小波等）。当前工程已改为 Haar DWT 与双极性嵌入后，**重新运行本程序得到的数值将替换 Word 中旧表**，属预期行为；正文分析框架（对照关系、β/L 梯度）不变。

## 评价指标文字

Word 中比特判决写为「估计值 >0.5 判为 1」。代码中提取端使用双极性 `{−1,+1}` 与零阈值后再二值化，`computeAccuracy` 与之一致；若审稿问及，可在文中改为「与实现一致的阈值判决」。
