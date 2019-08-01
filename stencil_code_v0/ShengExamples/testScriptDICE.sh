#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DScript 4 100 5 sum1DresultDICE.csv 3 100000 0 20
python3 plot.py sum1DresultDICE.csv sum1DresultDICE.pdf
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DScript 4 100 5 median1DresultDICE.csv 3 100000 0 20
python3 plot.py median1DresultDICE.csv median1DresultDICE.pdf
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DScript 4 100 5 sum2DresultDICE.csv 3 100 100 0 20
python3 plot.py sum2DresultDICE.csv sum2DresultDICE.pdf
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DScript 4 100 5 median2DresultDICE.csv 3 100 100 0 20
python3 plot.py median2DresultDICE.csv median2DresultDICE.pdf
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DScript 4 100 5 gaussianBlur2DresultDICE.csv 2 100 100 0 20
python3 plot.py gaussianBlur2DresultDICE.csv gaussianBlur2DresultDICE.pdf
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DScript 4 100 5 heatTransfer2DresultDICE.csv 1 100 100 0 20
python3 plot.py heatTransfer2DresultDICE.csv heatTransfer2DresultDICE.pdf
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DScript 4 100 5 sum3DresultDICE.csv 1 40 40 40 0 20
python3 plot.py sum3DresultDICE.csv sum3DresultDICE.pdf
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DScript 4 100 5 median3DresultDICE.csv 1 40 40 40 0 20
python3 plot.py median3DresultDICE.csv median3DresultDICE.pdf
echo "Median3D test done"
