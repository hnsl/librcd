/* Copyright © 2014, Jumpstarter AB. This file is part of the librcd project.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/* See the COPYING file distributed with this project for more information. */

#include "rcd.h"

#pragma librcd

join_locked(int) multi_fiber_test_get_number(int add_no, join_server_params, int test_number, rcd_fid_t expect_server_fiber_id) {
    atest(server_fiber_id == expect_server_fiber_id);
    atest(test_number == add_no);
    return test_number + add_no;
}

fiber_main multi_fiber_test_main(fiber_main_attr, int* test_number) {
    accept_join(multi_fiber_test_get_number, join_server_params, *test_number, lwt_get_fiber_id());
}

fiber_main multi_fiber_test_run_sync(fiber_main_attr, rcd_fid_t parent_fiber_id, int* output) {
    int x = 0;
    for (; x < 100000; x++);
    *output = x;
    rcd_fid_t fiber_id = ifc_fiber_sync_recv();
    atest(fiber_id == parent_fiber_id);
}

fiber_main multi_fiber_test_run_inf_cancel(fiber_main_attr) {
    try {
        for (;;) {
            lwt_cancellation_point();
            lwt_yield();
        }
    } catch (exception_canceled, e) {}
}

fiber_main multi_fiber_test_run_inf_join(fiber_main_attr) {
    try {
        ifc_fiber_sync_send(lwt_get_fiber_id());
        atest(false);
    } catch (exception_canceled, e) {}
}

join_locked(void) multi_fiber_test_join_race_inf_join(join_server_params, bool* out_entered) {
    try {
        *out_entered = true;
        ifc_fiber_sync_send(lwt_get_fiber_id());
        atest(false);
    } catch (exception_inner_join_fail, e) {
        atest(false);
    }
}

fiber_main multi_fiber_test_join_race_fiber0(fiber_main_attr, bool* out_entered) {
    try {
        accept_join(multi_fiber_test_join_race_inf_join, join_server_params, out_entered);
        atest(false);
    } catch (exception_canceled, e) {}
}

fiber_main multi_fiber_test_join_race_fiber1(fiber_main_attr, rcd_fid_t fiber_id) {
    try {
        multi_fiber_test_join_race_inf_join(fiber_id);
        atest(false);
    } catch (exception_inner_join_fail, e) {}
}

fiber_main multi_fiber_test_join_race2_fiber0(fiber_main_attr, rcd_fid_t final_join_fid, bool* out_entered) {
    if (final_join_fid != 0) {
        try {
            multi_fiber_test_join_race_inf_join(final_join_fid);
        } catch (exception_inner_join_fail, e) {}
    } else {
        accept_join(multi_fiber_test_join_race_inf_join, join_server_params, out_entered);
    }
}

join_locked(void) multi_fiber_test_join_race2_inf_join(rcd_fid_t final_join_fid, join_server_params, bool* out_entered) {
    if (final_join_fid != 0) {
        try {
            multi_fiber_test_join_race_inf_join(final_join_fid);
        } catch (exception_inner_join_fail, e) {
            atest(false);
        }
    } else {
        accept_join(multi_fiber_test_join_race_inf_join, join_server_params, out_entered);
    }
}

fiber_main multi_fiber_test_join_race2_fiber1(fiber_main_attr, bool* out_entered) {
    try {
        accept_join(multi_fiber_test_join_race2_inf_join, join_server_params, out_entered);
        atest(false);
    } catch (exception_canceled, e) {}
}

fiber_main multi_fiber_test_join_race2_fiber2(fiber_main_attr, rcd_fid_t final_join_fid, rcd_fid_t fiber1_id) {
    try {
        multi_fiber_test_join_race2_inf_join(final_join_fid, fiber1_id);
        atest(false);
    } catch (exception_inner_join_fail, e) {}
}


join_locked(void) multi_fiber_test_pool_do_work(int32_t client_counter, join_server_params, int32_t* worker_counter) {
    // This will fail miserably if ifc fails to lock one worker per client which enable us to detect that problem.
    int32_t worker_counter_v = *worker_counter;
    rio_wait(RIO_NS_MS);
    *worker_counter = (worker_counter_v + client_counter);
}

join_shared(void) multi_fiber_test_pool_accept_worker(int32_t* worker_counter, join_server_params) {
    server_accept_join(multi_fiber_test_pool_do_work, join_server_params, worker_counter);
}

fiber_main multi_fiber_test_pool_fiber(fiber_main_attr) {
    try {
        auto_accept_join(multi_fiber_test_pool_accept_worker, join_server_params);
    } catch (exception_canceled, e) {}
}

fiber_main multi_fiber_test_pool_worker_fiber(fiber_main_attr, rcd_fid_t pool_fid, int32_t* worker_counter) {
    try {
        for (;;) {
            multi_fiber_test_pool_accept_worker(worker_counter, pool_fid);
            rio_wait(RIO_NS_MS);
        }
    } catch (exception_inner_join_fail, e) {}
}

fiber_main multi_fiber_test_pool_client_fiber(fiber_main_attr, rcd_fid_t pool_fid, int32_t client_counter) {
    multi_fiber_test_pool_do_work(client_counter, pool_fid);
}

join_locked(void) multi_fiber_test_pool_upjoined(rcd_fid_t pool_fid, join_server_params, int32_t* shared_client_counter) {
    atest(*shared_client_counter >= 0);
    int32_t client_counter = *shared_client_counter;
    *shared_client_counter = -1;
    multi_fiber_test_pool_do_work(client_counter, pool_fid);
    atest(*shared_client_counter == -1);
    *shared_client_counter = client_counter + 1;
}

fiber_main multi_fiber_test_pool_upserver_fiber(fiber_main_attr, int32_t client_counter) {
    int32_t shared_client_counter = client_counter, prev_shared_client_counter = client_counter - 1;
    try {
        for (;;) {
            atest(shared_client_counter == prev_shared_client_counter + 1);
            prev_shared_client_counter = shared_client_counter;
            accept_join(multi_fiber_test_pool_upjoined, join_server_params, &shared_client_counter);
        }
    } catch (exception_canceled, e) {}
}

fiber_main multi_fiber_test_pool_upclient_fiber(fiber_main_attr, rcd_fid_t pool_fid, rcd_fid_t upserver_fid) {
    multi_fiber_test_pool_upjoined(pool_fid, upserver_fid);
}

void rcd_self_test_multi_fiber() {
    sub_heap {
        const int total_fibers = 2000;
        // Try starting a large number of concurrent fibers and do a really simple operation on each of them. (Test fiber startup overhead.)
        rcd_fid_t* fids = lwt_alloc_new(sizeof(rcd_fid_t) * total_fibers);
        for (int i = 0; i < total_fibers; i++) {
            fmitosis {
                int* test_number = new(int);
                *test_number = i;
                fids[i] = spawn_static_fiber(multi_fiber_test_main("", test_number));
            }
        }
        // Read the test number from them.
        for (int i = 0; i < total_fibers; i++) {
            int test_number = multi_fiber_test_get_number(i, fids[i]);
            atest(test_number == i * 2);
        }
    }
    // Try to call a non existing fiber and expect join/race exception.
    try {
        multi_fiber_test_get_number(1, ULONG_MAX);
        atest(false);
    } catch (exception_inner_join_fail, e) {}
    // Create fiber and try to run sync with it to see if run sync works (this is normally an anti-pattern as we are blindly sharing data between threads outside a joined context.)
    {
        int output = 0;
        rcd_fid_t child_fid;
        fmitosis {
            child_fid = spawn_static_fiber(multi_fiber_test_run_sync("", lwt_get_fiber_id(), &output));
        }
        ifc_fiber_sync_send(child_fid);
        atest(output == 100000);
    }
    // Test cancellation of non existing fiber id.
    lwt_cancel_fiber_id(ULONG_MAX);
    // Test basic cancellation + join failure.
    {
        rcd_fid_t child_fid;
        fmitosis {
            child_fid = spawn_static_fiber(multi_fiber_test_run_inf_cancel(""));
        }
        lwt_cancel_fiber_id(child_fid);
        try {
            ifc_fiber_sync_send(child_fid);
            atest(false);
        } catch (exception_inner_join_fail, e) {}
    }
    // Test cancellation of fiber that is infinitely waiting for join.
    {
        rcd_fid_t child_fid;
        fmitosis {
            child_fid = spawn_static_fiber(multi_fiber_test_run_inf_join(""));
        }
        lwt_yield();
        lwt_cancel_fiber_id(child_fid);
        try {
            ifc_fiber_sync_send(child_fid);
            atest(false);
        } catch(exception_inner_join_fail, e) {}
    }
    // Test that we can properly separate inner and outer join races.
    {
        bool entered0 = false;
        bool entered2a = false;
        bool entered2b = false;
        rcd_fid_t child0_fid0, child0_fid1, child2a_fid0, child2a_fid1, child2a_fid2, child2b_fid0, child2b_fid1, child2b_fid2;
        fmitosis {
            child0_fid0 = spawn_static_fiber(multi_fiber_test_join_race_fiber0("", &entered0));
        }
        fmitosis {
            child0_fid1 = spawn_static_fiber(multi_fiber_test_join_race_fiber1("", child0_fid0));
        }
        fmitosis {
            child2a_fid0 = spawn_static_fiber(multi_fiber_test_join_race2_fiber0("", 0, &entered2a));
        }
        fmitosis {
            child2a_fid1 = spawn_static_fiber(multi_fiber_test_join_race2_fiber1("", 0));
        }
        fmitosis {
            child2a_fid2 = spawn_static_fiber(multi_fiber_test_join_race2_fiber2("", child2a_fid0, child2a_fid1));
        }
        fmitosis {
            child2b_fid1 = spawn_static_fiber(multi_fiber_test_join_race2_fiber1("", &entered2b));
        }
        fmitosis {
            child2b_fid2 = spawn_static_fiber(multi_fiber_test_join_race2_fiber2("", 0, child2b_fid1));
        }
        fmitosis {
            child2b_fid0 = spawn_static_fiber(multi_fiber_test_join_race2_fiber0("", child2b_fid2, 0));
        }
        while (!entered0) {
            lwt_yield();
            sync_synchronize();
        }
        while (!entered2a) {
            lwt_yield();
            sync_synchronize();
        }
        while (!entered2b) {
            lwt_yield();
            sync_synchronize();
        }
        lwt_cancel_fiber_id(child0_fid0);
        try {
            ifc_fiber_sync_send(child0_fid0);
            atest(false);
        } catch(exception_join_race, e) {
            // Testing lwt_test_live_join_race() here which should make the flag have the same effect as exception_inner_join_race.
            lwt_test_live_join_race();
        }
        lwt_cancel_fiber_id(child2a_fid1);
        lwt_cancel_fiber_id(child2b_fid1);
#define WAIT_FOR_FIBER_END(fid) \
        try { \
            ifc_fiber_sync_send(fid); \
            atest(false); \
        } catch(exception_inner_join_fail, e) {}
        WAIT_FOR_FIBER_END(child2a_fid0);
        WAIT_FOR_FIBER_END(child2a_fid1);
        WAIT_FOR_FIBER_END(child2a_fid2);
        WAIT_FOR_FIBER_END(child2b_fid0);
        WAIT_FOR_FIBER_END(child2b_fid1);
        WAIT_FOR_FIBER_END(child2b_fid2);
    }
    // Test pool pattern.
    sub_heap {
        // Create pool fiber. This fiber is just used as a semaphore symbol.
        rcd_fid_t pool_fid;
        fmitosis {
            pool_fid = spawn_static_fiber(multi_fiber_test_pool_fiber(""));
        }
        // Create a fixed number of workers that act as the servers. They are anonymous as they are joined via the semaphore.
        rcd_fid_t worker_fids[8];
        int32_t worker_counters[8];
        for (size_t i = 0; i < LENGTHOF(worker_counters); i++) {
            worker_counters[i] = 0;
            fmitosis {
                worker_fids[i] = spawn_static_fiber(multi_fiber_test_pool_worker_fiber("", pool_fid, &worker_counters[i]));
            }
        }
        // We also test that we can also do server side concurrency inside normal concurrency. Create an "upserver" that will synchronize requests for "upjoined" joins.
        const int total_clients = 800;
        rcd_fid_t upserver_fid;
        fmitosis {
            upserver_fid = spawn_static_fiber(multi_fiber_test_pool_upserver_fiber("", total_clients / 2));
        }
        // Try starting a large number of concurrent client fibers. This will ensure that all workers should be used.
        size_t expected_total_worker_count = 0;
        rcd_fid_t* client_fids = lwt_alloc_new(sizeof(rcd_fid_t) * total_clients);
        for (int i = 0; i < total_clients / 2; i++) {
            fmitosis {
                client_fids[i] = spawn_static_fiber(multi_fiber_test_pool_client_fiber("", pool_fid, i));
            }
            expected_total_worker_count += i;
        }
        // Also create a large number of upjoined clients that need to join with the workers by first joining with the "upserver".
        for (int i = total_clients / 2; i < total_clients; i++) {
            fmitosis {
                client_fids[i] = spawn_static_fiber(multi_fiber_test_pool_upclient_fiber("", pool_fid, upserver_fid));
            }
            expected_total_worker_count += i;
        }
        // Wait for all clients to complete.
        for (int i = 0; i < total_clients; i++)
            ifc_wait(client_fids[i]);
        // Check that all worker counters was used and that the total is what we expect.
        size_t total_worker_count = 0;
        for (size_t i = 0; i < LENGTHOF(worker_counters); i++) {
            improb_atest(worker_counters[i] != 0, "pool worker #", ui2fs(i), " did not process any work at all");
            total_worker_count += worker_counters[i];
        }
        atest(total_worker_count == expected_total_worker_count);
    }
}
