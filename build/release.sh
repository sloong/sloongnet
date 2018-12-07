rm -rdf release
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ../../sloongnet
make