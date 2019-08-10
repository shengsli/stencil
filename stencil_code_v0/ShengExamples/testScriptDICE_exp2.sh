#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DScript_exp2 4 100 5 sum1DresultDICE_exp2.csv 3 1000000 0 20
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DScript_exp2 4 100 5 median1DresultDICE_exp2.csv 3 1000000 0 20
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DScript_exp2 4 100 5 sum2DresultDICE_exp2.csv 3 100 100 0 20
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DScript_exp2 4 100 5 median2DresultDICE_exp2.csv 3 100 100 0 20
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DScript_exp2 4 100 5 gaussianBlur2DresultDICE_exp2.csv 2 100 100 0 20
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DScript_exp2 4 100 5 heatTransfer2DresultDICE_exp2.csv 1 100 100 0 20
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DScript_exp2 4 100 5 sum3DresultDICE_exp2.csv 1 10 10 10 0 20
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DScript_exp2 4 100 5 median3DresultDICE_exp2.csv 1 10 10 10 0 20
echo "Median3D test done"
