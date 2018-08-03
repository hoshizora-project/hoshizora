FROM quay.io/pypa/manylinux1_x86_64

LABEL maintainer='amaya <mail@sapphire.in.net>'

RUN yum install -y make boost148-devel boost148-filesystem-devel boost148-system-devel numactl-devel


RUN wget https://cmake.org/files/v3.11/cmake-3.11.4.tar.gz -O - | tar xz
RUN cd cmake-3.11.4 && ./configure --system-curl && make -j2 && make install

# build mecab

# build clang


WORKDIR /opt
RUN apt update && \
    apt install -y make cmake clang libboost-all-dev libnuma-dev python3
RUN git clone https://github.com/hoshizora-project/hoshizora.git
WORKDIR /opt/hoshizora
RUN python3 setup.py bdist_wheel

