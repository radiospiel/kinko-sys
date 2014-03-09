#!/bin/bash
set -e

mkdir -p bin var
sudo install -o root -g kitest-server -m 4710 ../bin/run-as bin/kitest-server
sudo install -o root -g kitest-client -m 4710 ../bin/run-as bin/kitest-client
sudo install -o root -g kitest-server-limited -m 4710 ../bin/run-as bin/kitest-server-limited
sudo install -o root -g kitest-client-limited -m 4710 ../bin/run-as bin/kitest-client-limited

# -- create var2 tree ----------------------------------------------------------

rm -rf var2/*
mkdir -p var2

echo "test-server content"          > var2/test-server.data

chmod 600 var2/*.data
sudo chown kitest-server:kitest-server var2/test-server.data
