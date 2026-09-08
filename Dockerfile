# syntax=docker/dockerfile:1@sha256:ecfaec9ed6d810b56388c508f4121597bfbba70d41a6dfeee4d8cad5f295fc32

FROM python:3.12-slim-bookworm@sha256:782412e85d0f0984994c290652577d4018aff08145c85b262bb63dc0c7522254

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
        build-essential \
        ca-certificates \
        diffutils \
    && rm -rf /var/lib/apt/lists/*

ENV MPLBACKEND=Agg \
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1

WORKDIR /artifact

# Install the Python layer separately so source-only changes reuse this layer.
COPY ORAMEmulator/requirements.txt /tmp/oram-requirements.txt
RUN python -m pip install --no-cache-dir --requirement /tmp/oram-requirements.txt

COPY . /artifact

RUN chmod +x \
        /artifact/runme.sh \
        /artifact/BoundFinder/runme.sh \
        /artifact/ORAMEmulator/runme.sh

ENTRYPOINT ["/artifact/runme.sh"]
CMD ["quick"]
