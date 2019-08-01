#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DScript 4 100 5 sum1DresultDICE_problemsize1.csv 3 1000000 0 20
python3 plot.py sum1DresultDICE_problemsize1.csv sum1DresultDICE_problemsize1.pdf
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DScript 4 100 5 median1DresultDICE_problemsize1.csv 3 1000000 0 20
python3 plot.py median1DresultDICE_problemsize1.csv median1DresultDICE_problemsize1.pdf
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DScript 4 100 5 sum2DresultDICE_problemsize1.csv 3 1000 1000 0 20
python3 plot.py sum2DresultDICE_problemsize1.csv sum2DresultDICE_problemsize1.pdf
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DScript 4 100 5 median2DresultDICE_problemsize1.csv 3 1000 1000 0 20
python3 plot.py median2DresultDICE_problemsize1.csv median2DresultDICE_problemsize1.pdf
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DScript 4 100 5 gaussianBlur2DresultDICE_problemsize1.csv 2 1000 1000 0 20
python3 plot.py gaussianBlur2DresultDICE_problemsize1.csv gaussianBlur2DresultDICE_problemsize1.pdf
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DScript 4 100 5 heatTransfer2DresultDICE_problemsize1.csv 1 1000 1000 0 20
python3 plot.py heatTransfer2DresultDICE_problemsize1.csv heatTransfer2DresultDICE_problemsize1.pdf
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DScript 4 100 5 sum3DresultDICE_problemsize1.csv 1 100 100 100 0 20
python3 plot.py sum3DresultDICE_problemsize1.csv sum3DresultDICE_problemsize1.pdf
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DScript 4 100 5 median3DresultDICE_problemsize1.csv 1 100 100 100 0 20
python3 plot.py median3DresultDICE_problemsize1.csv median3DresultDICE_problemsize1.pdf
echo "Median3D test done"
