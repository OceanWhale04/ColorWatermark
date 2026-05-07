# IMD — 实现与文档变更记录

本文件按时间倒序记录对仓库的代码与文档改动，便于论文与工程对照。  
**约定**：每次合并或提交前可追加一条；Agent 会话内完成的一组相关改动记为同一条。

---

## 2026-05-07（消融实验 CSV 与实验五/六代码）

### 现象（仓库内 `experiments/output/*.csv`）

- **实验二（CLAHE）**：域对齐下 MEAN **accuracy_no_preprocess≈79.57%**，**accuracy_clahe≈79.35%**，**delta≈−0.22 pp**；多幅图两列完全相同，`mean_abs_clahe_minus_low` 仍显著大于 0（CLAHE 改变了像素但多数比特判决未变）。
- **实验五、六（旧版）**：维纳列大幅低于无预处理（约 **−21～−31 pp**），因代码采用 **`extract(wiener(I′), I_orig)`（仅复原待检图）**，与 **`WatermarkSystem`** 中 **`extract(wiener(I′), wiener(I_orig))`** 不一致，差分统计量在两侧变换域不对齐，结论不能支撑「系统默认提取管线」。
- **实验七**：已为双侧维纳；CSV 仍显示无预处理 **>** 维纳（约 83% vs 70%），属算法层现象（见下）。
- **论文中「60%→91%、63%→88%」等**：与**当前仓库 CSV 数值不一致**，若曾出自早期脚本或不同 β/噪声，需在文中注明数据来源或按现行脚本重跑更新。

### 代码修正

- **`ch6_exp5_motion_wiener.cpp`、`ch6_exp6_motion_length_sweep.cpp`**：维纳分支改为对 **cover** 同步 **`Preprocessor::wiener`**，与 **`WatermarkSystem`** 域对齐；需重新编译并**重跑**以刷新 `ch6_exp5_motion_wiener.csv`、`ch6_exp6_motion_lengths.csv`。

---

## 2026-05-07（§4.3 嵌入/提取流程与图 4-5、4-6）

- **新增** `docs/ch4_section_4_3_corrected.md`：**§4.3 全文修订稿**（算法 4-1/4-2、公式 4-1/4-2、图 4-5/4-6 说明），与代码一致，可整体替换 Word 中原 §4.3。

### 正文需更正的逻辑错误（相对当前仓库）

1. **算法 4-1 标题为「嵌入」，所列伪代码实为「提取」**（CLAHE 双侧、`W_bit=(Σ'−Σ)/α` 等）。应按 **`WatermarkCodec::embed`** 重写为：输入载体 **I**、水印 **W**、α；输出 **Iw**；步骤为展平水印映射为 **双极性 {−1,+1}**、`σ' = σ + α·w`、逆变换合并。（公式 4-1 若写作 Σ′=Σ+α·W_bit，须注明实现中 **W_bit 为 ±1**，或与二值 0/1 的换算关系。）
2. **算法 4-2 预处理**：`WatermarkSystem::extract` 在 **clahe / wiener** 时对 **待检测图与原始载体两侧同时**做相同预处理；伪代码若只处理 **I'** 与正文「域对齐」不一致，应改为双侧。（none 时两者均不处理。）
3. **公式 4-2**：应写作 **`W_bit = (Σ' − Σ) / α`**（须括号）；Σ、Σ′ 在代码中为 **每块 DCT 系数矩阵的最大奇异值（标量 σ）**，不是整条对角矩阵。
4. **投票与阈值**：`extract` 中为 **三通道估计值相加后除以 3（算术平均）**，再对每位 **`estimate ≥ 0` 判为 255，否则 0**（双极性解码）。正文「多数投票」「>0.5」与实现不完全一致，建议改为与代码一致的表述或注明伪代码抽象层级。
5. **嵌入规则**：`embedWatermarkToSigma` 为 **`sigma += alpha * w`，w ∈ {−1,+1}**，不是 `{0,1}` 直接乘 α。

### 新增图表

- `docs/diagrams/fig4_5_watermark_embed_flow.drawio`：嵌入主线（双极性比特、三通道重复、σ′=σ+α·w）。
- `docs/diagrams/fig4_6_watermark_extract_flow.drawio`：预处理双侧 → σ 差分 / α → 平均 → 零阈值 → reshape。

---

## 2026-04-30（§2.2.1 / 图 2-3 三层 DWT 结构）

- **新增** `docs/diagrams/fig2_3_dwt3level_structure.drawio`、`fig2_3_dwt3level_structure.mmd`：Mallat 塔式三层分解（每层 2×2 子带，仅 **LL** 继续下一层）；图注说明 **WatermarkCodec 仅一层 DWT**、Haar **[0.5,±0.5] 与实现 1/√2** 关系、正交与 **idwt2/dwt2** 可逆。
- **审查**：2.2.1 中 LH/HL 与水平、垂直边缘的对应与 `DWT_utils` 先列后行推导一致，无需对调；「三层分解图」为理论 MRA，与代码层数不一致处已在图注标明。

---

## 2026-04-30（定义 2-4 / 图 2-2 二维一层 DWT）

- **新增** `docs/diagrams/fig2_2_dwt2_onelevel.drawio`、`fig2_2_dwt2_onelevel.mmd`：二维一层 Haar 分解四子带（LL、LH、HL、HH）示意图；脚注说明可分离两步、与 `DWT_utils.cpp` 中「先列对、再行对」顺序及 **L² 归一化系数 1/√2**（与正文常见写法 **[½,½]、[½,−½]** 差常数因子，均为 Haar 正交族）的关系。

---

## 2026-04-30（§5.x 流程图；《系统设计.pdf》未入库）

### 关于《系统设计.pdf》

在仓库及常见上级目录中**未找到** `系统设计.pdf`（无 `*.pdf`）。图 5-7 / 5-9 / 5-11 已按 **`Preprocessor.cpp`、`Utils.cpp`** 与 `docs/ch6_module_flowcharts.mmd` 实现绘制；若与离线 PDF 字句不一致，以**代码与本次 draw.io** 为准或把 PDF 放入 `docs/` 后再对照修订。

### 新增图表文件

- `docs/diagrams/fig5_7_clahe_flow.drawio`、`fig5_7_clahe_flow.mmd`：CLAHE（灰度直通 / 彩色 LAB 仅增强 L）。
- `docs/diagrams/fig5_9_wiener_frequency_flow.drawio`、`fig5_9_wiener_frequency_flow.mmd`：分通道 padding、DFT、`G·H*/(|H|²+K)`、IDFT、循环至三通道后 merge（**已修正**初版中「单通道后即 merge」的流程错误）。
- `docs/diagrams/fig5_11_metrics_flow.drawio`、`fig5_11_metrics_flow.mmd`：PSNR（三通道 MSE + 255²）、SSIM（灰度 + 11×11 高斯）、准确率（二值逐像素）。

### 文档审查与修正

- **`experiments/README.md`**：程序表中原 `ch5_exp*.cpp` 与仓库中 **`ch6_exp*.cpp`** 不一致，已改为现用文件名；实验四/五对应 **`ch6_exp5_motion_wiener`**、**`ch6_exp6_motion_length_sweep`**；示例水印扩展名改为 **`.bmp`**；VS 说明中 `ch5_exp` 改为 `ch6_exp`。
- **`docs/ch6_module_flowcharts.mmd`**：单文件内多个顶层 `flowchart TD`，多数 Mermaid 预览器**只渲染第一段**；若需全文预览，请拆分为多个 `.mmd` 或使用 `---` 分页语法（依渲染器而定）。
- **`ColorWatermark.vcxproj`**：曾仅剩 `ch6_exp8` 为入口、缺少 **`src\main.cpp`**，与 README「默认 CLI」矛盾；已**恢复 `main.cpp` 参与生成**并将 `ch6_exp8` **ExcludedFromBuild**；资源 **`XM_32x32.bmp`**。

---

## 2026-04-30（§4.1 架构核对与图 4-1～4-3）

### 架构与表 4-1 对照代码的结论（无结构性错误，仅有表述补全建议）

1. **四层依赖**：`main` → `CLI::parse` → `WatermarkSystem::run` → `WatermarkCodec` / `Preprocessor`；`WatermarkCodec` 依赖 `DWT_utils`；`WatermarkSystem` 另包含 `Utils`（`imread`/`imwrite` 之外的 PSNR、SSIM、准确率、低光与运动模糊仿真）。正文若数据层仅写 OpenCV，建议在数据层或表 4-1 中顺带写明 **Utils**，与实现一致（图 4-1 draw.io 已在数据层框内合并说明）。
2. **模式与流程**：实现另有 **`compare_clahe`、`compare_wiener`**（对比图），不在「仅 embed/extract」的极简叙述中；若论文 4.1 不写，无逻辑错误，但与 CLI 全貌略有不齐。Mermaid 文件保留论文原文；draw.io 图 4-1 在「流程调度 / 模式选择」中写了「对比 / compare_*」作可选补充。
3. **DWT_utils 调用链**：业务层不直接 `#include` `DWT_utils`，仅 **WatermarkCodec** 调用；四层图中把 DWT_utils 放在算法层仍正确，表示「本层能力」而非「System 直接调用每个文件」。
4. **部署图 DLL 名**：与当前 OpenCV 4.8 构建一致时为 **`opencv_world480.dll`**（Debug 多为 `opencv_world480d.dll`）；若升级 OpenCV，部署图文件名需同步改。
5. **Preprocessor 命名**：代码中为 **`generateMotionPSF`**，非泛化 `generatePSF`；`fig4_1_architecture.mmd` 已用 `generateMotionPSF`。

### 本批新增/修改文件

- **新增** `docs/diagrams/fig4_1_layered_architecture.drawio`：图 4-1 四层架构（泳道 + 层间依赖箭头）。
- **新增** `docs/diagrams/fig4_1_architecture.mmd`：图 4-1 Mermaid 源码（与论文章节表述一致，便于 Typora/文档渲染）。
- **新增** `docs/diagrams/fig4_2_deployment.drawio`：图 4-2 单机部署（用户、命令行、exe、DLL、文件）。
- **新增** `docs/diagrams/fig4_3_module_tree.drawio`：图 4-3 功能模块关系（根分解、融合控制调度、算法模块至 I/O 与指标）。
- **修正** `fig4_3_module_tree.drawio` 末尾误复制的 XML 闭合标签。

---

## 2026-04-30

- **新增** `docs/diagrams/fig3_1_use_case_overall.drawio`：图 3-1 系统总体用例图（用户、系统边界、UC-01/02/05 及 UC-03/04 对 UC-02 的 `<<extend>>`）。
- **新增** `docs/diagrams/fig3_2_use_case_embed.drawio`：图 3-2 水印嵌入扩展用例图（UC-01 及 `<<include>>` 子用例，与表 3-1 一致）。
- **新增** `docs/diagrams/fig3_3_use_case_extract.drawio`：图 3-3 水印提取扩展用例图（UC-02 及 `<<include>>` 子用例，与表 3-3 一致）。
- **新增** 本文件 `docs/IMD.md`，作为后续改动的统一登记处。
- **修正** 三幅 `.drawio` 中边标签的 XML 转义，使 `<<extend>>` / `<<include>>` 在 diagrams.net 中正确显示。

---

（以下为历史占位，新记录请插入到「2026-04-30」之上。）
