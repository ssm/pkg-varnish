#!/bin/sh

test_description="varnishlog service"

. ./sharness.sh

test_expect_success "enable" "
  sed -i -e 's/# VARNISHLOG_ENABLED=1/VARNISHLOG_ENABLED=1/g' /etc/default/varnishlog
"

test_expect_success "start" "
  service varnishlog start
"

test_expect_success "status (after start)" "
  service varnishlog status
"

test_expect_success "reload" "
  service varnishlog reload
"

test_expect_success "status (after reload)" "
  service varnishlog status
"

test_expect_success "restart" "
  service varnishlog restart
"

test_expect_success "status (after restart)" "
  service varnishlog status
"

test_expect_success "stop" "
  service varnishlog stop
"

test_expect_success "status (after stop)" "
  if service varnishlog status
  then
    false
  else
    true
  fi
"


test_done
