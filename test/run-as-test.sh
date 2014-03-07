#!/usr/bin/env roundup
describe "test run-as wrapper"

it_verifies_setup() {
  test "$(echo $(groups kinko-test-server | tr ' ' "\n" | grep kinko | sort))" = \
    "kinko-test-server"
  
  test "$(echo $(groups kinko-test-client | tr ' ' "\n" | grep kinko | sort))" = \
      "kinko-test-client kinko-test-server kinko-test-server-limited"
}

it_runs_directly() {
  test "$(bin/kinko-test-server $(which whoami))" = "kinko-test-server"
  test "$(bin/kinko-test-client $(which whoami))" = "kinko-test-client"
}

it_cannot_call_client_from_server() {
  ! bin/kinko-test-server bin/kinko-test-client $(which whoami)
}

it_can_call_server_from_client() {
  test "$(bin/kinko-test-client bin/kinko-test-server $(which whoami))" = "kinko-test-server"
}

it_parses_pidfile() {
  rm -f foo.pid
  r=$(bin/kinko-test-client --pidfile foo.pid --keep-pidfile $(which echo) bar)
  test "$r" == "bar"
  test -f foo.pid

  r=$(bin/kinko-test-client --pidfile foo.pid $(which echo) bar)
  test "$r" == "bar"
  ! test -f foo.pid
}

# as the resulting user and group is determined by the basename of the
# executable a possible attack could be to just execute the binary
# with ARG[0] set differently.
#
# This test ensures that this is not working.
it_cannot_be_redirected_via_argv0() {
  res=$(
    (
    exec -a kinko-test-server bin/kinko-test-client $(which whoami)
    ) &
    wait
  )

  test "$res" = "kinko-test-client"
}

it_handles_access_rights() {
  cat=$(which cat)
  ./bin/kinko-test-client $cat var/test-client-limited.data 
  ./bin/kinko-test-client $cat var/test-client.data 
  ./bin/kinko-test-server $cat var/test-server-limited.data 
  ./bin/kinko-test-server $cat var/test-server.data 

  # The client can access the server data
  ./bin/kinko-test-client $cat var/test-server.data 
  ./bin/kinko-test-client $cat var/test-server-limited.data 

  # The server cannot access the client data
  ! ./bin/kinko-test-server $cat var/test-client-limited.data 
  ! ./bin/kinko-test-server $cat var/test-client.data 
}

it_handles_access_rights_in_subdirs() {
  cat=$(which cat)
  # ./bin/kinko-test-client $cat var/test-client-limited/data 
  ./bin/kinko-test-client $cat var/test-client/data 
  # ./bin/kinko-test-server $cat var/test-server-limited/data 
  ./bin/kinko-test-server $cat var/test-server/data 
  
  # The client can access the server data
  ./bin/kinko-test-client $cat var/test-server/data 
  # ./bin/kinko-test-client $cat var/test-server-limited/data 
  
  # The server cannot access the client data
  ! ./bin/kinko-test-server $cat var/test-client-limited/data 
  ! ./bin/kinko-test-server $cat var/test-client/data 
}

it_modifies_environment() {
  printenv=$(which printenv)

  test "/tmp" = $(./bin/kinko-test-client $printenv TMPDIR)
  test "kinko-test-client" = $(./bin/kinko-test-client $printenv USER)
  homebase=$(cd .. && pwd)/apps
  test "$homebase/kinko-test-client/var" = $(./bin/kinko-test-client $printenv HOME)
}
