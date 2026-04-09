#! /usr/bin/env bash
set -euo pipefail

echo "all tests: `ls Sample*`"
for i in `ls Sample*`; do echo "test - $i"; ./$i; done

