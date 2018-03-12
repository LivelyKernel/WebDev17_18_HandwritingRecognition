#!/bin/bash

WritePadWrapperDir=WritePadWrapper
# #TODO check this out as sub repository... not as sibling repo
WritePadEngineDir=WritePad-Handwriting-Recognition-Engine
JSONCPP=jsoncpp

mkdir $WritePadWrapperDir/include
mkdir $WritePadWrapperDir/lib
mkdir $WritePadWrapperDir/build

if [ ! -e $WritePadEngineDir ]; then
    git clone https://github.com/phatware/WritePad-Handwriting-Recognition-Engine.git
    pushd $WritePadEngineDir
    git apply ../WritePad-Handwriting-Recognition-Engine.patch
    popd
fi
cp $WritePadEngineDir/Linux/include/* $WritePadWrapperDir/include

pushd $WritePadEngineDir/Linux
make
popd
cp $WritePadEngineDir/Linux/Lib/libWritePadLib.a $WritePadWrapperDir/lib

if [ ! -e $JSONCPP ]; then
    echo "checking out jsoncpp"
    git clone https://github.com/open-source-parsers/jsoncpp.git
else
    echo "found jsoncpp"
fi

pushd $JSONCPP
python amalgamate.py
popd
cp -r $JSONCPP/dist/* $WritePadWrapperDir/include

if [ ! -e $WritePadWrapperDir/build/libWritePadWrapper.so ]; then
pushd $WritePadWrapperDir/build
cmake ..
make
popd
fi

for ea in "German.dct" "English.dct"; do
    if [ ! -e server/$ea ]; then
	echo "download "$ea
	curl https://github.com/phatware/WritePad-Handwriting-Recognition-Engine/blob/master/Dictionaries/$ea"?raw=true" > server/$ea 
    fi
done

   
