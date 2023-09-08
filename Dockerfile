FROM ubuntu:22.04
RUN apt update && apt upgrade
RUN apt install -y gcc make git binutils libc6-dev
WORKDIR /home
