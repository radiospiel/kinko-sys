#!/bin/bash
set -e

mkdir -p bin var
sudo install -o root -g kinko-test-server -m 4710 ../bin/run-as bin/kinko-test-server
sudo install -o root -g kinko-test-client -m 4710 ../bin/run-as bin/kinko-test-client
sudo install -o root -g kinko-test-server-limited -m 4710 ../bin/run-as bin/kinko-test-server-limited
sudo install -o root -g kinko-test-client-limited -m 4710 ../bin/run-as bin/kinko-test-client-limited

# -- create var2 tree ----------------------------------------------------------

rm -rf var2/*
mkdir -p var2

echo "test-server content"          > var2/test-server.data

chmod 600 var2/*.data
sudo chown kinko-test-server:kinko-test-server var2/test-server.data
