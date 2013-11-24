#!/bin/sh

test_description="varnishncsa service"

. ./sharness.sh

test_expect_success "enable" "
  sed -i -e 's/# VARNISHNCSA_ENABLED=1/VARNISHNCSA_ENABLED=1/g' /etc/default/varnishncsa
"

test_expect_success "start" "
  service varnishncsa start
"

test_expect_success "status (after start)" "
  service varnishncsa status
"

test_expect_success "restart" "
  service varnishncsa restart
"

test_expect_success "status (after restart)" "
  service varnishncsa status
"

test_expect_success "reload" "
  service varnishncsa reload
"

test_expect_success "status (after reload)" "
  service varnishncsa status
"

test_expect_success "stop" "
  service varnishncsa stop
"

test_expect_success "status (after stop)" "
  if service varnishncsa status
  then
    false
  else
    true
  fi
"


test_done
