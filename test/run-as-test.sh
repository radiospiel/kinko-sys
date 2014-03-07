#!/usr/bin/env roundup
describe "test run-as wrapper"

it_verifies_setup() {
  test "$(echo $(groups kinko-test-server | tr ' ' "\n" | grep kinko | sort))" = "kinko-test-server"
  test "$(echo $(groups kinko-test-client | tr ' ' "\n" | grep kinko | sort))" = "kinko-test-client kinko-test-server"
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
