# Building Retro Chiaki

The current release is cross-compiled for ARM64 with Ubuntu 22.04 in Docker.

```bash
docker build -t retro-chiaki-arm64 packaging/build
mkdir -p dist-build
docker run --rm \
  -v "$PWD:/chiaki-local:ro" \
  -v "$PWD/dist-build:/output" \
  retro-chiaki-arm64 bash /chiaki-local/packaging/build/build.sh
```

The GUI and CLI binaries are written to `dist-build/`. The PortMaster release additionally bundles the Qt runtime, XKB data, launcher, gptokeyb mapping and the Mali EGL shim. `packaging/build/mali_egl_shim.c` is the corresponding source for that shim.

The upstream submodules are fetched by the build script. For reproducible public releases, record the source commit and release checksum in the GitHub release notes.
