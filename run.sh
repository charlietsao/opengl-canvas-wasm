#emcc main.cpp -std=c++17 -s WASM=1 -s USE_SDL=2 -O3 -o index.js
emcc main.cpp -std=c++17 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s WASM=1 -s USE_SDL=2 -O3 -o index.js
python -m http.server 8080

