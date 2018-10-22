ARG CUDA_VERSION

FROM nvidia/cuda:${CUDA_VERSION}-cudnn7-devel-ubuntu18.04

RUN apt-get update && apt-get install -y build-essential git cmake python3-pip libmpfr-dev libgmp-dev wget

# RUN apt-get install -y libblas-dev liblapack-dev

RUN mkdir -p /root/magma/built

WORKDIR /root/magma

COPY . .

RUN cd built && cmake -DGPU_TARGET="Kepler, Maxwell, Pascal, Volta" -DCUDA_cublas_device_LIBRARY="" -DUSE_FORTRAN=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=`pwd`/output ../

RUN cd built && make

RUN cd built && make install