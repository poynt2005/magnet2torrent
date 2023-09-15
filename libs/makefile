cc = gcc
cxx = g++
ldflags = -L/lt/lib
cflags = -I/lt/include
libs = -ltorrent-rasterbar -lboost_system -lssl -lcrypto -ldl  -ltry_signal


so:
	$(cxx) -fPIC -std=c++17 $(cflags) -DTORRENT_LINKING_SHARED -c Magnet2Torrent.cc -o Magnet2Torrent.o
	$(cxx) -shared $(ldflags) -pthread Magnet2Torrent.o -Wl,-soname,libMagnet2Torrent.so.1 -o libMagnet2Torrent.so.1.0.0 $(libs)

m2t:
	$(cc) -c M2T_test.c -o M2T_test.o
	$(cc) -L./ $(ldflags) M2T_test.o -o m2t -lMagnet2Torrent $(libs)


