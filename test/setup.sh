#!/bin/bash
set -e

mkdir -p bin var
sudo install -o root -g kitest-server -m 4710 ../bin/run-as bin/kitest-server
sudo install -o root -g kitest-client -m 4710 ../bin/run-as bin/kitest-client
sudo install -o root -g kitest-server-limited -m 4710 ../bin/run-as bin/kitest-server-limited
sudo install -o root -g kitest-client-limited -m 4710 ../bin/run-as bin/kitest-client-limited

rm -rf var/*

echo "test-server content"          > var/test-server.data
echo "test-server-limited content"  > var/test-server-limited.data
echo "test-client content"          > var/test-client.data
echo "test-client-limited content"  > var/test-client-limited.data

chmod 660 var/*.data

sudo chown kitest-server:kitest-server         var/test-server.data
sudo chown kitest-server:kitest-server-limited var/test-server-limited.data
sudo chown kitest-client:kitest-client         var/test-client.data
sudo chown kitest-client:kitest-client-limited var/test-client-limited.data

for f in test-server test-client ; do
  mkdir -p var/$f
  chmod 6770 var/$f
  sudo chown ki$f:ki$f var/$f
  echo "$f content"          > var/$f/data
  echo sudo chown ki$f:ki$f var/$f
done
