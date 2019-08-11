#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
python3 plot_exp2.py sum1DresultDICE_exp2.csv
echo "Sum1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
python3 plot_exp2.py median1DresultDICE_exp2.csv
echo "Median1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
python3 plot_exp2.py sum2DresultDICE_exp2.csv
echo "Sum2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
python3 plot_exp2.py median2DresultDICE_exp2.csv
echo "Median2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
python3 plot_exp2.py gaussianBlur2DresultDICE_exp2.csv
echo "GaussianBlur2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
python3 plot_exp2.py heatTransfer2DresultDICE_exp2.csv
echo "HeatTransfer2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
python3 plot_exp2.py sum3DresultDICE_exp2.csv
echo "Sum3D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
python3 plot_exp2.py median3DresultDICE_exp2.csv
echo "Median3D plot done"
