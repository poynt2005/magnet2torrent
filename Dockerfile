FROM debian:bookworm

RUN apt update && apt install -y git build-essential 

RUN apt install -y  libssl-dev zlib1g-dev libncurses5-dev libncursesw5-dev libreadline-dev libsqlite3-dev libgdbm-dev libdb5.3-dev libbz2-dev libexpat1-dev liblzma-dev libffi-dev

WORKDIR /tmp

RUN git clone https://github.com/python/cpython.git && \    
    cd cpython && \
    ./configure && \ 
    make -j$(nproc) && \
    make test -j$(nproc) && \
    make install 

ENV C_INCLUDE_PATH=$C_INCLUDE_PATH:/usr/local/include/python3.12

WORKDIR /tmp/build-libtorrent

RUN apt-get -y install libboost-tools-dev libboost-dev libboost-system-dev wget libssl-dev

RUN git clone --recurse-submodules https://github.com/arvidn/libtorrent.git

RUN cd libtorrent && mkdir install && b2 install --prefix=/tmp/build-libtorrent/libtorrent/install

WORKDIR /tmp

RUN cp -r /tmp/build-libtorrent/libtorrent/install /tmp/libtorrent

COPY libs/* /tmp

RUN make so &&  \
    ln -s libGetTorrent.so.1.0.0 libGetTorrent.so.1 &&  \
    ln -s libGetTorrent.so.1.0.0 libGetTorrent.so && \
    mkdir GetTorrent && \
    cp libGetTorrent.so.1.0.0 GetTorrent && \
    cp libGetTorrent.so.1 GetTorrent && \
    cp libGetTorrent.so GetTorrent

RUN make pyd

RUN make test-torrent

ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/GetTorrent:/tmp/libtorrent/lib

WORKDIR /app

RUN pip3 install flask gunicorn

RUN cp /tmp/PyGetTorrent.so /app/PyGetTorrent.so

COPY app/* /app

EXPOSE 8080

CMD ["gunicorn", "-b", "0.0.0.0:8080", "--timeout", "300", "app:app"]
