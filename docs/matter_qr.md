# Matter QR Generation

Generate a commissioning QR from onboarding payload.

## Input

The script expects a payload string starting with MT:.
Example payload source:

- Device log after Matter stack startup
- Factory provisioning pipeline output

## Command

python3 tools/generate_matter_qr.py --payload MT:XXXX --png out/matter_qr.png --svg out/matter_qr.svg

## Extract payload from monitor log

python3 tools/extract_matter_payload.py --log monitor.log

## Dependencies

python3 -m pip install qrcode[pil]

## Outputs

- PNG QR image
- Optional SVG QR image
- payload text file for record keeping
