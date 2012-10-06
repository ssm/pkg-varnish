/*-
 * Copyright (c) 2006 Verdens Gang AS
 * Copyright (c) 2006-2010 Varnish Software AS
 * All rights reserved.
 *
 * Author: Poul-Henning Kamp <phk@phk.freebsd.dk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Handle configuration of backends from VCL programs.
 *
 */

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include <sys/socket.h>

#include "cache.h"
#include "vcli.h"
#include "vrt.h"
#include "cache_backend.h"
#include "cli_priv.h"

struct lock VBE_mtx;


/*
 * The list of backends is not locked, it is only ever accessed from
 * the CLI thread, so there is no need.
 */
static VTAILQ_HEAD(, backend) backends = VTAILQ_HEAD_INITIALIZER(backends);

/*--------------------------------------------------------------------
 */

static void
VBE_Nuke(struct backend *b)
{

	ASSERT_CLI();
	VTAILQ_REMOVE(&backends, b, list);
	free(b->ipv4);
	free(b->ipv4_addr);
	free(b->ipv6);
	free(b->ipv6_addr);
	free(b->port);
	VSM_Free(b->vsc);
	FREE_OBJ(b);
	VSC_C_main->n_backend--;
}

/*--------------------------------------------------------------------
 */

void
VBE_Poll(void)
{
	struct backend *b, *b2;

	ASSERT_CLI();
	VTAILQ_FOREACH_SAFE(b, &backends, list, b2) {
		assert(
			b->admin_health == ah_healthy ||
			b->admin_health == ah_sick ||
			b->admin_health == ah_probe
		);
		if (b->refcount == 0 && b->probe == NULL)
			VBE_Nuke(b);
	}
}

/*--------------------------------------------------------------------
 * Drop a reference to a backend.
 * The last reference must come from the watcher in the CLI thread,
 * as only that thread is allowed to clean up the backend list.
 */

void
VBE_DropRefLocked(struct backend *b)
{
	int i;
	struct vbc *vbe, *vbe2;

	CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);
	assert(b->refcount > 0);

	i = --b->refcount;
	Lck_Unlock(&b->mtx);
	if (i > 0)
		return;

	ASSERT_CLI();
	VTAILQ_FOREACH_SAFE(vbe, &b->connlist, list, vbe2) {
		VTAILQ_REMOVE(&b->connlist, vbe, list);
		if (vbe->fd >= 0) {
			AZ(close(vbe->fd));
			vbe->fd = -1;
		}
		vbe->backend = NULL;
		VBE_ReleaseConn(vbe);
	}
	VBE_Nuke(b);
}

void
VBE_DropRefVcl(struct backend *b)
{

	CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);

	Lck_Lock(&b->mtx);
	b->vsc->vcls--;
	VBE_DropRefLocked(b);
}

void
VBE_DropRefConn(struct backend *b)
{

	CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);

	Lck_Lock(&b->mtx);
	assert(b->n_conn > 0);
	b->n_conn--;
	VBE_DropRefLocked(b);
}

/*--------------------------------------------------------------------
 * See lib/libvcl/vcc_backend.c::emit_sockaddr()
 */

static void
copy_sockaddr(struct sockaddr_storage **sa, socklen_t *len,
    const unsigned char *src)
{

	assert(*src > 0);
	*sa = calloc(sizeof **sa, 1);
	XXXAN(*sa);
	memcpy(*sa, src + 1, *src);
	*len = *src;
}

/*--------------------------------------------------------------------
 * Add a backend/director instance when loading a VCL.
 * If an existing backend is matched, grab a refcount and return.
 * Else create a new backend structure with reference initialized to one.
 */

struct backend *
VBE_AddBackend(struct cli *cli, const struct vrt_backend *vb)
{
	struct backend *b;
	char buf[128];

	AN(vb->vcl_name);
	assert(vb->ipv4_sockaddr != NULL || vb->ipv6_sockaddr != NULL);
	(void)cli;
	ASSERT_CLI();

	/* Run through the list and see if we already have this backend */
	VTAILQ_FOREACH(b, &backends, list) {
		CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);
		if (strcmp(b->vcl_name, vb->vcl_name))
			continue;
		if (vb->ipv4_sockaddr != NULL && (
		    b->ipv4len != vb->ipv4_sockaddr[0] ||
		    memcmp(b->ipv4, vb->ipv4_sockaddr + 1, b->ipv4len)))
			continue;
		if (vb->ipv6_sockaddr != NULL && (
		    b->ipv6len != vb->ipv6_sockaddr[0] ||
		    memcmp(b->ipv6, vb->ipv6_sockaddr + 1, b->ipv6len)))
			continue;
		b->refcount++;
		b->vsc->vcls++;
		return (b);
	}

	/* Create new backend */
	ALLOC_OBJ(b, BACKEND_MAGIC);
	XXXAN(b);
	Lck_New(&b->mtx, lck_backend);
	b->refcount = 1;

	bprintf(buf, "%s(%s,%s,%s)",
	    vb->vcl_name,
	    vb->ipv4_addr == NULL ? "" : vb->ipv4_addr,
	    vb->ipv6_addr == NULL ? "" : vb->ipv6_addr, vb->port);

	b->vsc = VSM_Alloc(sizeof *b->vsc, VSC_CLASS, VSC_TYPE_VBE, buf);
	b->vsc->vcls++;

	VTAILQ_INIT(&b->connlist);

	VTAILQ_INIT(&b->troublelist);

	/*
	 * This backend may live longer than the VCL that instantiated it
	 * so we cannot simply reference the VCL's copy of things.
	 */
	REPLACE(b->vcl_name, vb->vcl_name);
	REPLACE(b->ipv4_addr, vb->ipv4_addr);
	REPLACE(b->ipv6_addr, vb->ipv6_addr);
	REPLACE(b->port, vb->port);

	/*
	 * Copy over the sockaddrs
	 */
	if (vb->ipv4_sockaddr != NULL)
		copy_sockaddr(&b->ipv4, &b->ipv4len, vb->ipv4_sockaddr);
	if (vb->ipv6_sockaddr != NULL)
		copy_sockaddr(&b->ipv6, &b->ipv6len, vb->ipv6_sockaddr);

	assert(b->ipv4 != NULL || b->ipv6 != NULL);

	b->healthy = 1;
	b->admin_health = ah_probe;

	VTAILQ_INSERT_TAIL(&backends, b, list);
	VSC_C_main->n_backend++;
	return (b);
}

/*--------------------------------------------------------------------*/

void
VRT_init_dir(struct cli *cli, struct director **dir, const char *name,
    int idx, const void *priv)
{

	ASSERT_CLI();
	if (!strcmp(name, "simple"))
		VRT_init_dir_simple(cli, dir, idx, priv);
	else if (!strcmp(name, "hash"))
		VRT_init_dir_hash(cli, dir, idx, priv);
	else if (!strcmp(name, "random"))
		VRT_init_dir_random(cli, dir, idx, priv);
	else if (!strcmp(name, "dns"))
		VRT_init_dir_dns(cli, dir, idx, priv);
	else if (!strcmp(name, "round-robin"))
		VRT_init_dir_round_robin(cli, dir, idx, priv);
	else if (!strcmp(name, "fallback"))
		VRT_init_dir_fallback(cli, dir, idx, priv);
	else if (!strcmp(name, "client"))
		VRT_init_dir_client(cli, dir, idx, priv);
	else
		INCOMPL();
}

void
VRT_fini_dir(struct cli *cli, struct director *b)
{

	(void)cli;
	ASSERT_CLI();
	CHECK_OBJ_NOTNULL(b, DIRECTOR_MAGIC);
	b->fini(b);
	b->priv = NULL;
}

/*---------------------------------------------------------------------
 * String to admin_health
 */

static enum admin_health
vbe_str2adminhealth(const char *wstate)
{

	if (strcasecmp(wstate, "healthy") == 0)
		return(ah_healthy);
	if (strcasecmp(wstate, "sick") == 0)
		return(ah_sick);
	if (strcmp(wstate, "auto") == 0)
		return(ah_probe);
	return (ah_invalid);
}

/*---------------------------------------------------------------------
 * A general function for finding backends and doing things with them.
 *
 * Return -1 on match-argument parse errors.
 *
 * If the call-back function returns non-zero, the search is terminated
 * and we relay that return value.
 *
 * Otherwise we return the number of matches.
 */

typedef int bf_func(struct cli *cli, struct backend *b, void *priv);

static int
backend_find(struct cli *cli, const char *matcher, bf_func *func, void *priv)
{
	struct backend *b;
	const char *s;
	const char *name_b;
	ssize_t name_l = 0;
	const char *ip_b = NULL;
	ssize_t ip_l = 0;
	const char *port_b = NULL;
	ssize_t port_l = 0;
	int found = 0;
	int i;

	name_b = matcher;
	if (matcher != NULL) {
		s = strchr(matcher,'(');

		if (s != NULL)
			name_l = s - name_b;
		else
			name_l = strlen(name_b);

		if (s != NULL) {
			s++;
			while (isspace(*s))
				s++;
			ip_b = s;
			while (*s != '\0' &&
			    *s != ')' &&
			    *s != ':' &&
			    !isspace(*s))
				s++;
			ip_l = s - ip_b;
			while (isspace(*s))
				s++;
			if (*s == ':') {
				s++;
				while (isspace(*s))
					s++;
				port_b = s;
				while (*s != '\0' && *s != ')' && !isspace(*s))
					s++;
				port_l = s - port_b;
			}
			while (isspace(*s))
				s++;
			if (*s != ')') {
				VCLI_Out(cli,
				    "Match string syntax error:"
				    " ')' not found.");
				VCLI_SetResult(cli, CLIS_CANT);
				return (-1);
			}
			s++;
			while (isspace(*s))
				s++;
			if (*s != '\0') {
				VCLI_Out(cli,
				    "Match string syntax error:"
				    " junk after ')'");
				VCLI_SetResult(cli, CLIS_CANT);
				return (-1);
			}
		}
	}
	VTAILQ_FOREACH(b, &backends, list) {
		CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);
		if (port_b != NULL && strncmp(b->port, port_b, port_l) != 0)
			continue;
		if (name_b != NULL && strncmp(b->vcl_name, name_b, name_l) != 0)
			continue;
		if (ip_b != NULL &&
		    (b->ipv4_addr == NULL ||
		      strncmp(b->ipv4_addr, ip_b, ip_l)) &&
		    (b->ipv6_addr == NULL ||
		      strncmp(b->ipv6_addr, ip_b, ip_l)))
			continue;
		found++;
		i = func(cli, b, priv);
		if (i)
			return(i);
	}
	return (found);
}

/*---------------------------------------------------------------------*/

static int __match_proto__()
do_list(struct cli *cli, struct backend *b, void *priv)
{
	int *hdr;
	char buf[128];

	AN(priv);
	hdr = priv;
	if (!*hdr) {
		VCLI_Out(cli, "%-30s %-6s %-10s %s",
		    "Backend name", "Refs", "Admin", "Probe");
		*hdr = 1;
	}
	CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);

	bprintf(buf, "%s(%s,%s,%s)",
		b->vcl_name,
		b->ipv4_addr == NULL ? "" : b->ipv4_addr,
		b->ipv6_addr == NULL ? "" : b->ipv6_addr, b->port);
	VCLI_Out(cli, "\n%-30s %-6d", buf, b->refcount);

	if (b->admin_health == ah_probe)
		VCLI_Out(cli, " %-10s", "probe");
	else if (b->admin_health == ah_sick)
		VCLI_Out(cli, " %-10s", "sick");
	else if (b->admin_health == ah_healthy)
		VCLI_Out(cli, " %-10s", "healthy");
	else
		VCLI_Out(cli, " %-10s", "invalid");

	if (b->probe == NULL)
		VCLI_Out(cli, " %s", "Healthy (no probe)");
	else {
		if (b->healthy)
			VCLI_Out(cli, " %s", "Healthy ");
		else
			VCLI_Out(cli, " %s", "Sick ");
		VBP_Summary(cli, b->probe);
	}

	return (0);
}

static void
cli_backend_list(struct cli *cli, const char * const *av, void *priv)
{
	int hdr = 0;

	(void)av;
	(void)priv;
	ASSERT_CLI();
	(void)backend_find(cli, av[2], do_list, &hdr);
}

/*---------------------------------------------------------------------*/

static int __match_proto__()
do_set_health(struct cli *cli, struct backend *b, void *priv)
{
	enum admin_health state;

	(void)cli;
	state = *(enum admin_health*)priv;
	CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);
	b->admin_health = state;
	return (0);
}

static void
cli_backend_set_health(struct cli *cli, const char * const *av, void *priv)
{
	enum admin_health state;
	int n;

	(void)av;
	(void)priv;
	ASSERT_CLI();
	AN(av[2]);
	AN(av[3]);
	state = vbe_str2adminhealth(av[3]);
	if (state == ah_invalid) {
		VCLI_Out(cli, "Invalid state %s", av[3]);
		VCLI_SetResult(cli, CLIS_PARAM);
		return;
	}
	n = backend_find(cli, av[2], do_set_health, &state);
	if (n == 0) {
		VCLI_Out(cli, "No Backends matches");
		VCLI_SetResult(cli, CLIS_PARAM);
	}
}

/*---------------------------------------------------------------------*/

static struct cli_proto backend_cmds[] = {
	{ "backend.list", "backend.list",
	    "\tList all backends\n", 0, 1, "", cli_backend_list },
	{ "backend.set_health", "backend.set_health matcher state",
	    "\tShow a backend\n", 2, 2, "", cli_backend_set_health },
	{ NULL }
};

/*---------------------------------------------------------------------*/

void
VBE_Init(void)
{

	Lck_New(&VBE_mtx, lck_vbe);
	CLI_AddFuncs(backend_cmds);
}
