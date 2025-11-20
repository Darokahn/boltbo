all:
	emcc *.c -s ASYNCIFY=1 -s EXPORTED_RUNTIME_METHODS="['HEAPU8']" -o test.html
	python3 -m http.server 5000
