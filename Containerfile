FROM fedora:latest AS build
LABEL maintainer="sebastien.lefevre@abls-habitat.fr"

RUN dnf upgrade -y && dnf install -y \
    cmake gcc pkg-config glib2-devel json-glib-devel libgpiod-devel wget && \
    dnf clean all

# These URLs are intended to point to GitHub Releases artifacts.
ARG ABLS_LIBS_DEVEL_RPM_URL=""
ARG ABLS_AGENT_LIBS_DEVEL_RPM_URL=""

RUN if [ -n "${ABLS_LIBS_DEVEL_RPM_URL}" ]; then dnf install -y "${ABLS_LIBS_DEVEL_RPM_URL}"; fi
RUN if [ -n "${ABLS_AGENT_LIBS_DEVEL_RPM_URL}" ]; then dnf install -y "${ABLS_AGENT_LIBS_DEVEL_RPM_URL}"; fi

WORKDIR /app
COPY . /app
RUN ./build.sh

FROM fedora:latest AS runtime
RUN dnf upgrade -y && dnf install -y \
    glib2 json-glib libgpiod && \
    dnf clean all

ARG ABLS_LIBS_RPM_URL=""
ARG ABLS_AGENT_LIBS_RPM_URL=""

RUN if [ -n "${ABLS_LIBS_RPM_URL}" ]; then dnf install -y "${ABLS_LIBS_RPM_URL}"; fi
RUN if [ -n "${ABLS_AGENT_LIBS_RPM_URL}" ]; then dnf install -y "${ABLS_AGENT_LIBS_RPM_URL}"; fi

COPY --from=build /app/build/abls-agent-gpiod /usr/local/bin/abls-agent-gpiod

ENV ABLS_IN_A_CONTAINER=1
CMD ["/usr/local/bin/abls-agent-gpiod"]
