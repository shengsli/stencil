#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
python3 plot_exp2.py sum1DresultJames_exp2.csv
echo "Sum1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
python3 plot_exp2.py median1DresultJames_exp2.csv
echo "Median1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
python3 plot_exp2.py sum2DresultJames_exp2.csv
echo "Sum2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
python3 plot_exp2.py median2DresultJames_exp2.csv
echo "Median2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
python3 plot_exp2.py gaussianBlur2DresultJames_exp2.csv
echo "GaussianBlur2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
python3 plot_exp2.py heatTransfer2DresultJames_exp2.csv
echo "HeatTransfer2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
python3 plot_exp2.py sum3DresultJames_exp2.csv
echo "Sum3D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
python3 plot_exp2.py median3DresultJames_exp2.csv
echo "Median3D plot done"
