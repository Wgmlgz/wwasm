build: src/wwasm/cpp/wwasm.cpp
	em++ --no-entry src/wwasm/cpp/wwasm.cpp -o src/wwasm/react/wwasm.mjs  \
	  -s ENVIRONMENT='web'  \
	  -s SINGLE_FILE=1  \
	  -s EXPORT_NAME='WWasmModule'  \
	  -s USE_ES6_IMPORT_META=0  \
	  -s TOTAL_MEMORY=500MB \
	  -s EXPORTED_FUNCTIONS='["_malloc", "_free"]'  \
	  -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'  \
	  -O3