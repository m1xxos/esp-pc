#!/usr/bin/env python3
"""Generate PNG/SVG QR for a Matter onboarding payload (MT:...)."""

import argparse
import pathlib
import sys


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate Matter QR image from onboarding payload")
    parser.add_argument("--payload", required=True, help="Onboarding payload, example: MT:XXXX")
    parser.add_argument("--png", default="matter_qr.png", help="Output PNG path")
    parser.add_argument("--svg", default="", help="Optional SVG output path")
    parser.add_argument("--payload-out", default="matter_payload.txt", help="Save payload to text file")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    payload = args.payload.strip()
    if not payload.startswith("MT:"):
        print("ERROR: payload must start with MT:", file=sys.stderr)
        return 1

    try:
        import qrcode
        import qrcode.image.svg
    except ImportError as exc:
        print("ERROR: missing dependency qrcode", file=sys.stderr)
        print("Install with: ./.venv/bin/pip install -r tools/requirements.txt", file=sys.stderr)
        print(f"Details: {exc}", file=sys.stderr)
        return 2

    png_path = pathlib.Path(args.png)
    png_path.parent.mkdir(parents=True, exist_ok=True)

    png_img = qrcode.make(payload)
    png_img.save(png_path)

    if args.svg:
        svg_path = pathlib.Path(args.svg)
        svg_path.parent.mkdir(parents=True, exist_ok=True)

        qr = qrcode.QRCode(border=4, box_size=10, image_factory=qrcode.image.svg.SvgImage)
        qr.add_data(payload)
        qr.make(fit=True)
        svg_img = qr.make_image()
        svg_img.save(svg_path)

    payload_path = pathlib.Path(args.payload_out)
    payload_path.parent.mkdir(parents=True, exist_ok=True)
    payload_path.write_text(payload + "\n", encoding="utf-8")

    print(f"OK: payload={payload}")
    print(f"OK: png={png_path}")
    if args.svg:
        print(f"OK: svg={args.svg}")
    print(f"OK: payload_out={payload_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
