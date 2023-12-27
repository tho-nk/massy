export project_name=massy
export THIRD_PARTY=$HOME/third-party
export Boost_Version='1.81.0'
export GoogleTest_Version='1.13.0'
export Rapidjson_Version='1.1.0'

cmake -B build -DCMAKE_BUILD_TYPE=Debug
make -C build -j${nproc}