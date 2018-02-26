## Handwriting Recognition via REST

This project contains a small server, which wraps the [WritePad-Handwriting-Recognition-Engine](https://github.com/phatware/WritePad-Handwriting-Recognition-Engine) (see [License](https://github.com/phatware/WritePad-Handwriting-Recognition-Engine/blob/master/LICENSE) and [Copyright](https://github.com/phatware/WritePad-Handwriting-Recognition-Engine/blob/master/COPYRIGHT)) to provide handwriting recognition via a simple REST-Interface.

## Setup

After checking out this repository, a few setup steps must be done:
1. Create an ```include``` folder in the [WritePadWrapper](./WritePadWrapper) directory
2. Add the [header files](https://github.com/phatware/WritePad-Handwriting-Recognition-Engine/tree/master/Linux/include) from the WritePad Handwriting Recognition Engine
3. Create a ```lib``` folder in the [WritePadWrapper](./WritePadWrapper) directory
4. Build the [WritePad Handwriting Recognition Engine](https://github.com/phatware/WritePad-Handwriting-Recognition-Engine) library and copy the generated ```libWritePadLib.a``` file into the created ```lib``` folder
5. Checkout the [jsoncpp](https://github.com/open-source-parsers/jsoncpp) repository and [generate](https://github.com/open-source-parsers/jsoncpp/wiki/Amalgamated) the header & source files. Add the content from the dist folder to the ```include``` folder
6. Create a build folder in the [WritePadWrapper](./WritePadWrapper) directory. Go into this folder and run ```cmake .. && make```

When running the python server, the path to the directory containing the created library above must be specified. This could be done as follows:
```LD_LIBRARY_PATH=/path/to/WritePadWrapper/build python3 main.py```

When the server should use [dictionaries](https://github.com/phatware/WritePad-Handwriting-Recognition-Engine/tree/master/Dictionaries), make sure they are in the same folder from which the python server is started. Otherwise the recognition will be done without the help of those dictionaries.

The final folder structure should contain at least the following files:
```
WritePadWrapper
--CMakeLists.txt
--build
----libWritePadWrapper.so
--include
----InkWrapper.h
----RecognizerApi.h
----RecognizerWrapper.h
----gestures.h
----json
------json-forwards.h
------json.h
----jsoncpp.cpp
----langid.h
----recodefs.h
----recotypes.h
--lib
----libWritePadLib.a
--main.cpp
server
--English.dct
--German.dct
--main.py
```
