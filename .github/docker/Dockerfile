FROM alpine:latest

RUN apk add --no-cache \
    build-base \
    llvm \
    ccache \
    git \
    criterion

RUN mkdir -p /ccache

ENV CC="ccache cc"
ENV CCACHE_DIR=/ccache
