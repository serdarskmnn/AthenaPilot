#!/usr/bin/env bash
# Run the Ground Control Station (GCS) from the repo root so imports resolve.
set -e
cd "$(dirname "$0")"
python -m ground_control.main
