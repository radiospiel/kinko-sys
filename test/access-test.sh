#!/usr/bin/env roundup
describe "test run-as wrapper"

it_handles_access_rights_in_subdirs() {
  true
  # # The current user should be a member of the kinko-test-server group.
  # 
  # # It should be able to run the ./bin/kinko-test-server program
  # ./bin/kinko-test-server $(which pwd)
  # 
  # # The test file should exist
  # sudo cat var2/test-server.data
  # 
  # # but the current user should not be able to access it; neither directly..
  # ! cat var2/test-server.data
  # 
  # # ..nor via the bin/kinko-test-server command
  # ! ./bin/kinko-test-server $(which cat) var2/test-server.data
}
