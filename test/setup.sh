#!/bin/bash
set -e

mkdir -p bin var
sudo install -o root -g kinko-test-server -m 4710 ../bin/run-as bin/kinko-test-server
sudo install -o root -g kinko-test-client -m 4710 ../bin/run-as bin/kinko-test-client
sudo install -o root -g kinko-test-server-limited -m 4710 ../bin/run-as bin/kinko-test-server-limited
sudo install -o root -g kinko-test-client-limited -m 4710 ../bin/run-as bin/kinko-test-client-limited

rm -rf var/*

echo "test-server content"          > var/test-server.data
echo "test-server-limited content"  > var/test-server-limited.data
echo "test-client content"          > var/test-client.data
echo "test-client-limited content"  > var/test-client-limited.data

chmod 660 var/*.data

sudo chown kinko-test-server:kinko-test-server         var/test-server.data
sudo chown kinko-test-server:kinko-test-server-limited var/test-server-limited.data
sudo chown kinko-test-client:kinko-test-client         var/test-client.data
sudo chown kinko-test-client:kinko-test-client-limited var/test-client-limited.data

for f in test-server test-client ; do
  mkdir -p var/$f
  chmod 6770 var/$f
  sudo chown kinko-$f:kinko-$f var/$f
  echo "$f content"          > var/$f/data
  echo sudo chown kinko-$f:kinko-$f var/$f
done
