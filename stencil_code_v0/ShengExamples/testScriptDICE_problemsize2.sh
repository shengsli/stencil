#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DScript 4 100 5 sum1DresultDICE_problemsize2.csv 3 100000 0 20
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DScript 4 100 5 median1DresultDICE_problemsize2.csv 3 100000 0 20
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DScript 4 100 5 sum2DresultDICE_problemsize2.csv 3 100 100 0 20
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DScript 4 100 5 median2DresultDICE_problemsize2.csv 3 100 100 0 20
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DScript 4 100 5 gaussianBlur2DresultDICE_problemsize2.csv 2 100 100 0 20
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DScript 4 100 5 heatTransfer2DresultDICE_problemsize2.csv 1 100 100 0 20
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DScript 4 100 5 sum3DresultDICE_problemsize2.csv 1 40 40 40 0 20
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DScript 4 100 5 median3DresultDICE_problemsize2.csv 1 40 40 40 0 20
echo "Median3D test done"
