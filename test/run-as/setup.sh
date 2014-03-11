#!/bin/bash
set -e

RUN_AS=../../bin/run-as

mkdir -p bin var

sudo install -o root -g kitest-server -m 4710 $RUN_AS bin/kitest-server
sudo install -o root -g kitest-client -m 4710 $RUN_AS bin/kitest-client
sudo install -o root -g kitest-server-limited -m 4710 $RUN_AS bin/kitest-server-limited
sudo install -o root -g kitest-client-limited -m 4710 $RUN_AS bin/kitest-client-limited
