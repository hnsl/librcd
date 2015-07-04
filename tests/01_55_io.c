/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"
#include "linux.h"

#pragma librcd

join_locked(fstr_mem_t*) get_bounced_message(join_server_params, fstr_t msg) {
    return fstr_cpy(msg);
}

fiber_main io_test_bounce_tcp(fiber_main_attr, int listen_port) {
    rio_in_addr4_t in_addr = {.address = RIO_IPV4_ADDR_PACK(127, 0, 0, 1), .port = listen_port};
    rio_t* tcp_server = rio_tcp_server(in_addr, 100);
    ifc_fiber_sync_recv();
    rio_in_addr4_t remote_addr;
    rio_t* tcp_client = rio_tcp_accept(tcp_server, &remote_addr);
    improb_atest(remote_addr.address == RIO_IPV4_ADDR_PACK(127, 0, 0, 1), "the remote tcp address was [", fss(rio_serial_in_addr4(remote_addr)), "] and not 127.0.0.1 as expected (some systems return another interface address)");
    fstr_mem_t* buffer = rio_read_fstr(tcp_client);
    accept_join(get_bounced_message, join_server_params, fstr_str(buffer));
}

fiber_main io_test_bounce_udp(fiber_main_attr, int listen_port) {
    fstr_t buffer = fstr_str(fstr_alloc(2000));
    fstr_t message;
    sub_heap {
        rio_in_addr4_t in_addr = {.address = RIO_IPV4_ADDR_PACK(127, 0, 0, 1), .port = listen_port};
        rio_t* udp_server = rio_udp_server(&in_addr);
        ifc_fiber_sync_recv();
        rio_in_addr4_t remote_addr;
        message = rio_msg_recv_udp(udp_server, buffer, &remote_addr);
        // In some systems the remote address might not be 127.0.0.1.
        improb_atest(remote_addr.address == RIO_IPV4_ADDR_PACK(127, 0, 0, 1), "the remote tcp address was [", fss(rio_serial_in_addr4(remote_addr)), "] and not 127.0.0.1 as expected (some systems return another interface address)");
    }
    accept_join(get_bounced_message, join_server_params, message);
}

fiber_main io_test_peek_lookahead(fiber_main_attr, int listen_port) {
    fstr_t expected_start = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed odio nibh, malesuada ut dapibus sit amet, interdum fringilla elit. Sed sed lobortis ante. Cras varius diam id diam fringilla id varius libero aliquet. Etiam id tortor a felis aliquet placerat. Nunc ac velit velit";
    fstr_t expected_end = ". In id magna odio. Praesent odio tellus, dapibus at aliquet et, porttitor ac lacus. Aenean vitae nibh et dui gravida semper. Aliquam iaculis fermentum porttitor. Curabitur lobortis nulla ut massa rutrum aliquet. In hac habitasse platea dictumst. Mauris a leo ullamcorper dolor semper feugiat. Sed posuere, nibh a tristique dictum, felis diam pharetra nibh, sit amet imperdiet orci sapien sed nunc. Ut porttitor laoreet magna a euismod. Aenean faucibus ante et ipsum elementum sit amet sodales lorem vehicula. Quisque convallis, velit nec dignissim rutrum, nunc orci molestie ligula, vel tempus nunc ligula nec quam.";
    sub_heap {
        rio_in_addr4_t in_addr = {.address = RIO_IPV4_ADDR_PACK(127, 0, 0, 1), .port = listen_port};
        rio_t* tcp_server = rio_tcp_server(in_addr, 100);
        ifc_fiber_sync_recv();
        rio_in_addr4_t remote_addr;
        rio_t* tcp_client = rio_tcp_accept(tcp_server, &remote_addr);
        tcp_client = rio_realloc_peek_buffer(tcp_client, 500);
        fstr_t buffer = fstr_str(fstr_alloc(PAGE_SIZE));
        fstr_t buffer_tail = buffer;
        fstr_t chunk;
        list(fstr_t)* chunk_log = new_list(fstr_t);
        fstr_t consumed_chunk;
        size_t refill_return_state = 0;
        for (bool match_found = false; !match_found;) {
            try {
                chunk = rio_peek(tcp_client);
            } catch(exception_io, e) {
                DBG("peek failed, chunks read: [", fss(fstr_implode(chunk_log, "], [")), "]");
                throw("peek failed", exception_arg);
            }
            consumed_chunk = chunk;
            list_push_end(chunk_log, fstr_t, fss(fstr_cpy(chunk)));
#pragma re2c(chunk, refill_buffer_lbl, refill_return_state): \s*velit[ ]velit {@found_match}
            // No match.
            atest(false);
found_match:
            match_found = true;
            // Adjust consumed chunk and slice off the not yet consumed bytes.
            consumed_chunk = fstr_sslice(consumed_chunk, 0, -((ssize_t) chunk.len) - 1);
refill_buffer_lbl:
            // Add the consumed chunk to the buffer.
            atest(consumed_chunk.len < buffer_tail.len);
            fstr_cpy_over(buffer_tail, consumed_chunk, &buffer_tail, 0);
            // Skip the already consumed part in the chunk.
            rio_skip(tcp_client, consumed_chunk.len);
        }
        // Get the buffer we read. (the start)
        fstr_t buffer_start = fstr_sslice(buffer, 0, -((ssize_t) buffer_tail.len) - 1);
        atest(fstr_equal(buffer_start, expected_start));
        buffer = buffer_tail;
        try {
            for (;;) {
                chunk = rio_read(tcp_client, buffer_tail);
                atest(chunk.len < buffer_tail.len);
                fstr_cpy_over(buffer_tail, chunk, &buffer_tail, 0);
            }
        } catch (exception_io, e) {}
        // Get the buffer we read. (the end)
        fstr_t buffer_end = fstr_sslice(buffer, 0, -((ssize_t) buffer_tail.len) - 1);
        atest(fstr_equal(buffer_end, expected_end));
        // Sync with the parent to indicate that the tests was successful and that it may proceed.
        ifc_fiber_sync_recv();
    }
}

fiber_main io_send_buffer(fiber_main_attr, fstr_t message, rio_t* send_to) {
    for (size_t i = 0; i < 2; i++)
        atest(rio_poll(send_to, false, (i == 0)));
    rio_write(send_to, message);
}

static bool io_test_abstract_poll(rcd_fid_t fid_arg, bool read, bool wait) {
    return true;
}

static fstr_t io_test_abstract_read(rcd_fid_t fid_arg, fstr_t buffer, bool* more_hint_out) {
    return ifc_pipe_read(fid_arg, buffer);
}

static const rio_class_t io_test_read_class = {
    .read_part_fn = io_test_abstract_read,
    .poll_fn = io_test_abstract_poll,
};

static fstr_t io_test_abstract_write(rcd_fid_t fid_arg, fstr_t buffer, bool more_hint) {
    ifc_pipe_write(fid_arg, buffer);
    return fstr_sslice(buffer, -1, -1);
}

static const rio_class_t io_test_write_class = {
    .write_part_fn = io_test_abstract_write,
    .poll_fn = io_test_abstract_poll,
};

static const rio_class_t io_test_rw_class = {
    .read_part_fn = io_test_abstract_read,
    .write_part_fn = io_test_abstract_write,
    .poll_fn = io_test_abstract_poll,
};

/// Theoretically it's not possible to test I/O but in practice it's helpful.
void rcd_self_test_io() {
    // Test serializing and unserializing an ipv4 address.
    sub_heap {
        fstr_t ipv4_addr_serial = fss(rio_serial_addr4(0x04ff0201));
        atest(fstr_equal(ipv4_addr_serial, "4.255.2.1"));
        uint32_t ipv4_addr = rio_unserial_addr4("254.0.255.7");
        atest(ipv4_addr == 0xfe00ff07);
        try {
            rio_unserial_addr4("254.0.255.7 ");
        } catch (exception_io, e) {}
        try {
            rio_unserial_addr4("254.256.2.1");
        } catch (exception_io, e) {}
        try {
            rio_unserial_addr4("");
        } catch (exception_io, e) {}
        try {
            rio_unserial_addr4("222222222222222222222222222222234512345");
        } catch (exception_io, e) {}
    }
    fstr_t test_message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed odio nibh, malesuada ut dapibus sit amet, interdum fringilla elit. Sed sed lobortis ante. Cras varius diam id diam fringilla id varius libero aliquet. Etiam id tortor a felis aliquet placerat. Nunc ac velit velit. In id magna odio. Praesent odio tellus, dapibus at aliquet et, porttitor ac lacus. Aenean vitae nibh et dui gravida semper. Aliquam iaculis fermentum porttitor. Curabitur lobortis nulla ut massa rutrum aliquet. In hac habitasse platea dictumst. Mauris a leo ullamcorper dolor semper feugiat. Sed posuere, nibh a tristique dictum, felis diam pharetra nibh, sit amet imperdiet orci sapien sed nunc. Ut porttitor laoreet magna a euismod. Aenean faucibus ante et ipsum elementum sit amet sodales lorem vehicula. Quisque convallis, velit nec dignissim rutrum, nunc orci molestie ligula, vel tempus nunc ligula nec quam.";
    // Test rio_file_exists().
    atest(rio_file_exists("/bin"));
    atest(!rio_file_exists("/!@NONEXISTING_FILE@!"));
    try {
        rio_file_exists("/bin/cat/foo");
        atest(false);
    } catch (exception_io, e) {}
    // Test for rio_mkdir().
    sub_heap {
        fstr_t dir_path = "/tmp/librcd-tmp-test-dir";
        try {
            rio_file_rmdir(dir_path);
        } catch (exception_io, e) {}
        rio_file_mkdir(dir_path);
        rio_file_list(dir_path);
        rio_file_rmdir(dir_path);
    }
    // Test for writing (creating) to a file and reading the data back.
    sub_heap {
        fstr_t file_path = "/tmp/librcd-tmp-test";
        try {
            rio_file_unlink(file_path);
        } catch (exception_io, e) {}
        rio_write_file_contents(file_path, test_message);
        fstr_t read_data = fss(rio_read_file_contents(file_path));
        atest(fstr_cmp(test_message, read_data) == 0);
        rio_file_unlink(file_path);
    }
    // Test listing the root and see that we can find bin tmp and var.
    sub_heap {
        list(fstr_mem_t*)* files = rio_file_list("/");
        int32_t bin_cnt = 0, tmp_cnt = 0, var_cnt = 0, dot_cnt = 0;
        list_foreach(files, fstr_mem_t*, file_mem) {
            fstr_t file = fss(file_mem);
            if (fstr_equal(file, "bin"))
                bin_cnt++;
            else if (fstr_equal(file, "tmp"))
                tmp_cnt++;
            else if (fstr_equal(file, "var"))
                var_cnt++;
            else if (fstr_equal(file, ".") || fstr_equal(file, ".."))
                dot_cnt++;
        }
        atest(bin_cnt == 1);
        atest(tmp_cnt == 1);
        atest(var_cnt == 1);
        atest(dot_cnt == 0);
    }
    // Create a fiber that bounces a TCP message. Do it twice on the same port to also test that the fd really closes and the port becomes available again.
    for (int i = 0; i < 2; i++) {
        sub_heap {
            int32_t test_port = 10000 + lwt_rdrand64() % 9999;
            rcd_fid_t child_fid;
            fmitosis {
                child_fid = spawn_static_fiber(io_test_bounce_tcp("", test_port));
            }
            sub_heap {
                // Sync run state to remove race condition.
                ifc_fiber_sync_send(child_fid);
                // Connect over TCP now.
                rio_in_addr4_t in_addr = {.address = RIO_IPV4_ADDR_PACK(127, 0, 0, 1), .port = test_port};
                rio_t* tcp_client = rio_tcp_client(in_addr);
                // Test rio_get_socket_address().
                rio_in_addr4_t in_addr_test = rio_get_socket_address(tcp_client, true);
                atest(in_addr_test.address == in_addr.address);
                atest(in_addr_test.port == in_addr.port);
                rio_in_addr4_t in_addr_test2 = rio_get_socket_address(tcp_client, false);
                atest(in_addr_test2.address == in_addr.address);
                // Pack data and send it in pieces.
                size_t length = test_message.len;
                size_t length_nbo = RIO_NBO_SWAP64(length);
                fstr_t data = fss(conc(FSTR_PACK(length_nbo), test_message));
                for (int size = 0; data.len > 0; size += 1) {
                    rio_write(tcp_client, fstr_slice(data, 0, size));
                    data = fstr_slice(data, size, data.len);
                }
            }
            // Get the message.
            fstr_t bounced_message = fss(get_bounced_message(child_fid));
            atest(fstr_equal(bounced_message, test_message));
        }
    }
    // Create a fiber that bounces an UDP message. Do it twice on the same port to also test that the fd really closes and the port becomes available again.
    for (int i = 0; i < 2; i++) {
        sub_heap {
            int32_t test_port = 10000 + lwt_rdrand64() % 9999;
            rcd_fid_t child_fid;
            fmitosis {
                child_fid = spawn_static_fiber(io_test_bounce_udp("", test_port));
            }
            sub_heap {
                // Sync run state to remove race condition.
                ifc_fiber_sync_send(child_fid);
                // Send UDP message now.
                rio_in_addr4_t in_addr = {.address = RIO_IPV4_ADDR_PACK(127, 0, 0, 1), .port = test_port};
                rio_t* udp_client = rio_udp_client(&in_addr);
                rio_msg_send_udp(udp_client, test_message, in_addr);
            }
            // Get the message.
            fstr_t bounced_message = fss(get_bounced_message(child_fid));
            atest(fstr_equal(bounced_message, test_message));
        }
    }
    // Test time and alarms.
    sub_heap {
        uint128_t timer_time = rio_get_time_timer();
        atest(timer_time > 0);
        uint128_t clock_time = rio_epoch_ns_now();
        atest(clock_time > 0);
        rio_t* timer = rio_timer_create();
        atest(timer != 0);
        rio_t* clock = rio_clock_create();
        atest(clock != 0);
        rio_alarm_set(timer, RIO_NS_MS * 3, false, RIO_NS_MS * 3);
        rio_alarm_set(clock, RIO_NS_MS, false, RIO_NS_MS);
        uint64_t n_timeouts = rio_alarm_wait(timer);
        improb_atest(n_timeouts == 1, "we got ", ui2fs(n_timeouts), " timeouts but expected exactly 1");
        {
            uint128_t timer_delta = rio_get_time_timer() - timer_time;
            atest(timer_delta > RIO_NS_MS * 2);
            // This can technically occur if we don't get enough CPU time, however we make the odds as small as possible by making the interval large.
            atest(timer_delta < RIO_NS_SEC * 100);
        }
        // This is technically undefined but the test should pass as long as nothing weird happens with the system.
        {
            uint128_t clock_delta = rio_epoch_ns_now() - clock_time;
            atest(clock_delta > RIO_NS_MS * 2);
            atest(clock_delta < RIO_NS_SEC);
        }
        // If we wait on the clock now it should return at least two timeouts and it should not wait at all.
        timer_time = rio_get_time_timer();
        n_timeouts = rio_alarm_wait(clock);
        improb_atest(n_timeouts >= 2 && n_timeouts <= 4, "we got ", ui2fs(n_timeouts), " timeouts but expected 2 - 4");
        {
            uint128_t timer_delta = rio_get_time_timer() - timer_time;
            // This can technically occur if we don't get enough CPU time, however we make the odds as small as possible by making the interval large.
            atest(timer_delta < RIO_NS_MS * 100);
        }
        {
            // Test rfc3339 parsing.
            rio_clock_time_t ct1 = rio_rfc3339_to_clock("2009-03-04T00:34:35.1234567899Z");
            atest(ct1.year == 2009
                && ct1.month == 3
                && ct1.month_day == 4
                && ct1.hour == 0
                && ct1.minute == 34
                && ct1.second == 35
                && ct1.nanosecond == 123456789);
            (void) rio_rfc3339_to_clock("2009-12-31T00:34:35+00:00");
            (void) rio_rfc3339_to_clock("2000-02-10T23:34:35.009Z");
        }
        {
            rio_clock_time_t ct1 = rio_rfc3339_to_clock("2009-03-04T00:34:35.123456789Z");
            atest(rio_clock_time_to_epoch(ct1) == 1236126875123456789);
            // Adds 2min 3s 123456789ns each loop.
            for (uint128_t t = 1236126875123456789; t < 1236126875123456789 + 123123456789 * 1000000; t += 62312345678900 )
                atest(t == rio_clock_time_to_epoch(rio_epoch_to_clock_time(t)));
        }
        // Test serializing a clock and expect a specific result.
        {
            rio_clock_time_t test_clock_time = rio_epoch_to_clock_time(1355795881234905503);
            rio_date_time_t test_date_time = rio_clock_to_date_time(test_clock_time);
            atest(test_date_time.year == 2012);
            atest(test_date_time.month == 12);
            atest(test_date_time.month_day == 18);
            atest(test_date_time.hour == 1);
            atest(test_date_time.minute == 58);
            atest(test_date_time.second == 1);
            atest(test_date_time.week_day == 1);
            atest(test_date_time.year_day == 352);
        }
        // Test ISO 8061 serialization.
        sub_heap {
            rio_clock_time_t ct = rio_iso8601_date_to_clock("2009-03-04");
            DBGFN(fss(rio_clock_to_iso8601_date(ct, false, false)));
            atest(fstr_equal(fss(rio_clock_to_iso8601_date(ct, false, false)), "2009-03-04"));
            atest(fstr_equal(fss(rio_clock_to_iso8601_date(ct, true, false)), "20090304"));
            atest(fstr_equal(fss(rio_clock_to_iso8601_date(ct, false, true)), "2009-03"));
            atest(ct.year == 2009);
            atest(ct.month == 3);
            atest(ct.month_day == 4);
            atest(ct.hour == 0);
            atest(ct.minute == 0);
            atest(ct.second == 0);
            atest(ct.nanosecond == 0);
        }
        // Test serializing a clock with rfc3339.
        sub_heap {
            rio_clock_time_t test_clock_time = rio_epoch_to_clock_time(1388758009123456789);
            fstr_t rfc3339_0 = fss(rio_clock_to_rfc3339(test_clock_time, 0));
            atest(fstr_equal(rfc3339_0, "2014-01-03T14:06:49Z"));
            fstr_t rfc3339_1 = fss(rio_clock_to_rfc3339(test_clock_time, 1));
            atest(fstr_equal(rfc3339_1, "2014-01-03T14:06:49.1Z"));
            fstr_t rfc3339_2 = fss(rio_clock_to_rfc3339(test_clock_time, 2));
            atest(fstr_equal(rfc3339_2, "2014-01-03T14:06:49.12Z"));
            fstr_t rfc3339_3 = fss(rio_clock_to_rfc3339(test_clock_time, 3));
            atest(fstr_equal(rfc3339_3, "2014-01-03T14:06:49.123Z"));
            fstr_t rfc3339_8 = fss(rio_clock_to_rfc3339(test_clock_time, 8));
            atest(fstr_equal(rfc3339_8, "2014-01-03T14:06:49.12345678Z"));
            fstr_t rfc3339_9 = fss(rio_clock_to_rfc3339(test_clock_time, 9));
            atest(fstr_equal(rfc3339_9, "2014-01-03T14:06:49.123456789Z"));
            fstr_t rfc3339_10 = fss(rio_clock_to_rfc3339(test_clock_time, 10));
            atest(fstr_equal(rfc3339_10, "2014-01-03T14:06:49.123456789Z"));
        }
        sub_heap {
            rio_clock_time_t test_clock_time = rio_epoch_to_clock_time(1251333275000000000);
            fstr_t rfc3339_0 = fss(rio_clock_to_rfc3339(test_clock_time, 0));
            atest(fstr_equal(rfc3339_0, "2009-08-27T00:34:35Z"));
            fstr_t rfc3339_1 = fss(rio_clock_to_rfc3339(test_clock_time, 1));
            atest(fstr_equal(rfc3339_1, "2009-08-27T00:34:35.0Z"));
            fstr_t rfc3339_2 = fss(rio_clock_to_rfc3339(test_clock_time, 2));
            atest(fstr_equal(rfc3339_2, "2009-08-27T00:34:35.00Z"));
            fstr_t rfc3339_3 = fss(rio_clock_to_rfc3339(test_clock_time, 3));
            atest(fstr_equal(rfc3339_3, "2009-08-27T00:34:35.000Z"));
            fstr_t rfc3339_8 = fss(rio_clock_to_rfc3339(test_clock_time, 8));
            atest(fstr_equal(rfc3339_8, "2009-08-27T00:34:35.00000000Z"));
            fstr_t rfc3339_9 = fss(rio_clock_to_rfc3339(test_clock_time, 9));
            atest(fstr_equal(rfc3339_9, "2009-08-27T00:34:35.000000000Z"));
            fstr_t rfc3339_10 = fss(rio_clock_to_rfc3339(test_clock_time, 10));
            atest(fstr_equal(rfc3339_10, "2009-08-27T00:34:35.000000000Z"));
        }
        sub_heap {
            rio_clock_time_t test_clock_time = rio_epoch_to_clock_time(1251333275000090000);
            fstr_t rfc3339_0 = fss(rio_clock_to_rfc3339(test_clock_time, 0));
            atest(fstr_equal(rfc3339_0, "2009-08-27T00:34:35Z"));
            fstr_t rfc3339_1 = fss(rio_clock_to_rfc3339(test_clock_time, 1));
            atest(fstr_equal(rfc3339_1, "2009-08-27T00:34:35.0Z"));
            fstr_t rfc3339_2 = fss(rio_clock_to_rfc3339(test_clock_time, 2));
            atest(fstr_equal(rfc3339_2, "2009-08-27T00:34:35.00Z"));
            fstr_t rfc3339_3 = fss(rio_clock_to_rfc3339(test_clock_time, 3));
            atest(fstr_equal(rfc3339_3, "2009-08-27T00:34:35.000Z"));
            fstr_t rfc3339_8 = fss(rio_clock_to_rfc3339(test_clock_time, 8));
            atest(fstr_equal(rfc3339_8, "2009-08-27T00:34:35.00009000Z"));
            fstr_t rfc3339_9 = fss(rio_clock_to_rfc3339(test_clock_time, 9));
            atest(fstr_equal(rfc3339_9, "2009-08-27T00:34:35.000090000Z"));
            fstr_t rfc3339_10 = fss(rio_clock_to_rfc3339(test_clock_time, 10));
            atest(fstr_equal(rfc3339_10, "2009-08-27T00:34:35.000090000Z"));
        }
        // Test serializing a clock with rfc1123.
        sub_heap {
            fstr_t rfc1123_0 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(0)));
            atest(fstr_equal(rfc1123_0, "Thu, 01 Jan 1970 00:00:00 GMT"));
            fstr_t rfc1123_1 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(1)));
            atest(fstr_equal(rfc1123_1, "Thu, 01 Jan 1970 00:00:00 GMT"));
            fstr_t rfc1123_2 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(2)));
            atest(fstr_equal(rfc1123_2, "Thu, 01 Jan 1970 00:00:00 GMT"));
            fstr_t rfc1123_3 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(RIO_NS_SEC - 1)));
            atest(fstr_equal(rfc1123_3, "Thu, 01 Jan 1970 00:00:00 GMT"));
            fstr_t rfc1123_4 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(1 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_4, "Thu, 01 Jan 1970 00:00:01 GMT"));
            fstr_t rfc1123_5 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(10 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_5, "Thu, 01 Jan 1970 00:00:10 GMT"));
            fstr_t rfc1123_6 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(100 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_6, "Thu, 01 Jan 1970 00:01:40 GMT"));
            fstr_t rfc1123_7 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(1000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_7, "Thu, 01 Jan 1970 00:16:40 GMT"));
            fstr_t rfc1123_8 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(10000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_8, "Thu, 01 Jan 1970 02:46:40 GMT"));
            fstr_t rfc1123_9 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(100000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_9, "Fri, 02 Jan 1970 03:46:40 GMT"));
            fstr_t rfc1123_10 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(1000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_10, "Mon, 12 Jan 1970 13:46:40 GMT"));
            fstr_t rfc1123_11 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(10000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_11, "Sun, 26 Apr 1970 17:46:40 GMT"));
            fstr_t rfc1123_12 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(100000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_12, "Sat, 03 Mar 1973 09:46:40 GMT"));
            fstr_t rfc1123_13 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(1000000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_13, "Sun, 09 Sep 2001 01:46:40 GMT"));
            fstr_t rfc1123_14 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(2000000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_14, "Wed, 18 May 2033 03:33:20 GMT"));
            fstr_t rfc1123_15 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(4000000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_15, "Tue, 02 Oct 2096 07:06:40 GMT"));
            fstr_t rfc1123_16 = fss(rio_clock_to_rfc1123(rio_epoch_to_clock_time(10000000000 * RIO_NS_SEC)));
            atest(fstr_equal(rfc1123_16, "Sat, 20 Nov 2286 17:46:40 GMT"));
        }

    }
    // Test absolute timeout and no repeat.
    sub_heap {
        uint128_t timer_time = rio_get_time_timer();
        rio_t* timer = rio_timer_create();
        rio_alarm_set(timer, timer_time + RIO_NS_MS, true, 0);
        uint64_t n_timeouts = rio_alarm_wait(timer);
        atest(n_timeouts >= 1);
        uint128_t timer_delta = rio_get_time_timer() - timer_time;
        atest(timer_delta >= RIO_NS_MS);
    }
    // Test eventfd.
    sub_heap {
        rio_t* eventfd = rio_eventfd_create(1, false);
        atest(rio_eventfd_wait(eventfd) == 1);
        rio_eventfd_trigger(eventfd, 0xff);
        rio_eventfd_trigger(eventfd, 0xff);
        atest(rio_eventfd_wait(eventfd) == 0xff * 2);
    }
    // Test splitting an eventfd.
    sub_heap {
        rio_t *exit_eventfd_h = rio_eventfd_create(2, false), *exit_eventfd_rh, *exit_eventfd_wh;
        rio_realloc_split(exit_eventfd_h, &exit_eventfd_rh, &exit_eventfd_wh);
        atest(rio_eventfd_wait(exit_eventfd_rh) == 2);
        rio_eventfd_trigger(exit_eventfd_wh, 0xff);
        rio_eventfd_trigger(exit_eventfd_wh, 0xff);
        atest(rio_eventfd_wait(exit_eventfd_rh) == 0xff * 2);
    }
    // Test peek + lookahead of tcp stream.
    for (int i = 0; i < 2; i++) {
        sub_heap {
            int32_t test_port = 10000 + lwt_rdrand64() % 9999;
            rcd_fid_t child_fid;
            fmitosis {
                child_fid = spawn_static_fiber(io_test_peek_lookahead("", test_port));
            }
            sub_heap {
                // Sync run state to remove race condition.
                ifc_fiber_sync_send(child_fid);
                // Connect over TCP now.
                rio_in_addr4_t in_addr = {.address = RIO_IPV4_ADDR_PACK(127, 0, 0, 1), .port = test_port};
                rio_t* tcp_client = rio_tcp_client(in_addr);
                // Send the message 3-5 bytes at a time every 1 ms.
                fstr_t data = test_message;
                for (int size = 0; data.len > 0; size = (size + 1) % 3) {
                    size_t total_size = 3 + size;
                    rio_write(tcp_client, fstr_slice(data, 0, total_size));
                    data = fstr_slice(data, total_size, data.len);
                }
            }
            // Wait for child to receive and verify.
            ifc_fiber_sync_send(child_fid);
        }
    }
    // Test which. This varies between environment so we can't exactly write a good unit test.
    sub_heap {
        atest(!fstr_equal(fss(rio_which("env")), ""));
        try {
            rio_which("foo-bar");
            atest(false);
        } catch (exception_io, e) {}
    }
    // Test execute of a simple binary (wait for exit code).
    sub_heap {
        int32_t exit_code = rio_proc_execute_and_wait(fss(rio_which("bash")), new_list(fstr_t, "-c", "exit 42"), true);
        atest(exit_code == 42);
    }
    sub_heap {
        rio_proc_t* proc_h;
        rio_t* pipe;
        rio_proc_execute_and_pipe(fss(rio_which("cat")), new_list(fstr_t), true, &proc_h, &pipe);
        rio_t *pipe_r, *pipe_w;
        rio_realloc_split(pipe, &pipe_r, &pipe_w);
        // Test that reading and writing is not possible on the wrong ends.
        try {
            rio_write(pipe_r, "test");
            atest(false);
        } catch (exception_arg, e) {}
        try {
            rio_read(pipe_w, test_message);
            atest(false);
        } catch (exception_arg, e) {}
        fmitosis {
            lwt_alloc_import(pipe_w);
            spawn_static_fiber(io_send_buffer("", fss(fstr_cpy(test_message)), pipe_w));
        }
        fstr_t test_message_recv = fss(fstr_alloc(test_message.len));
        rio_read_fill(pipe_r, test_message_recv);
        atest(fstr_equal(test_message, test_message_recv));
    }
    // Test execute of a non existing binary. We should get the execve error at the point of rio_proc_execute and not later.
    sub_heap {
        try {
            int32_t exit_code = rio_proc_execute_and_wait(concs("/bin/", fss(fstr_hexrandom(12))), new_list(fstr_t), true);
            atest(false);
        } catch_eio(syscall, e, data) {
            atest(data.errno_v == ENOENT);
        }
    }
    // Test rio_exec() wrapper function.
    sub_heap {
        fstr_t file_path = "/tmp/librcd-tmp-test";
        try {
            rio_file_unlink(file_path);
        } catch (exception_io, e) {}
        rio_exec("touch", new_list(fstr_t, file_path));
        atest(rio_file_exists(file_path));
        rio_file_unlink(file_path);
        try {
            rio_exec("cat", new_list(fstr_t, "--no-such-arg"));
            atest(false);
        } catch (exception_io, e) {};
    }
    // Test passing a file descriptor through IPC (testing unix dgram and stream sockets)
    sub_heap {
        fstr_t path = fss(lwt_get_program_path());
        rio_t *unix_dgram0_h, *unix_dgram1_h;
        rio_open_unix_socket_dgram_pair(&unix_dgram0_h, &unix_dgram1_h);
        // This does not work if there was a previous trap in the program (BREAKPT), the subprocess shuts down instantly with exit code 0, investigate this.
        rio_sub_exec_t se = { .exec = {
            .path = path,
            .args = new_list(fstr_t, "01_55_io/ipc_test_post_execve"),
            .io_in = unix_dgram1_h,
        }};
        rio_proc_t* proc_h = rio_proc_execute(se);
        lwt_alloc_free(unix_dgram1_h);
        rio_t *unix_stream0_h, *unix_stream1_h;
        rio_open_unix_socket_stream_pair(&unix_stream0_h, &unix_stream1_h);
        rio_ipc_fd_send(unix_dgram0_h, unix_stream1_h);
        rio_write_fstr(unix_stream0_h, "yolo");
        fstr_t buffer = fss(rio_read_fstr(unix_stream0_h));
        atest(fstr_equal(buffer, "swag"));
    }
    // Test abstract streams.
    sub_heap {
        rcd_sub_fiber_t* ifc_pipe = ifc_create_pipe(32);
        rcd_fid_t ifc_pipe_fid = lwt_get_sub_fiber_id(ifc_pipe);
        rio_t* abstract_rio_pipe_r = rio_new_abstract(&io_test_read_class, ifc_pipe_fid, 0);
        fmitosis {
            rio_t* abstract_rio_pipe_w = rio_new_abstract(&io_test_write_class, ifc_pipe_fid, 0);
            spawn_static_fiber(io_send_buffer("", fss(fstr_cpy(test_message)), abstract_rio_pipe_w));
        }
        for (size_t i = 0; i < 2; i++)
            atest(rio_poll(abstract_rio_pipe_r, true, (i == 0)));
        fstr_t test_message_recv = fss(fstr_alloc(test_message.len));
        rio_read_fill(abstract_rio_pipe_r, test_message_recv);
        atest(fstr_equal(test_message, test_message_recv));
    }
    // Test read to separator (complex read peeking).
    for (size_t i = 0; i < 2; i++) sub_heap {
        rcd_sub_fiber_t* ifc_pipe = ifc_create_pipe(9);
        rcd_fid_t ifc_pipe_fid = lwt_get_sub_fiber_id(ifc_pipe);
        rio_t *abstract_rio_pipe_r, *abstract_rio_pipe_w;
        if (i == 0) {
            abstract_rio_pipe_r = rio_new_abstract(&io_test_read_class, ifc_pipe_fid, 0);
        } else {
            // Test splitting an abstract rio stream.
            rio_t* abstract_rio_pipe_h = rio_new_abstract(&io_test_rw_class, ifc_pipe_fid, 0);
            rio_write(abstract_rio_pipe_h, "...");
            fstr_t peek_buf = rio_peek(abstract_rio_pipe_h);
            atest(peek_buf.len > 0);
            atest(peek_buf.str[0] == '.');
            rio_realloc_split(abstract_rio_pipe_h, &abstract_rio_pipe_r, &abstract_rio_pipe_w);
            lwt_alloc_free(abstract_rio_pipe_h);
            fstr_t rbuf = fss(fstr_alloc(3));
            rio_read_fill(abstract_rio_pipe_r, rbuf);
            atest(fstr_equal(rbuf, "..."));
        }
        // Test that writing to read side fails.
        try {
            rio_write(abstract_rio_pipe_r, ".");
            assert(false);
        } catch (exception_arg, e);
        fmitosis {
            if (i == 0) {
                abstract_rio_pipe_w = rio_new_abstract(&io_test_write_class, ifc_pipe_fid, 0);
            } else {
                lwt_alloc_import(abstract_rio_pipe_w);
            }
            // Test that reading to write side fails.
            try {
                rio_read(abstract_rio_pipe_w, ".");
                assert(false);
            } catch (exception_arg, e);
            spawn_static_fiber(io_send_buffer("", fss(fstr_cpy("hello world how are you\nthis is the second message\nmore testing more testing\ndon't forget this data")), abstract_rio_pipe_w));
        }
        fstr_t buffer = fss(fstr_alloc(0x1000));
        for (size_t i = 0; i < 2; i++)
            atest(rio_poll(abstract_rio_pipe_r, true, (i == 0)));
        rio_t* peek_pipe_r = rio_realloc_peek_buffer(abstract_rio_pipe_r, 0x1000);
        {
            fstr_t data = rio_read_to_separator(peek_pipe_r, "\n", buffer);
            atest(fstr_equal(data, "hello world how are you"));
        }{
            fstr_t data = rio_read_to_separator(peek_pipe_r, "\n", buffer);
            atest(fstr_equal(data, "this is the second message"));
        }{
            fstr_t data = rio_read_to_separator(peek_pipe_r, "\n", buffer);
            atest(fstr_equal(data, "more testing more testing"));
        }{
            fstr_t expect_end = "don't forget this data";
            fstr_t data = fstr_slice(buffer, 0, expect_end.len);
            rio_read_fill(peek_pipe_r, data);
            atest(fstr_equal(data, expect_end));
        }
    }
    // Test hostname and dns lookups.
    sub_heap {
        list(uint32_t)* addr_list = rio_resolve_host_ipv4_addr("www.google.com");
        atest(list_count(addr_list, uint32_t) >= 1);
    } sub_heap {
        list(uint32_t)* addr_list = rio_resolve_host_ipv4_addr("google-public-dns-a.google.com");
        atest(list_count(addr_list, uint32_t) == 1);
        atest(list_peek_start(addr_list, uint32_t) == RIO_IPV4_ADDR_PACK(8, 8, 8, 8));
    } sub_heap {
        list(uint32_t)* addr_list = rio_resolve_host_ipv4_addr("google-public-dns-b.google.com");
        atest(list_count(addr_list, uint32_t) == 1);
        atest(list_peek_start(addr_list, uint32_t) == RIO_IPV4_ADDR_PACK(8, 8, 4, 4));
    } sub_heap {
        try {
            list(uint32_t)* addr_list = rio_resolve_host_ipv4_addr(concs("a", fss(fstr_hexrandom(12)), "x.com"));
            atest(false);
        } catch (exception_io, e) {}
    }
}

void ipc_test_post_execve() {
    rio_t* unix_dgram1_h = rio_new_h(rio_type_unix_dgram, STDIN_FILENO, true, true, 0);
    rio_t* unix_stream1_h = rio_ipc_fd_recv(unix_dgram1_h, rio_type_unix_stream, true, true);
    fstr_t buffer = fss(rio_read_fstr(unix_stream1_h));
    atest(fstr_equal(buffer, "yolo"));
    rio_write_fstr(unix_stream1_h, "swag");
    try {
        rio_read(unix_stream1_h, fss(fstr_alloc(1)));
    } catch(exception_io, e) {};
}
