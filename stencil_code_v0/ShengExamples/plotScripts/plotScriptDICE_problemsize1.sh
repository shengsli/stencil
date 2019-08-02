#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
python3 plot.py sum1DresultDICE_problemsize1.csv sum1DresultDICE_problemsize1.pdf
echo "Sum1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
python3 plot.py median1DresultDICE_problemsize1.csv median1DresultDICE_problemsize1.pdf
echo "Median1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
python3 plot.py sum2DresultDICE_problemsize1.csv sum2DresultDICE_problemsize1.pdf
echo "Sum2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
python3 plot.py median2DresultDICE_problemsize1.csv median2DresultDICE_problemsize1.pdf
echo "Median2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
python3 plot.py gaussianBlur2DresultDICE_problemsize1.csv gaussianBlur2DresultDICE_problemsize1.pdf
echo "GaussianBlur2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
python3 plot.py heatTransfer2DresultDICE_problemsize1.csv heatTransfer2DresultDICE_problemsize1.pdf
echo "HeatTransfer2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
python3 plot.py sum3DresultDICE_problemsize1.csv sum3DresultDICE_problemsize1.pdf
echo "Sum3D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
python3 plot.py median3DresultDICE_problemsize1.csv median3DresultDICE_problemsize1.pdf
echo "Median3D plot done"
