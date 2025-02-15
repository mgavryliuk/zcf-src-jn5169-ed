FROM ubuntu:latest

RUN apt update && apt install -y cmake make libfl2

WORKDIR /project
CMD ["bash"]
