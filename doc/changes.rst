================================
Changes from 3.0.4 rc 1 to 3.0.4
================================

varnishd
--------

- Set the waiter pipe as non-blocking and record overflows.  `Bug
  #1285`
- Fix up a bug in the ACL compile code that could lead to false
  negatives.  CVE-2013-4090.    `Bug #1312`
- Return an error if the client sends multiple Host headers.

.. _bug #1285: http://varnish-cache.org/trac/ticket/1285
.. _bug #1312: http://varnish-cache.org/trac/ticket/1312

================================
Changes from 3.0.3 to 3.0.4 rc 1
================================

varnishd
--------

- Fix error handling when uncompressing fetched objects for ESI
  processing. `Bug #1184`
- Be clearer about which timeout was reached in logs.
- Correctly decrement n_waitinglist counter.  `Bug #1261`
- Turn off Nagle/set TCP_NODELAY.
- Avoid panic on malformed Vary headers.  `Bug #1275`
- Increase the maximum length of backend names.  `Bug #1224`
- Add support for banning on http.status.  `Bug #1076`
- Make hit-for-pass correctly prefer the transient storage.

.. _bug #1076: http://varnish-cache.org/trac/ticket/1076
.. _bug #1184: http://varnish-cache.org/trac/ticket/1184
.. _bug #1224: http://varnish-cache.org/trac/ticket/1224
.. _bug #1261: http://varnish-cache.org/trac/ticket/1261
.. _bug #1275: http://varnish-cache.org/trac/ticket/1275


varnishlog
----------

- If -m, but neither -b or -c is given, assume both.  This filters out
  a lot of noise when -m is used to filter.  `Bug #1071`

.. _bug #1071: http://varnish-cache.org/trac/ticket/1071

varnishadm
----------

- Improve tab completion and require libedit/readline to build.

varnishtop
----------

- Reopen log file if Varnish is restarted.

varnishncsa
-----------

- Handle file descriptors above 64k (by ignoring them).  Prevents a
  crash in some cases with corrupted shared memory logs.
- Add %D and %T support for more timing information.

Other
-----

- Documentation updates.
- Fixes for OSX
- Disable PCRE JIT-er, since it's broken in some PCRE versions, at
  least on i386.
- Make libvarnish prefer exact hits when looking for VSL tags.

===========================
Changes from 3.0.2 to 3.0.3
===========================

Varnishd
--------

- Fix a race on the n_sess counter. This race made varnish do excessive
  session workspace allocations. `Bug #897`_.
- Fix some crashes in the gzip code when it runs out of memory. `Bug #1037`_.
  `Bug #1043`_. `Bug #1044`_.
- Fix a bug where the regular expression parser could end up in an infinite
  loop. `Bug #1047`_.
- Fix a memory leak in the regex code.
- DNS director now uses port 80 by default if not specified.
- Introduce `idle_send_timeout` and increase default value for `send_timeout`
  to 600s. This allows a long send timeout for slow clients while still being
  able to disconnect idle clients.
- Fix an issue where <esi:remove> did not remove HTML comments. `Bug #1092`_.
- Fix a crash when passing with streaming on.
- Fix a crash in the idle session timeout code.
- Fix an issue where the poll waiter did not timeout clients if all clients
  were idle. `Bug #1023`_.
- Log regex errors instead of crashing.
- Introduce `pcre_match_limit`, and `pcre_match_limit_recursion` parameters.
- Add CLI commands to manually control health state of a backend.
- Fix an issue where the s_bodybytes counter is not updated correctly on
  gunzipped delivery.
- Fix a crash when we couldn't allocate memory for a fetched object.
  `Bug #1100`_.
- Fix an issue where objects could end up in the transient store with a
  long TTL, when memory could not be allocated for them in the requested
  store. `Bug #1140`_.
- Activate req.hash_ignore_busy when req.hash_always_miss is activated.
  `Bug #1073`_.
- Reject invalid tcp port numbers for listen address. `Bug #1035`_.
- Enable JIT for better performing regular expressions. `Bug #1080`_.
- Return VCL errors in exit code when using -C. `Bug #1069`_.
- Stricter validation of acl syntax, to avoid a crash with 5-octet IPv4
  addresses. `Bug #1126`_.
- Fix a crash when first argument to regsub was null. `Bug #1125`_.
- Fix a case where varnish delivered corrupt gzip content when using ESI.
  `Bug #1109`_.
- Fix a case where varnish didn't remove the old Date header and served
  it alongside the varnish-generated Date header. `Bug #1104`_.
- Make saint mode work, for the case where we have no object with that hash.
  `Bug #1091`_.
- Don't save the object body on hit-for-pass objects.
- n_ban_gone counter added to count the number of "gone" bans.
- Ban lurker rewritten to properly sleep when no bans are present, and
  otherwise to process the list at the configured speed.
- Fix a case where varnish delivered wrong content for an uncompressed page
  with compressed ESI child. `Bug #1029`_.
- Fix an issue where varnish runs out of thread workspace when processing
  many ESI includes on an object. `Bug #1038`_.
- Fix a crash when streaming was enabled for an empty body.
- Better error reporting for some fetch errors.
- Small performance optimizations.

.. _bug #897: http://varnish-cache.org/trac/ticket/897
.. _bug #1023: http://varnish-cache.org/trac/ticket/1023
.. _bug #1029: http://varnish-cache.org/trac/ticket/1029
.. _bug #1023: http://varnish-cache.org/trac/ticket/1023
.. _bug #1035: http://varnish-cache.org/trac/ticket/1035
.. _bug #1037: http://varnish-cache.org/trac/ticket/1037
.. _bug #1038: http://varnish-cache.org/trac/ticket/1038
.. _bug #1043: http://varnish-cache.org/trac/ticket/1043
.. _bug #1044: http://varnish-cache.org/trac/ticket/1044
.. _bug #1047: http://varnish-cache.org/trac/ticket/1047
.. _bug #1069: http://varnish-cache.org/trac/ticket/1069
.. _bug #1073: http://varnish-cache.org/trac/ticket/1073
.. _bug #1080: http://varnish-cache.org/trac/ticket/1080
.. _bug #1091: http://varnish-cache.org/trac/ticket/1091
.. _bug #1092: http://varnish-cache.org/trac/ticket/1092
.. _bug #1100: http://varnish-cache.org/trac/ticket/1100
.. _bug #1104: http://varnish-cache.org/trac/ticket/1104
.. _bug #1109: http://varnish-cache.org/trac/ticket/1109
.. _bug #1125: http://varnish-cache.org/trac/ticket/1125
.. _bug #1126: http://varnish-cache.org/trac/ticket/1126
.. _bug #1140: http://varnish-cache.org/trac/ticket/1140

varnishncsa
-----------

- Support for \t\n in varnishncsa format strings.
- Add new format: %{VCL_Log:foo}x which output key:value from std.log() in
  VCL.
- Add user-defined date formatting, using %{format}t.

varnishtest
-----------

- resp.body is now available for inspection.
- Make it possible to test for the absence of an HTTP header. `Bug #1062`_.
- Log the full panic message instead of shortening it to 512 characters.

.. _bug #1062: http://varnish-cache.org/trac/ticket/1062

varnishstat
-----------

- Add json output (-j).

Other
-----

- Documentation updates.
- Bump minimum number of threads to 50 in RPM packages.
- RPM packaging updates.
- Fix some compilation warnings on Solaris.
- Fix some build issues on Open/Net/DragonFly-BSD.
- Fix build on FreeBSD 10-current.
- Fix libedit detection on \*BSD OSes. `Bug #1003`_.

.. _bug #1003: http://varnish-cache.org/trac/ticket/1003

================================
Changes from 3.0.2 rc 1 to 3.0.2
================================

Varnishd
--------

- Make the size of the synthetic object workspace equal to
  `http_resp_size` and add workaround to avoid a crash when setting
  too long response strings for synthetic objects.

- Ensure the ban lurker always sleeps the advertised 1 second when it
  does not have anything to do.

- Remove error from `vcl_deliver`.  Previously this would assert while
  it will now give a syntax error.

varnishncsa
-----------

- Add default values for some fields when logging incomplete records
  and document the default values.

Other
-----

- Documentation updates

- Some Solaris portability updates.

================================
Changes from 3.0.1 to 3.0.2 rc 1
================================

Varnishd
--------

- Only log the first 20 bytes of extra headers to prevent overflows.

- Fix crasher bug which sometimes happened if responses are queued and
  the backend sends us Vary. `Bug #994`_.

- Log correct size of compressed when uncompressing them for clients
  that do not support compression. `Bug #996`_.

- Only send Range responses if we are going to send a body. `Bug #1007`_.

- When varnishd creates a storage file, also unlink it to avoid
  leaking disk space over time.  `Bug #1008`_.

- The default size of the `-s file` parameter has been changed to
  100MB instead of 50% of the available disk space.

- The limit on the number of objects we remove from the cache to make
  room for a new one was mistakenly lowered to 10 in 3.0.1.  This has
  been raised back to 50.  `Bug #1012`_.

- `http_req_size` and `http_resp_size` have been increased to 8192
  bytes.  This better matches what other HTTPds have.   `Bug #1016`_.

.. _bug #994: http://varnish-cache.org/trac/ticket/994
.. _bug #992: http://varnish-cache.org/trac/ticket/992
.. _bug #996: http://varnish-cache.org/trac/ticket/996
.. _bug #1007: http://varnish-cache.org/trac/ticket/1007
.. _bug #1008: http://varnish-cache.org/trac/ticket/1008
.. _bug #1012: http://varnish-cache.org/trac/ticket/1012
.. _bug #1016: http://varnish-cache.org/trac/ticket/1016

VCL
---

- Allow relational comparisons of floating point types.

- Make it possible for vmods to fail loading and so cause the VCL
  loading to fail.

varnishncsa
-----------

- Fixed crash when client was sending illegal HTTP headers.

- `%{Varnish:handling}` in format strings was broken, this has been
  fixed.

Other
-----

- Documentation updates

- Some Solaris portability updates.

================================
Changes from 3.0.1 rc 1 to 3.0.1
================================

Varnishd
--------

- Fix crash in streaming code.

- Add `fallback` director, as a variant of the `round-robin`
  director.

- The parameter `http_req_size` has been reduced on 32 bit machines.

VCL
---

- Disallow error in the `vcl_init` and `vcl_fini` VCL functions.

varnishncsa
-----------

- Fixed crash when using `-X`.

- Fix error when the time to first byte was in the format string.

Other
-----

- Documentation updates

================================
Changes from 3.0.0 to 3.0.1 rc 1
================================

Varnishd
--------

- Avoid sending an empty end-chunk when sending bodyless responsed.

- `http_resp_hdr_len` and `http_req_hdr_len` were set to too low
  values leading to clients receiving `HTTP 400 Bad Request` errors.
  The limit has been increased and the error code is now `HTTP 413
  Request entity too large`.

- Objects with grace or keep set were mistakenly considered as
  candidates for the transient storage.  They now have their grace and
  keep limited to limit the memory usage of the transient stevedore.

- If a request was restarted from `vcl_miss` or `vcl_pass` it would
  crash.  This has been fixed.  `Bug #965`_.

- Only the first few clients waiting for an object from the backend
  would be woken up when object arrived and this lead to some clients
  getting stuck for a long time.  This has now been fixed. `Bug #963`_.

- The `hash` and `client` directors would mistakenly retry fetching an
  object from the same backend unless health probes were enabled.
  This has been fixed and it will now retry a different backend.

.. _bug #965: http://varnish-cache.org/trac/ticket/965
.. _bug #963: http://varnish-cache.org/trac/ticket/963

VCL
---

- Request specific variables such as `client.*` and `server.*` are now
  correctly marked as not available in `vcl_init` and `vcl_fini`.

- The VCL compiler would fault if two IP comparisons were done on the
  same line.  This now works correctly.  `Bug #948`_.

.. _bug #948: http://varnish-cache.org/trac/ticket/948

varnishncsa
-----------

- Add support for logging arbitrary request and response headers.

- Fix crashes if `hitmiss` and `handling` have not yet been set.

- Avoid printing partial log lines if there is an error in a format
  string.

- Report user specified format string errors better.

varnishlog
----------

- `varnishlog -r` now works correctly again and no longer opens the
  shared log file of the running Varnish.

Other
-----

- Various documentation updates.

- Minor compilation fixes for newer compilers.

- A bug in the ESI entity replacement parser has been fixed.  `Bug
  #961`_.

- The ABI of vmods are now checked.  This will require a rebuild of
  all vmods against the new version of Varnish.

.. _bug #961: http://varnish-cache.org/trac/ticket/961

================================
Changes from 3.0 beta 2 to 3.0.0
================================

Varnishd
--------

- Avoid sending an empty end-chunk when sending bodyless responsed.

VCL
---

- The `synthetic` keyword has now been properly marked as only
  available in `vcl_deliver`.  `Bug #936`_.

.. _bug #936: http://varnish-cache.org/trac/ticket/936

varnishadm
----------

- Fix crash if the secret file was unreadable.  `Bug #935`_.

- Always exit if `varnishadm` can't connect to the backend for any
  reason.

.. _bug #935: http://varnish-cache.org/trac/ticket/935

=====================================
Changes from 3.0 beta 1 to 3.0 beta 2
=====================================

Varnishd
--------

- thread_pool_min and thread_pool_max now each refer to the number of
  threads per pool, rather than being inconsistent as they were
  before.

- 307 Temporary redirect is now considered cacheable.  `Bug #908`_.

- The `stats` command has been removed from the CLI interface.  With
  the new counters, it would mean implementing more and more of
  varnishstat in the master CLI process and the CLI is
  single-threaded so we do not want to do this work there in the first
  place.  Use varnishstat instead.

.. _bug #908: http://varnish-cache.org/trac/ticket/908

VCL
---

- VCL now treats null arguments (unset headers for instance) as empty
  strings.  `Bug #913`_.

- VCL now has vcl_init and vcl_fini functions that are called when a
  given VCL has been loaded and unloaded.

- There is no longer any interpolation of the right hand side in bans
  where the ban is a single string.  This was confusing and you now
  have to make sure bits are inside or outside string context as
  appropriate.

- Varnish is now stricter in enforcing no duplication of probes,
  backends and ACLs.

.. _bug #913: http://varnish-cache.org/trac/ticket/913

varnishncsa
-----------

- varnishncsa now ignores piped requests, since we have no way of
  knowing their return status.

VMODs
-----

- The std module now has proper documentation, including a manual page

================================
Changes from 2.1.5 to 3.0 beta 1
================================

Upcoming changes
----------------

- The interpretation of bans will change slightly between 3.0 beta 1
  and 3.0 release.  Currently, doing ``ban("req.url == req.url")``
  will cause the right hand req.url to be interpreted in the context
  of the request creating the ban.  This will change so you will have
  to do ``ban("req.url == " + req.url)`` instead.  This syntax already
  works and is recommended.

Varnishd
--------

- Add streaming on ``pass`` and ``miss``.  This is controlled by the
  ``beresp.do_stream`` boolean.  This includes support for
  compression/uncompression.
- Add support for ESI and gzip.
- Handle objects larger than 2G.
- HTTP Range support is now enabled by default
- The ban lurker is enabled by default
- if there is a backend or director with the name ``default``, use
  that as the default backend, otherwise use the first one listed.
- Add many more stats counters.  Amongst those, add per storage
  backend stats and per-backend statistics.
- Syslog the platform we are running on
- The ``-l`` (shared memory log file) argument has been changed,
  please see the varnishd manual for the new syntax.
- The ``-S`` and ``-T`` arguments are now stored in the shmlog
- Fix off-by-one error when exactly filling up the workspace.  `Bug #693`_.
- Make it possible to name storage backends.  The names have to be
  unique.
- Update usage output to match the code.  `Bug #683`_
- Add per-backend health information to shared memory log.
- Always recreate the shared memory log on startup.
- Add a ``vcl_dir`` parameter.  This is used to resolve relative path
  names for ``vcl.load`` and ``include`` in .vcl files.
- Make it possible to specify ``-T :0``.  This causes varnishd to look
  for a free port automatically.  The port is written in the shared
  memory log so varnishadm can find it.
- Classify locks into kinds and collect stats for each kind,
  recording the data in the shared memory log.
- Auto-detect necessary flags for pthread support and ``VCC_CC``
  flags.  This should make Varnish somewhat happier on Solaris.  `Bug
  #663`_
- The ``overflow_max`` parameter has been renamed to ``queue_max``.
- If setting a parameter fails, report which parameter failed as this
  is not obvious during startup.
- Add a parameter named ``shortlived``.  Objects whose TTL is less
  than the parameter go into transient (malloc) storage.
- Reduce the default ``thread_add_delay`` to 2ms.
- The ``max_esi_includes`` parameter has been renamed to
  ``max_esi_depth``.
- Hash string components are now logged by default.
- The default connect timeout parameter has been increased to 0.7
  seconds.
- The ``err_ttl`` parameter has been removed and is replaced by a
  setting in default.vcl.
- The default ``send_timeout`` parameter has been reduced to 1 minute.
- The default ``ban_lurker`` sleep has been set to 10ms.
- When an object is banned, make sure to set its grace to 0 as well.
- Add ``panic.show`` and ``panic.clear`` CLI commands.
- The default ``http_resp_hdr_len`` and ``http_req_hdr_len`` has been
  increased to 2048 bytes.
- If ``vcl_fetch`` results in ``restart`` or ``error``, close the
  backend connection rather than fetching the object.
- If allocating storage for an object, try reducing the chunk size
  before evicting objects to make room.  `Bug #880`_
- Add ``restart`` from ``vcl_deliver``.  `Bug #411`_
- Fix an off-by-up-to-one-minus-epsilon bug where if an object from
  the backend did not have a last-modified header we would send out a
  304 response which did include a ``Last-Modified`` header set to
  when we received the object.  However, we would compare the
  timestamp to the fractional second we got the object, meaning any
  request with the exact timestamp would get a ``200`` response rather
  than the correct ``304``.
- Fix a race condition in the ban lurker where a serving thread and
  the lurker would both look at an object at the same time, leading to
  Varnish crashing.
- If a backend sends a ``Content-Length`` header and we are streaming and
  we are not uncompressing it, send the ``Content-Length`` header on,
  allowing browsers to diplay a progress bar.
- All storage must be at least 1M large.  This is to prevent
  administrator errors when specifying the size of storage where the
  admin might have forgotten to specify units.

.. _bug #693: http://varnish-cache.org/trac/ticket/693
.. _bug #683: http://varnish-cache.org/trac/ticket/683
.. _bug #663: http://varnish-cache.org/trac/ticket/663
.. _bug #880: http://varnish-cache.org/trac/ticket/880
.. _bug #411: http://varnish-cache.org/trac/ticket/411
.. _bug #693: http://varnish-cache.org/trac/ticket/693

Tools
-----

common
******

- Add an ``-m $tag:$regex`` parameter, used for selecting some
  transactions.  The parameter can be repeated, in which case it is
  logically and-ed together.

varnishadm
**********

- varnishadm will now pick up the -S and -T arguments from the shared
  memory log, meaning just running it without any arguments will
  connect to the running varnish.  `Bug #875`_
- varnishadm now accepts an -n argument to specify the location of the
  shared memory log file
- add libedit support

.. _bug #875: http://varnish-cache.org/trac/ticket/875

varnishstat
***********

- reopen shared memory log if the varnishd process is restarted.
- Improve support for selecting some, but not all fields using the
  ``-f`` argument. Please see the documentation for further details on
  the use of ``-f``.
- display per-backend health information

varnishncsa
***********

- Report error if called with ``-i`` and ``-I`` as they do not make
  any sense for varnishncsa.
- Add custom log formats, specified with ``-F``.  Most of the Apache
  log formats are supported, as well as some Varnish-specific ones.
  See the documentation for further information.  `Bug #712`_ and `bug #485`_

.. _bug #712: http://varnish-cache.org/trac/ticket/712
.. _bug #485: http://varnish-cache.org/trac/ticket/485

varnishtest
***********

- add ``-l`` and ``-L`` switches which leave ``/tmp/vtc.*`` behind on
  error and unconditionally respectively.
- add ``-j`` parameter to run tests in parallell and use this by
  default.

varnishtop
**********

- add ``-p $period`` parameter.  The units in varnishtop were
  previously undefined, they are now in requests/period.  The default
  period is 60 seconds.

varnishlog
**********

- group requests by default.  This can be turned off by using ``-O``
- the ``-o`` parameter is now a no-op and is ignored.

VMODs
-----

- Add a std vmod which includes a random function, log, syslog,
  fileread, collect,

VCL
---

- Change string concatenation to be done using ``+`` rather than
  implicitly.
- Stop using ``%xx`` escapes in VCL strings.
- Change ``req.hash += value`` to ``hash_data(value)``
- Variables in VCL now have distinct read/write access
- ``bereq.connect_timeout`` is now available in ``vcl_pipe``.
- Make it possible to declare probes outside of a director. Please see
  the documentation on how to do this.
- The VCL compiler has been reworked greatly, expanding its abilities
  with regards to what kinds of expressions it understands.
- Add ``beresp.backend.name``, ``beresp.backend.ip`` and
  ``beresp.backend.port`` variables.  They are only available from
  ``vcl_fetch`` and are read only.  `Bug #481`_
- The default VCL now calls pass for any objects where
  ``beresp.http.Vary == "*"``.  `Bug #787`_
- The ``log`` keyword has been moved to the ``std`` vmod.
- It is now possible to choose which storage backend to be used
- Add variables ``storage.$name.free_space``,
  ``storage.$name.used_space`` and ``storage.$name.happy``
- The variable ``req.can_gzip`` tells us whether the client accepts
  gzipped objects or not.
- ``purge`` is now called ``ban``, since that is what it really is and
  has always been.
- ``req.esi_level`` is now available.  `Bug #782`_
- esi handling is now controlled by the ``beresp.do_esi`` boolean rather
  than the ``esi`` function.
- ``beresp.do_gzip`` and ``beresp.do_gunzip`` now control whether an
  uncompressed object should be compressed and a compressed object
  should be uncompressed in the cache.
- make it possible to control compression level using the
  ``gzip_level`` parameter.
- ``obj.cacheable`` and ``beresp.cacheable`` have been removed.
  Cacheability is now solely through the ``beresp.ttl`` and
  ``beresp.grace`` variables.
- setting the ``obj.ttl`` or ``beresp.ttl`` to zero now also sets the
  corresponding grace to zero.  If you want a non-zero grace, set
  grace after setting the TTL.
- ``return(pass)`` in ``vcl_fetch`` has been renamed to
  ``return(hit_for_pass)`` to make it clear that pass in ``vcl_fetch``
  and ``vcl_recv`` are different beasts.
- Add actual purge support.  Doing ``purge`` will remove an object and
  all its variants.

.. _bug #481: http://varnish-cache.org/trac/ticket/481
.. _bug #787: http://varnish-cache.org/trac/ticket/787
.. _bug #782: http://varnish-cache.org/trac/ticket/782


Libraries
---------

- ``libvarnishapi`` has been overhauled and the API has been broken.
  Please see git commit logs and the support tools to understand
  what's been changed.
- Add functions to walk over all the available counters.  This is
  needed because some of the counter names might only be available at
  runtime.
- Limit the amount of time varnishapi waits for a shared memory log
  to appear before returning an error.
- All libraries but ``libvarnishapi`` have been moved to a private
  directory as they are not for public consumption and have no ABI/API
  guarantees.

Other
-----

- Python is now required to build
- Varnish Cache is now consistently named Varnish Cache.
- The compilation process now looks for kqueue on NetBSD
- Make it possible to use a system jemalloc rather than the bundled
  version.
- The documentation has been improved all over and should now be in
  much better shape than before
