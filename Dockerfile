FROM alpine AS cpp-builder
ENV BOOST_BUILD_PATH="/boost_1_83_0/tools/build"
ENV BOOST_ROOT="/boost_1_83_0"
RUN apk add openssl linux-headers openssl-dev build-base 
RUN echo "using gcc ;" > ~/user-config.jam 
RUN wget https://nchc.dl.sourceforge.net/project/boost/boost/1.83.0/boost_1_83_0.tar.gz -O boost.tar.gz \
    && tar -zxvf boost.tar.gz \
    && cd ./boost_1_83_0 \
    && ./bootstrap.sh --with-libraries=all --with-toolset=gcc \
    && ./b2 install --prefix=/usr/local
RUN wget https://github.com/arvidn/libtorrent/releases/download/v2.0.9/libtorrent-rasterbar-2.0.9.tar.gz -O libtorrent.tar.gz \
    && tar -zxvf libtorrent.tar.gz \
    && cd libtorrent-rasterbar-2.0.9 \
    && mkdir /lt \
    && /boost_1_83_0/b2 install --prefix=/lt
COPY ./libs /m2t-libs
WORKDIR /m2t-libs
RUN make -j$(nproc) \
    && ln -sf libMagnet2Torrent.so.1.0.0 libMagnet2Torrent.so \
    && make -j$(nproc) m2t \
    && chmod +x m2t
RUN mkdir ./bin \
    && cd ./bin \
    && cp /lt/lib/libboost_system.so.1.83.0 \
          /lt/lib/libtorrent-rasterbar.so.2.0.9 \
          /m2t-libs/libMagnet2Torrent.so.1.0.0 \
          /m2t-libs/m2t \
          ./ \
    && ln -sf libboost_system.so.1.83.0 libboost_system.so.1 \
    && ln -sf libboost_system.so.1.83.0 libboost_system.so \
    && ln -sf libMagnet2Torrent.so.1.0.0 libMagnet2Torrent.so.1 \
    && ln -sf libMagnet2Torrent.so.1.0.0 libMagnet2Torrent.so \
    && ln -sf libtorrent-rasterbar.so.2.0.9 libtorrent-rasterbar.so.2 \
    && ln -sf libtorrent-rasterbar.so.2.0.9 libtorrent-rasterbar.so

FROM golang:alpine AS go-builder
ENV CGO_ENABLED=1
RUN apk add build-base
WORKDIR /progress
WORKDIR /usr/local/go/src/
COPY --from=cpp-builder /m2t-libs/bin ./m2t-libs
COPY server ./server
COPY libs/*.h ./m2t-libs
RUN cd server \
    && go build -o server main.go \
    && chmod +x server \
    && mv server /progress/server

FROM alpine
RUN apk add --no-cache libstdc++ openssl
WORKDIR /progress
COPY --from=cpp-builder /m2t-libs/bin ./m2t-libs
WORKDIR /server
COPY --from=go-builder /progress/server ./server
ENV LD_LIBRARY_PATH=/progress/m2t-libs
CMD ["/server/server"]



