"""
PlatformIO post-build helper:
- prints the final firmware.bin size against the OTA app partition limit
- keeps the build output limited to firmware.bin
"""

from __future__ import annotations

import csv
import glob
import os


def load_app_partition_size(project_dir: str) -> int | None:
    partitions_path = os.path.join(project_dir, "partitions.csv")
    if not os.path.isfile(partitions_path):
        return None

    with open(partitions_path, "r", encoding="utf-8", newline="") as handle:
        reader = csv.reader(handle)
        for row in reader:
            if not row or row[0].startswith("#"):
                continue
            if row[0].strip() == "app0":
                raw_size = row[4].strip()
                return int(raw_size, 0)
    return None


def after_build(target, source, env):
    bin_path = str(target[0])
    if not os.path.isfile(bin_path):
        print(f"Post-build: firmware bin not found at {bin_path}")
        return

    project_dir = env["PROJECT_DIR"]
    pio_env = env["PIOENV"]
    bin_size = os.path.getsize(bin_path)
    app_limit = load_app_partition_size(project_dir)

    if app_limit:
        margin = app_limit - bin_size
        usage = (bin_size / app_limit) * 100
        print(
            f"Post-build: firmware.bin = {bin_size} bytes | "
            f"app partition = {app_limit} bytes | "
            f"usage = {usage:.1f}% | margin = {margin} bytes"
        )

    output_dir = os.path.dirname(bin_path)
    if pio_env != "vcodex_release":
        return

    stale_pattern = os.path.join(output_dir, "firmware.*-vcodex.bin")
    stale_bins = glob.glob(stale_pattern)
    for stale_bin in stale_bins:
        try:
            os.remove(stale_bin)
            print(f"Post-build: removed stale {os.path.basename(stale_bin)}")
        except OSError as exc:
            print(f"Post-build: failed to remove stale {os.path.basename(stale_bin)}: {exc}")


Import("env")
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)
