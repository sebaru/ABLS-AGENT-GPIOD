# abls-agent-gpiod

Standalone GPIOD runtime for Abls-Habitat.

## Current implementation status

- Runtime skeleton based on ABLS-AGENT-LIBS
- Facility fixed to `gpiod`
- Instance identified by `agent_tech_id`
- Linux GPIO access via `libgpiod`
- Local command handling for `SET_DO/<agent_tech_id>/<agent_acronyme>`
- Config bootstrap via Json_read_config using precedence:
  1. Environment variables (ABLS_*)
  2. /etc/abls/abls-agent.conf and per-agent config
  3. Defaults in code

Supported GPIOD features:

- discovery of available lines on `/dev/gpiochip0`
- API registration through `/run/gpiod/add/io`
- input polling with publication through `SET_DI/<agent_tech_id>/<agent_acronyme>`
- output control through local MQTT `SET_DO`

## Build

Requires libgpiod 2.x development headers (version 2.0 or newer).

```sh
./install_deps.sh
./build.sh
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces runtime RPM package in `build/`.

## Packaging DEB

```sh
./build_apt.sh --dist bookworm
./build_apt.sh --dist trixie
```

Default target suite is detected from host OS codename (`/etc/os-release`), with `bookworm` fallback.

Useful options:

- `--version-suffix <s>`: override Debian version suffix (example `~trixie`)
- `--no-dist-suffix`: disable automatic `~<suite>` suffix

Produces runtime DEB package and copies normalized artifacts to:

- `build/deb/<suite>/<arch>/`

`build_apt.sh` builds only the native host architecture.

Package signatures are centralized in ABLS-PKGS (both DEB repository metadata and RPM package/repository signatures).

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- copies RPM to `../ABLS-PKGS/public/rpms/<arch>/`
- copies DEB to `../ABLS-PKGS/deb-packages/<suite>/<arch>/`

## Container build

```sh
podman build -t abls-agent-gpiod:dev \
  --build-arg ABLS_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_LIBS_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_RPM_URL=<url> \
  -f Containerfile .
```
