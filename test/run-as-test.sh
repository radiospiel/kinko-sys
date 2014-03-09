#!/usr/bin/env roundup
describe "test run-as wrapper"

it_verifies_setup() {
  test "$(echo $(groups kitest-server | tr ' ' "\n" | grep kitest | sort))" = \
    "kitest-server"
  
  test "$(echo $(groups kitest-client | tr ' ' "\n" | grep kitest | sort))" = \
      "kitest-client kitest-server kitest-server-limited"
}

it_runs_directly() {
  test "$(bin/kitest-server $(which whoami))" = "kitest-server"
  test "$(bin/kitest-client $(which whoami))" = "kitest-client"
}

it_cannot_call_client_from_server() {
  ! bin/kitest-server bin/kitest-client $(which whoami)
}

it_can_call_server_from_client() {
  test "$(bin/kitest-client bin/kitest-server $(which whoami))" = "kitest-server"
}

it_parses_pidfile() {
  rm -f foo.pid
  r=$(bin/kitest-client --pidfile foo.pid --keep-pidfile $(which echo) bar)
  test "$r" == "bar"
  test -f foo.pid

  r=$(bin/kitest-client --pidfile foo.pid $(which echo) bar)
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
    exec -a kitest-server bin/kitest-client $(which whoami)
    ) &
    wait
  )

  test "$res" = "kitest-client"
}

it_handles_access_rights() {
  cat=$(which cat)
  ./bin/kitest-client $cat var/test-client-limited.data 
  ./bin/kitest-client $cat var/test-client.data 
  ./bin/kitest-server $cat var/test-server-limited.data 
  ./bin/kitest-server $cat var/test-server.data 

  # The client can access the server data
  ./bin/kitest-client $cat var/test-server.data 
  ./bin/kitest-client $cat var/test-server-limited.data 

  # The server cannot access the client data
  ! ./bin/kitest-server $cat var/test-client-limited.data 
  ! ./bin/kitest-server $cat var/test-client.data 
}

it_handles_access_rights_in_subdirs() {
  cat=$(which cat)
  # ./bin/kitest-client $cat var/test-client-limited/data 
  ./bin/kitest-client $cat var/test-client/data 
  # ./bin/kitest-server $cat var/test-server-limited/data 
  ./bin/kitest-server $cat var/test-server/data 
  
  # The client can access the server data
  ./bin/kitest-client $cat var/test-server/data 
  # ./bin/kitest-client $cat var/test-server-limited/data 
  
  # The server cannot access the client data
  ! ./bin/kitest-server $cat var/test-client-limited/data 
  ! ./bin/kitest-server $cat var/test-client/data 
}

it_modifies_environment() {
  printenv=$(which printenv)

  test "/tmp" = $(./bin/kitest-client $printenv TMPDIR)
  test "kitest-client" = $(./bin/kitest-client $printenv USER)

  test "$HOME" = $(./bin/kitest-client $printenv HOME)
}
