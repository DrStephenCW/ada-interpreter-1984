#!/bin/sh
# regenerate src/ from the tape and the rebuilt files, then build
set -e
cd "$(dirname "$0")"
rm -rf src lib && mkdir src
python3 port.py "${TAPE:-../original}" "${REBUILT:-../rebuild}" src
make -s 2>&1 | grep -v "gets. function\|in function .get_user_comment" || true
