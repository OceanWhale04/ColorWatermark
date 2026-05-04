# Demos

This directory contains standalone demo programs for paper figures.
They do not change the main business flow in `src/`.

## Directory layout

```text
ColorWatermark/
├── include/
├── src/
├── demos/
│   ├── legacy/
│   │   ├── main_clahe_demo.cpp
│   │   └── main_wiener_demo.cpp
│   └── paper_figures/
│   ├──output/
│   ├── fig4_1/
│   ├── fig4_2/
│   ├── fig4_3/
│   └── fig4_4/
│       ├── demo_fig4_1_embed_visual.cpp
│       ├── demo_fig4_2_extract_clean.cpp
│       ├── demo_fig4_3_lowlight_clahe_ablation.cpp
│       └── demo_fig4_4_motion_wiener_ablation.cpp
├── test_images/
   ├── color/
   ├── gray/
   └── XM_32x32.bmp
 
```

## Suggested assets for paper figures

- Cover image (Lena-like 512x512): `test_images/color/512/4.2.07.tiff`
- Watermark image (binary 32x32): `test_images/XM_32x32.bmp`

You can replace them with your own paths through command line args in each demo.

## Outputs

Each demo writes outputs under `demos/paper_figures/output/fig4_x/` and prints key metrics to console
(PSNR, SSIM, extraction accuracy).
