/*
 * Check decoding of PTP_* commands of ioctl syscall.
 *
 * Copyright (c) 2018 Harsha Sharma <harshasharmaiitr@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#ifdef HAVE_STRUCT_PTP_SYS_OFFSET

#include "xlat.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <unistd.h>
#include <linux/ptp_clock.h>

#include "xlat/ptp_flags_options.h"

static void
test_no_device(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_clock_caps, caps);
	fill_memory(caps, sizeof(*caps));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_sys_offset, sysoff);
	fill_memory(sysoff, sizeof(*sysoff));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_extts_request, extts);
	fill_memory(extts, sizeof(*extts));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_perout_request, perout);

	/* PTP_CLOCK_GETCAPS */
	ioctl(-1, PTP_CLOCK_GETCAPS, NULL);
	printf("ioctl(-1, PTP_CLOCK_GETCAPS, NULL) = -1 EBADF (%m)\n");
	ioctl(-1, PTP_CLOCK_GETCAPS, caps);
	printf("ioctl(-1, PTP_CLOCK_GETCAPS, %p) = -1 EBADF (%m)\n", caps);

	/* PTP_SYS_OFFSET */
	ioctl(-1, PTP_SYS_OFFSET, NULL);
	printf("ioctl(-1, PTP_SYS_OFFSET, NULL) = -1 EBADF (%m)\n");
	ioctl(-1, PTP_SYS_OFFSET, sysoff);
	printf("ioctl(-1, PTP_SYS_OFFSET, {n_samples=%u}) = -1 EBADF (%m)\n",
	       sysoff->n_samples);

	/* PTP_ENABLE_PPS */
	ioctl(-1, PTP_ENABLE_PPS, 0);
	printf("ioctl(-1, PTP_ENABLE_PPS, 0) = -1 EBADF (%m)\n");
	ioctl(-1, PTP_ENABLE_PPS, 1);
	printf("ioctl(-1, PTP_ENABLE_PPS, 1) = -1 EBADF (%m)\n");

	/* PTP_EXTTS_REQUEST */
	ioctl(-1, PTP_EXTTS_REQUEST, NULL);
	printf("ioctl(-1, PTP_EXTTS_REQUEST, NULL) = -1 EBADF (%m)\n");
	ioctl(-1, PTP_EXTTS_REQUEST, extts);
	printf("ioctl(-1, PTP_EXTTS_REQUEST, {index=%d, flags=", extts->index);
	printflags(ptp_flags_options, extts->flags, "PTP_???");
	printf("}) = -1 EBADF (%m)\n");

	/* PTP_PEROUT_REQUEST */
	ioctl(-1, PTP_PEROUT_REQUEST, NULL);
	printf("ioctl(-1, PTP_PEROUT_REQUEST, NULL) = -1 EBADF (%m)\n");
	ioctl(-1, PTP_PEROUT_REQUEST, perout);
	printf("ioctl(-1, PTP_PEROUT_REQUEST, {start={sec=%" PRId64 ""
	       ", nsec=%" PRIu32 "}, period={sec=%" PRId64 ", nsec=%" PRIu32 "}"
	       ", index=%d, flags=", (int64_t)perout->start.sec,
	       perout->start.nsec, (int64_t)perout->period.sec,
	       perout->period.nsec, perout->index);
	printflags(ptp_flags_options, perout->flags, "PTP_???");
	printf("}) = -1 EBADF (%m)\n");
}
static void
test_ptp0_device(void)
{

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_clock_caps, caps);
	fill_memory(caps, sizeof(*caps));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_sys_offset, sysoff);
	fill_memory(sysoff, sizeof(*sysoff));

	static const char device[] = "/dev/ptp0";
	int fd = open(device, O_RDWR);
	if (fd < 0) {
		perror(device);
		return;
	}

	/* PTP_CLOCK_GETCAPS */
	int getcaps_ret = ioctl(fd, PTP_CLOCK_GETCAPS, caps);
	if (getcaps_ret < 0) {
		printf("ioctl(%d, PTP_CLOCK_GETCAPS, %p) = %s\n",
		       fd, caps,  sprintrc(getcaps_ret));
	} else {
		printf("ioctl(%d, PTP_CLOCK_GETCAPS, {max_adj=%d, n_alarm=%d"
		    ", n_ext_ts=%d, n_per_out=%d, pps=%d}) = %s\n",
		    fd, caps->max_adj, caps->n_alarm, caps->n_ext_ts,
		    caps->n_per_out, caps->pps, sprintrc(getcaps_ret));
	}

	/* PTP_SYS_OFFSET */
	sysoff->n_samples = PTP_MAX_SAMPLES;
	int sysoff_ret = ioctl(fd, PTP_SYS_OFFSET, sysoff);
	if (sysoff_ret < 0) {
		printf("ioctl(%d, PTP_SYS_OFFSET, {n_samples=%u}) = %s\n",
		       fd, sysoff->n_samples, sprintrc(sysoff_ret));
	} else {
		unsigned int i;

		printf("ioctl(%d, PTP_SYS_OFFSET, ts=[", fd);
		for (i = 0; i < 2 * sysoff->n_samples + 1; ++i) {
			if (i > 0)
				printf(", ");
			printf("{sec=%" PRId64 ", nsec=%" PRIu32 "}",
			       (int64_t)sysoff->ts[i].sec, sysoff->ts[i].nsec);
		}
		printf("]) = %s\n", sprintrc(sysoff_ret));
	}

}

int
main(void)
{
	test_no_device();
	test_ptp0_device();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_STRUCT_PTP_SYS_OFFSET")

#endif /* HAVE_STRUCT_PTP_SYS_OFFSET */
