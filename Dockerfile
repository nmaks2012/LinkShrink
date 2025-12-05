FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg:latest AS builder

ARG USER_ID=1000
ARG GROUP_ID=1000
RUN groupadd -g ${GROUP_ID} user && \
    useradd -u ${USER_ID} -g ${GROUP_ID} -m -d /home/user -s /bin/bash user

WORKDIR /build

COPY . .

RUN cmake -B build_release -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build_release -- -j$(nproc)

FROM ubuntu:22.04

RUN groupadd -r user && \
    useradd -r -g user -m -d /home/user -s /bin/bash user

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    tzdata \
    libssl3 \
    zlib1g \
    libpq5 \
    libcurl4 \
    libc-ares2 \
    libev4 \
    libjemalloc2 \
    libfmt8 \
    libyaml-cpp0.7 \
    libre2-9 \
    libcctz2 \
    libcrypto++8 \
    libboost-filesystem1.74.0 \
    libboost-iostreams1.74.0 \
    libboost-program-options1.74.0 \
    libboost-regex1.74.0 \
    libboost-stacktrace1.74.0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /linkshrink-app

COPY --from=builder /build/build_release/linkshrink-app /linkshrink-app/service_bin
COPY --from=builder /build/configs /linkshrink-app/configs
COPY --from=builder /build/static /linkshrink-app/static

RUN chown -R user:user /linkshrink-app

USER user

CMD ["/linkshrink-app/service_bin", "--config", "/linkshrink-app/configs/static_config.yaml", "--config_vars", "/linkshrink-app/configs/config_vars.yaml"]