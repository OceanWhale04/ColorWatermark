# ColorWatermark

基于 **DWT–DCT–SVD** 的彩色图像盲水印（嵌入端）与非盲提取（需提供原始载体）实验工程。**语言**：C++（Visual Studio 2022）、**图像库**：OpenCV 4.x。**第六章消融实验**、**单元/集成/性能测试**及论文插图脚本均在同一仓库中维护。

---

## 仓库结构

```
ColorWatermark/                    # 解决方案根目录（含 ColorWatermark.sln）
├── ColorWatermark.sln
├── README.md                      # 本文件：项目总览与运行入口
├── run_all_experiments.py        # 可选：合并 experiments/output 下 CSV
└── ColorWatermark/               # VS 工程目录（含源码与资源路径）
    ├── ColorWatermark.vcxproj
    ├── opencv_debug.props        # OpenCV 包含路径与链接库（按本机修改）
    ├── include/                  # 对外头文件（CLI、WatermarkCodec、WatermarkSystem 等）
    ├── src/                      # 主程序与各模块实现（main.cpp、编解码、预处理、指标）
    ├── experiments/              # 第六章实验入口 *.cpp、ch6_images.csv、exp_common.h
    │   ├── output/               # 消融与其它实验生成的 CSV / 部分 PNG（图 6-8 等）
    │   └── README.md             # 消融口径、域对齐、各 ch6_exp 与输出文件名说明
    ├── tests/                    # 单元 / 集成 / 性能测试（独立 main）
    │   ├── output/               # 测试默认 CSV 输出目录（含 .gitkeep）
    │   ├── test_output_csv.h
    │   └── README.md             # 参数约定与各 CSV 列说明
    ├── demos/                    # 演示与论文插图脚本（legacy / paper_figures）
    └── docs/                     # 论文对齐段落、IMD、diagrams/*.drawio、mmd 等
```

运行 CLI 或测试时，**当前工作目录**应设为内含 **`test_images/`**、**`experiments/ch6_images.csv`** 的 **`ColorWatermark\ColorWatermark`**（即 `.vcxproj` 所在目录），以便相对路径一致。

---

## 环境与编译

| 依赖 | 说明 |
|------|------|
| Visual Studio 2022 | 加载 **`ColorWatermark.sln`**，推荐平台 **x64** |
| OpenCV 4.x | 通过 **`ColorWatermark/opencv_debug.props`** 配置包含目录与 `opencv_world*.lib` |
| Python 3（可选） | 合并 CSV：`python run_all_experiments.py` |

默认 **`src/main.cpp`** 参与生成，产出 **`ColorWatermark.exe`**（输出目录一般为 **`x64\Debug`** 或 **`x64\Release`**，与 `.sln` 同级；具体以 VS 为准）。

### 多入口说明（重要）

工程内还有 **`experiments/ch6_exp*.cpp`**、**`experiments/ch6_fig68_ll_dct_spectrum.cpp`**、**`tests/*.cpp`** 等自带 **`main`** 的程序。Visual Studio 同一时刻只能链接 **一个** `main`，因此：

1. 将 **`src\main.cpp`** 设为 **从生成中排除**；
2. **仅取消排除** 你需要运行的那一个 **`*.cpp`**；
3. 重新生成；可执行文件名仍为 **`ColorWatermark.exe`**。

更细的切换步骤见 **`ColorWatermark/README.md`**（工程内简短说明）、**`experiments/README.md`**、**`tests/README.md`**。

---

## 主程序：命令行水印工具（`main.cpp`）

不带兼容模式参数时，必须通过 **`--mode`** 指定运行模式，否则会打印用法并退出。

常用 **`--mode`**：

| 模式 | 作用 |
|------|------|
| `embed` | 嵌入水印 |
| `extract` | 提取水印（非盲，需提供 `--original`） |
| `compare_clahe` | 原图 / 低光 / CLAHE 三图横拼 |
| `compare_wiener` | 原图 / 模糊 / 维纳复原三图横拼 |

嵌入示例（工作目录：`ColorWatermark\ColorWatermark`）：

```text
ColorWatermark.exe --mode embed ^
  --input test_images\color\512\4.2.07.tiff ^
  --watermark test_images\XM_32x32.bmp ^
  --output output\watermarked.png ^
  --alpha 0.05
```

提取示例（可选预处理 **`none` / `clahe` / `wiener`**）：

```text
ColorWatermark.exe --mode extract ^
  --input degraded.png ^
  --original cover.png ^
  --watermark test_images\XM_32x32.bmp ^
  --output extracted.png ^
  --alpha 0.05 ^
  --preprocess clahe --clipLimit 2.0 --tileSize 8
```

控制台会输出 **PSNR、SSIM（嵌入）** 或 **准确率（提取）**；图像写入 **`--output`**。

---

## 输出路径一览

| 类别 | 目录或文件 | 说明 |
|------|------------|------|
| 第六章消融 CSV | **`ColorWatermark/experiments/output/`** | 各 `ch6_exp*.cpp` 默认写入 `ch6_exp*.csv` 等 |
| 图 6-8 DCT 谱 | 同上 | **`ch6_fig68_dct_spectrum.csv`**、**`ch6_fig68_dct_heatmap.png`**、**`ch6_fig68_dct_zigzag.png`**（由 **`ch6_fig68_ll_dct_spectrum.cpp`** 生成） |
| 单元测试 | **`ColorWatermark/tests/output/unit_test_results.csv`** | 可 `argv[1]` 覆盖路径 |
| 集成测试 | **`ColorWatermark/tests/output/integration_test_it_em01.csv`** | 依赖 `ch6_images.csv`、水印路径 |
| 性能测试 | **`ColorWatermark/tests/output/perf_test_results.csv`** | §6.3.4 表 6-14 类计时 |

一键汇总 **`experiments/output/*.csv`**（工程根或脚本说明目录下执行）：

```bash
python run_all_experiments.py
```

可选参数见脚本内注释。

---

## 第六章实验与消融

- 图像列表：**`ColorWatermark/experiments/ch6_images.csv`**（`name,path`）。
- 固定参数与 **Word / 域对齐** 口径：**`ColorWatermark/experiments/README.md`**。
- 性能基准已放在 **`tests/perf_test_benchmark.cpp`**，输出见上表。

---

## 测试程序

详见 **`ColorWatermark/tests/README.md`**（默认 CSV 路径与 VS 切换步骤）。

---

## 文档与图表源码

- **`ColorWatermark/docs/`**：章节修订稿、消融叙述、`IMD.md` 变更记录、draw.io / Mermaid 图源。
- **`ColorWatermark/demos/README.md`**：演示脚本说明。

---

## 许可证与引用

论文或报告中使用本仓库代码与实验数据时，请注明工程名称及对应章节/表号；若开源许可证尚未填写，可在仓库后续补充 `LICENSE`。
