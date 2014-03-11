#!/usr/bin/env roundup
describe "test account wrappers"

it_lists_groups() {
  main_group=$(id -g -n)
  # (for g in $(groups) ; do echo $g ; done) | grep $main_group
  ../../sbin/lsgroups | grep $main_group
  
  # pattern matching
  [ "$(../../sbin/lsgroups nonmatchingpattern)" = "" ]
  ../../sbin/lsgroups $main_group | grep $main_group
  ../../sbin/lsgroups ${main_group}* | grep $main_group
}

it_creates_users() {
  sudo ../../sbin/rmusers -f kitest1 || true

  sudo ../../sbin/mkuser kitest1 kitestgr
  sudo -u kitest1 whoami
  sudo -u kitest1 groups | grep kitest1
  sudo -u kitest1 groups | grep kitestgr
  
  sudo ../../sbin/rmusers -f kitest1
  sudo ../../sbin/rmgroups -f kitestgr

  ! sudo -u kitest1 whoami
}
