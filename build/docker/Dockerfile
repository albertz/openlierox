# OpenLieroX dedicated server — Debian 11 "bullseye"
#
# Why bullseye: the dedicated-server automation (share/gamedir/scripts/
# dedicated_control*) is written in Python 2 and runs via a "#!/usr/bin/python"
# shebang. Debian 11 is the last Debian release that still ships python2.7
# (2.7.18) in its main repository; Debian 12 "bookworm" and Ubuntu 24.04 removed
# it. bullseye also predates the t64 library-rename transition, so the runtime
# package names below are the stable, classic ones.

# ---------- build stage ----------
FROM debian:bullseye-slim AS build

ENV DEBIAN_FRONTEND=noninteractive
# bullseye is EOL: deb.debian.org no longer reliably serves it (some packages
# already 404 mid-archival). Pin to the snapshot.debian.org mirror line the
# base image itself ships commented out in /etc/apt/sources.list, which
# preserves the exact package set from that date. Snapshot Release files carry
# a past Valid-Until, hence Check-Valid-Until=false. snapshot.debian.org is
# also prone to transient 5xx errors under load, hence the retries.
RUN sed -i \
        -e '/^deb http:\/\/deb\.debian\.org/s/^/# /' \
        -e '/^# deb http:\/\/snapshot\.debian\.org/s/^# //' \
        /etc/apt/sources.list \
    && printf 'Acquire::Check-Valid-Until "false";\nAcquire::Retries "5";\n' > /etc/apt/apt.conf.d/99snapshot
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-mixer-dev \
        libxml2-dev \
        libgd-dev \
        zlib1g-dev \
        libzip-dev \
        libx11-dev \
        libcurl4-openssl-dev \
        libboost-dev \
        libopenal-dev \
        libalut-dev \
        libvorbis-dev \
        libyaml-cpp-dev \
        binutils-dev \
        libiberty-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# DEDICATED_ONLY=Yes      -> headless build, no GUI/G15 code paths.
# HAWKNL_BUILTIN=Yes      -> use the bundled HawkNL networking lib.
# OLX_DOWNLOAD_...=OFF     -> use the committed gamecontrollerdb.txt instead of
#                            fetching it at configure time (no network needed,
#                            and irrelevant for a dedicated server anyway).
# The in-binary version is derived from the git history (CMakeOlxVersion.cmake
# -> get_version_full.sh), which is why git and the copied .git dir are needed.
# The make step leaves one core free during the build (floors at 1).
RUN cmake -DDEDICATED_ONLY=Yes -DHAWKNL_BUILTIN=Yes -DOLX_DOWNLOAD_GAMECONTROLLERDB=OFF -DDEBUG=No . \
    && n="$(nproc)" \
    && { [ "$n" -gt 1 ] && n="$((n - 1))" || true; } \
    && make -j"$n"

# ---------- runtime stage ----------
FROM debian:bullseye-slim

ENV DEBIAN_FRONTEND=noninteractive
# See the build stage above for why this pins to the snapshot mirror.
RUN sed -i \
        -e '/^deb http:\/\/deb\.debian\.org/s/^/# /' \
        -e '/^# deb http:\/\/snapshot\.debian\.org/s/^# //' \
        /etc/apt/sources.list \
    && printf 'Acquire::Check-Valid-Until "false";\nAcquire::Retries "5";\n' > /etc/apt/apt.conf.d/99snapshot
RUN apt-get update && apt-get install -y --no-install-recommends \
        python2.7 \
        libbinutils \
        libsdl2-2.0-0 \
        libsdl2-image-2.0-0 \
        libsdl2-mixer-2.0-0 \
        libxml2 \
        libgd3 \
        zlib1g \
        libzip4 \
        libcurl4 \
        libopenal1 \
        libalut0 \
        libvorbisfile3 \
        libyaml-cpp0.6 \
    && ln -sf /usr/bin/python2.7 /usr/bin/python \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/bin/openlierox /usr/local/bin/openlierox
COPY --from=build /src/share/gamedir /opt/openlierox
COPY build/docker/entrypoint.sh /usr/local/bin/entrypoint.sh

# Run as a non-root user. ~/.OpenLieroX (under HOME) is where OLX writes its
# own configs, logs and rankings — mount a volume there to persist them.
RUN chmod +x /usr/local/bin/entrypoint.sh \
    && useradd -r -m -d /home/openlierox openlierox \
    && chown -R openlierox:openlierox /opt/openlierox

USER openlierox
ENV HOME=/home/openlierox
WORKDIR /opt/openlierox

# OpenLieroX dedicated servers listen on UDP 23400 by default.
EXPOSE 23400/udp

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
