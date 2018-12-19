#!/bin/bash
cd "$(dirname "$0")/.."
set -e

docker build -f tests/Dockerfile . -t memleax-tests
docker run --cap-add=SYS_PTRACE --privileged -it memleax-tests sh -c "cd /src/tests && make tests"
