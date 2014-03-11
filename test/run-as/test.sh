#!/usr/bin/env roundup
describe "test run-as wrapper"

before() {
  mkdir -p var/bin

  # The private beacon, must be stored 0600: accessible by the 
  # kitest-server user only.
  sudo rm -f var/beacon.private
  echo "beacon.private" > var/beacon.private
  chmod 600 var/beacon.private
  sudo chown kitest-server:kitest-server var/beacon.private
}

it_verifies_configuration() {
  # The private beacon should exist, and everyone may know about it...
  ls var/beacon.private

  # the current user should not be able to access it.
  ! cat var/beacon.private

  # but the test-server user should be able to access it.
  sudo -u kitest-server cat var/beacon.private
}

it_handles_access_rights_for_binary_code() {
  # Install the klc binary
  gcc -o bin/klc src/klc.c -Wall
  sudo install -o kitest-server -g kitest-server -m 04750 bin/klc var/bin/test-server-klc
  ls -ld var/bin/test-server-klc

  # klc must fail when called by the current user: the current user is 
  # not a member of the test-server group
  ! var/bin/test-server-klc

  # the kitest-server user, however, can run klc
  sudo -u kitest-server var/bin/test-server-klc var/beacon.private

  # the kitest-client user, however, can run klc
  sudo -u kitest-client var/bin/test-server-klc var/beacon.private
}

it_handles_access_rights_for_scripts() {
  # Note: The code below is similar to the testcase in
  # it_handles_access_rights_for_binary_code() above; however, the installed
  # binary is a script. The OS should ignore the setuid and setgid bits on 
  # these - consequently the code would not run through.. 
  #
  # For a discussion why this is a good idea check the internets.
  #
  # Install the klc script
  sudo install -o kitest-server -g kitest-server -m 04750 src/klc.sh var/bin/test-server-klc.sh
  ls -ld var/bin/test-server-klc.sh

  # klc must fail when called by the current user: the current user is 
  # not a member of the test-server group
  ! var/bin/test-server-klc.sh

  # the kitest-server user, however, can run klc
  sudo -u kitest-server var/bin/test-server-klc.sh var/beacon.private

  # but the kitest-client user can't.
  ! sudo -u kitest-client var/bin/test-server-klc.sh var/beacon.private
}
