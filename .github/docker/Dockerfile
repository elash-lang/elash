FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
        base-devel \
        llvm \
        criterion \
        ccache \
        git && \
    pacman -Scc --noconfirm

RUN mkdir -p /ccache

ENV CC="ccache cc"
ENV CCACHE_DIR=/ccache
