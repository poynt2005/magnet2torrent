cc = gcc
cxx = g++
ldflags = -L./libtorrent/lib 
cflags = -I./libtorrent/include -I/usr/local/include/python3.12
libs = -ltorrent-rasterbar -lboost_system -lssl -lcrypto -ldl  -ltry_signal
pylibs =  -lpython312 -ldl -lm


so:
	$(cxx) -fPIC -shared -std=c++17 $(cflags) -DTORRENT_LINKING_SHARED -c GetTorrent.cc -o GetTorrent.o
	$(cxx) -shared $(ldflags) -pthread GetTorrent.o -Wl,-soname,libGetTorrent.so.1 -o libGetTorrent.so.1.0.0 $(libs)

pyd:
	$(cc) -fPIC -shared $(cflags) -c pyBinding.c -o pyBinding.o
	$(cc) -shared -L./ /usr/local/lib/libpython3.12.a pyBinding.o -o PyGetTorrent.so -lGetTorrent
	

test-torrent:
	$(cxx) -c test-torrent.cc -o test-torrent.o
	$(cxx) -L./ $(ldflags) test-torrent.o -o test-torrent -lGetTorrent $(libs)





