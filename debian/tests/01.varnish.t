#!/bin/sh

test_description="varnishd service"

. ./sharness.sh

test_expect_success "status (started by default)" "
  service varnish status
"

test_expect_success "restart" "
  service varnish restart
"

test_expect_success "status (after restart)" "
  service varnish status
"

test_expect_success "reload" "
  service varnish reload
"

test_expect_success "status (after reload)" "
  service varnish status
"

test_expect_success "stop" "
  service varnish stop
"

test_expect_success "status (after stop)" "
  if service varnish status
  then
    false
  else
    true
  fi
"

test_done
