FROM ubuntu:24.04 AS vislam-builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libopencv-dev \
    libspdlog-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
COPY gpu ./gpu

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVISLAM_BUILD_TESTS=OFF -DVISLAM_ENABLE_CUDA=OFF \
    && cmake --build build --target vislam_app -j"$(nproc)"

FROM ubuntu:24.04 AS api-runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
    libopencv-dev \
    libspdlog-dev \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace/services/api

COPY services/api/package*.json ./
RUN npm ci --omit=dev

COPY services/api ./

RUN mkdir -p /workspace/bin /workspace/artifacts/runs
COPY --from=vislam-builder /workspace/build/vislam_app /workspace/bin/vislam_app

ENV PORT=8080
ENV VISLAM_EXECUTABLE=/workspace/bin/vislam_app
ENV ARTIFACTS_DIR=/workspace/artifacts/runs

EXPOSE 8080

CMD ["node", "src/server.js"]

