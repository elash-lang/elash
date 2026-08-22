FROM alpine:latest AS builder

RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    libffi-dev \
    meson \
    ninja

RUN git clone --recursive https://github.com/Snaipe/Criterion.git /tmp/criterion \
    && cd /tmp/criterion \
    && meson setup build \
    && meson compile -C build \
    && meson install -C build


FROM alpine:latest

RUN apk add --no-cache \
    build-base \
    ccache \
    git \
    libffi \
    llvm \
    llvm-dev \
    nodejs \
    npm

COPY --from=builder /usr/local /usr/local

RUN mkdir -p /ccache

ENV CC="ccache cc"
ENV CCACHE_DIR=/ccache

