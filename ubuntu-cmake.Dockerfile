FROM ubuntu:latest

ENV VENV_DIR=/venv

RUN apt-get update && apt-get install -y --no-install-recommends \
    curl ca-certificates cmake make libfl2 wget bzip2

ADD https://astral.sh/uv/install.sh /uv-installer.sh
RUN UV_INSTALL_DIR="/usr/local/bin" sh /uv-installer.sh && rm /uv-installer.sh

RUN uv python install 3.12 && uv venv -p 3.12 ${VENV_DIR} && uv --directory ${VENV_DIR} pip install xmltodict

CMD ["bash"]
