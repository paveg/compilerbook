FROM ubuntu:latest

RUN apt-get update && apt-get install -y gcc make git binutils libc6-dev

WORKDIR /root
COPY . .
WORKDIR /root/9cc

CMD ["/bin/bash"]
