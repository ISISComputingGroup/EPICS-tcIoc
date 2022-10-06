#####################################################################
# doxygen.ps1
# Author: Daniel Sigg/Maggie Tse
# Date: June 2019
#####################################################################
#
# Powershell script to run doxygen
#
# Requires doxygen
# wweb: http://doxygen.nl/
# Requires graphviz
# web: http://www.graphviz.org/
#
# Parent directory
$parent = "$PSScriptRoot"
$download = "$parent\Download"
$html = "html"
$latex = "latex"
# 
$version = "2_3"
#
# set path
$env:Path = "C:\Program Files\doxygen\bin;$env:Path"
$env:Path = "C:\Program Files (x86)\Graphviz2.38\bin;$env:Path"

cd $parent
del -r $html
doxygen $parent\Doxyfile
Compress-Archive -Path $html -DestinationPath $download\html_${version}.zip -Force
#
cp .\doxygen.sty $latex
cd $latex
./make
cd ..
copy $latex\refman.pdf $download\latex_documentation_${version}.pdf