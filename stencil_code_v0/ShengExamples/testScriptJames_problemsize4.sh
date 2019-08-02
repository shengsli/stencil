#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DJamesScript 64 100 5 sum1DresultJames_problemsize3.csv 3 1000 0 20
python3 plot.py sum1DresultJames_problemsize3.csv sum1DresultJames_problemsize3.pdf
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DJamesScript 64 100 5 median1DresultJames_problemsize3.csv 3 1000 0 20
python3 plot.py median1DresultJames_problemsize3.csv median1DresultJames_problemsize3.pdf
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DJamesScript 64 100 5 sum2DresultJames_problemsize3.csv 3 30 30 0 20
python3 plot.py sum2DresultJames_problemsize3.csv sum2DresultJames_problemsize3.pdf
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DJamesScript 64 100 5 median2DresultJames_problemsize3.csv 3 30 30 0 20
python3 plot.py median2DresultJames_problemsize3.csv median2DresultJames_problemsize3.pdf
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DJamesScript 64 100 5 gaussianBlur2DresultJames_problemsize3.csv 2 30 30 0 20
python3 plot.py gaussianBlur2DresultJames_problemsize3.csv gaussianBlur2DresultJames_problemsize3.pdf
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DJamesScript 64 100 5 heatTransfer2DresultJames_problemsize3.csv 1 30 30 0 20
python3 plot.py heatTransfer2DresultJames_problemsize3.csv heatTransfer2DresultJames_problemsize3.pdf
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DJamesScript 64 100 5 sum3DresultJames_problemsize3.csv 1 10 10 10 0 20
python3 plot.py sum3DresultJames_problemsize3.csv sum3DresultJames_problemsize3.pdf
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DJamesScript 64 100 5 median3DresultJames_problemsize3.csv 1 10 10 10 0 20
python3 plot.py median3DresultJames_problemsize3.csv median3DresultJames_problemsize3.pdf
echo "Median3D test done"
