FROM ubuntu:latest

RUN apt update && apt install -y cmake make libfl2 wget bzip2 git

WORKDIR /project
CMD ["bash"]
