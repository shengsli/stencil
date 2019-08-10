#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DJamesScript_exp2 4 100 5 sum1DresultJames_exp2.csv 3 64 0 20
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DJamesScript_exp2 4 100 5 median1DresultJames_exp2.csv 3 64 0 20
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DJamesScript_exp2 4 100 5 sum2DresultJames_exp2.csv 3 8 8 0 20
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DJamesScript_exp2 4 100 5 median2DresultJames_exp2.csv 3 8 8 0 20
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DJamesScript_exp2 4 100 5 gaussianBlur2DresultJames_exp2.csv 2 8 8 0 20
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DJamesScript_exp2 4 100 5 heatTransfer2DresultJames_exp2.csv 1 8 8 0 20
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DJamesScript_exp2 4 100 5 sum3DresultJames_exp2.csv 1 4 4 4 0 20
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DJamesScript_exp2 4 100 5 median3DresultJames_exp2.csv 1 4 4 4 0 20
echo "Median3D test done"
