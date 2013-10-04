echo "Script for building:"
echo " - P7 library"
echo " - Example"
echo " - Speed test"
echo " - Tracing tests"
echo "------------------------------------------------------------"

if [ $# -lt 2 ] ; then
   echo "run [./build.sh clean] to make cleanup"
fi

echo "P7----------------------------------------------------------"
cd ./Sources/
make $1
cd ..

cd ./Tests/

echo "Example-----------------------------------------------------"
cd ./Example/
make $1
cd ..

echo "Speed-------------------------------------------------------"
cd ./Speed/
make $1
cd ..

echo "Trace-------------------------------------------------------"
cd ./Trace/
make $1
cd ..

cd ..
