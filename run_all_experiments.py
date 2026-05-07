#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
一键汇总实验 CSV（works.md 表 6-6～6-13）。
说明：VS 工程默认只生成 ColorWatermark.exe；各 ch6 实验与 tests 在 vcxproj 中
标记为 ExcludedFromBuild。默认生成的是带 --mode 的 CLI（main.cpp）。
跑单元/集成/某一章实验时：在 VS 中排除 main.cpp、取消排除对应 tests 或 ch6_exp*.cpp 后单独编译。
本脚本在 ColorWatermark 工程目录运行；默认读取 experiments/output 下的 CSV。

用法（在 ColorWatermark 工程目录）:
  python run_all_experiments.py
可选:
  python run_all_experiments.py --input-dir experiments/output --out output/output_tables_merged.csv
"""

from __future__ import annotations

import argparse
import csv
import glob
import os
import sys
from pathlib import Path


def merge_csv_files(input_dir: Path, out_path: Path) -> None:
    rows_out: list[list[str]] = []
    rows_out.append(["source_file", "row_index", "merged_cells..."])

    pattern = str(input_dir / "*.csv")
    files = sorted(glob.glob(pattern))
    if not files:
        print(f"No CSV under {input_dir}", file=sys.stderr)
        return

    for fp in files:
        with open(fp, "r", encoding="utf-8", newline="") as f:
            reader = csv.reader(f)
            for idx, row in enumerate(reader):
                rows_out.append([os.path.basename(fp), str(idx)] + row)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerows(rows_out)
    print(f"Wrote {out_path} ({len(rows_out)} lines)")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--input-dir", default="experiments/output", help="CSV 目录")
    ap.add_argument("--out", default="output/output_tables_merged.csv", help="合并输出")
    args = ap.parse_args()

    root = Path(__file__).resolve().parent
    input_dir = (root / args.input_dir).resolve() if not os.path.isabs(args.input_dir) else Path(args.input_dir)
    out_path = (root / args.out).resolve() if not os.path.isabs(args.out) else Path(args.out)

    merge_csv_files(input_dir, out_path)
    print(
        "\n提示：若需自动依次调用各 .exe，请为每个实验单独配置生成目标后，"
        "在此脚本中用 subprocess 调用对应路径。"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
