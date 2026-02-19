# Multi-platform xxhash-wrapper builder
# Builds on Alpine Linux for size; supports both x86-64 and aarch64

FROM alpine:latest

RUN apk add --no-cache \
    meson \
    ninja \
    gcc \
    musl-dev \
    pkgconfig

WORKDIR /workspace

# Copy everything
COPY . .

# Default: build for native platform
CMD ["sh", "-c", "meson setup build && meson compile -C build && meson test -C build --print-errorlogs"]
