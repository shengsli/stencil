#!/bin/sh
cd ~/stencil/stencil_code_v0/ShengExamples/Sum1D
python3 plot.py sum1DresultJames_problemsize2.csv sum1DresultJames_problemsize2.pdf
echo "Sum1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median1D
python3 plot.py median1DresultJames_problemsize2.csv median1DresultJames_problemsize2.pdf
echo "Median1D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum2D
python3 plot.py sum2DresultJames_problemsize2.csv sum2DresultJames_problemsize2.pdf
echo "Sum2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median2D
python3 plot.py median2DresultJames_problemsize2.csv median2DresultJames_problemsize2.pdf
echo "Median2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/GaussianBlur2D
python3 plot.py gaussianBlur2DresultJames_problemsize2.csv gaussianBlur2DresultJames_problemsize2.pdf
echo "GaussianBlur2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/HeatTransfer2D
python3 plot.py heatTransfer2DresultJames_problemsize2.csv heatTransfer2DresultJames_problemsize2.pdf
echo "HeatTransfer2D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Sum3D
python3 plot.py sum3DresultJames_problemsize2.csv sum3DresultJames_problemsize2.pdf
echo "Sum3D plot done"

cd ~/stencil/stencil_code_v0/ShengExamples/Median3D
python3 plot.py median3DresultJames_problemsize2.csv median3DresultJames_problemsize2.pdf
echo "Median3D plot done"
