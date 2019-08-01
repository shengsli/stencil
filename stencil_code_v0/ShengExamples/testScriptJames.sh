#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
./sum1DJamesScript 64 100 5 sum1DresultJames.csv 3 100000 0 20
python3 plot.py sum1DresultJames.csv sum1DresultJames.pdf
echo "Sum1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
./median1DJamesScript 64 100 5 median1DresultJames.csv 3 100000 0 20
python3 plot.py median1DresultJames.csv median1DresultJames.pdf
echo "Median1D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
./sum2DJamesScript 64 100 5 sum2DresultJames.csv 3 100 100 0 20
python3 plot.py sum2DresultJames.csv sum2DresultJames.pdf
echo "Sum2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
./median2DJamesScript 64 100 5 median2DresultJames.csv 3 100 100 0 20
python3 plot.py median2DresultJames.csv median2DresultJames.pdf
echo "Median2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
./gaussianBlur2DJamesScript 64 100 5 gaussianBlur2DresultJames.csv 3 100 100 0 20
python3 plot.py gaussianBlur2DresultJames.csv gaussianBlur2DresultJames.pdf
echo "GaussianBlur2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
./heatTransfer2DJamesScript 64 100 5 heatTransfer2DresultJames.csv 3 100 100 0 20
python3 plot.py heatTransfer2DresultJames.csv heatTransfer2DresultJames.pdf
echo "HeatTransfer2D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
./sum3DJamesScript 64 100 5 sum3DresultJames.csv 1 40 40 40 0 20
python3 plot.py sum3DresultJames.csv sum3DresultJames.pdf
echo "Sum3D test done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
./median3DJamesScript 64 100 5 median3DresultJames.csv 1 40 40 40 0 20
python3 plot.py median3DresultJames.csv median3DresultJames.pdf
echo "Median3D test done"
