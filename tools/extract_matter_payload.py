#!/usr/bin/env python3
"""Extract the latest Matter onboarding payload (MT:...) from an ESP log file."""

import argparse
import re
import sys

PAYLOAD_RE = re.compile(r"MT:[A-Z0-9.-]+")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Extract Matter payload from monitor log")
    parser.add_argument("--log", required=True, help="Path to monitor log file")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        with open(args.log, "r", encoding="utf-8", errors="ignore") as fh:
            content = fh.read()
    except OSError as exc:
        print(f"ERROR: cannot read log: {exc}", file=sys.stderr)
        return 1

    matches = PAYLOAD_RE.findall(content)
    if not matches:
        print("ERROR: no MT payload found", file=sys.stderr)
        return 2

    print(matches[-1])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
