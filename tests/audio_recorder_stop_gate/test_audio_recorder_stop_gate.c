#include "audio_recorder_stop_gate.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

static audio_recorder_stop_gate_t racing_gate;
static unsigned phase, done;
static bool retired;
static void *race_producer(void *unused)
{
    (void)unused;
    for (unsigned round = 1; round <= 20000; ++round) {
        while (__atomic_load_n(&phase, __ATOMIC_ACQUIRE) < round) {}
        if (audio_recorder_stop_gate_try_enter(&racing_gate)) {
            assert(!__atomic_load_n(&retired, __ATOMIC_ACQUIRE));
            audio_recorder_stop_gate_leave(&racing_gate);
        }
        __atomic_store_n(&done, round, __ATOMIC_RELEASE);
    }
    return NULL;
}
static void test_close_admission_race(void)
{
    pthread_t producer;
    audio_recorder_stop_gate_init(&racing_gate);
    assert(!pthread_create(&producer, NULL, race_producer, NULL));
    for (unsigned round = 1; round <= 20000; ++round) {
        __atomic_store_n(&retired, false, __ATOMIC_RELEASE);
        assert(audio_recorder_stop_gate_open(&racing_gate));
        __atomic_store_n(&phase, round, __ATOMIC_RELEASE);
        audio_recorder_stop_gate_close(&racing_gate);
        while (!audio_recorder_stop_gate_is_quiescent(&racing_gate)) {}
        __atomic_store_n(&retired, true, __ATOMIC_RELEASE);
        while (__atomic_load_n(&done, __ATOMIC_ACQUIRE) < round) {}
    }
    assert(!pthread_join(producer, NULL));
}

static int failures;
static unsigned s_checks;
#define CHECK(x) do { s_checks++; if (!(x)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); failures++; } } while (0)

int main(void)
{
    test_close_admission_race();
    audio_recorder_stop_gate_t gate;
    audio_recorder_stop_gate_init(&gate);
    CHECK(audio_recorder_stop_gate_is_quiescent(&gate));
    CHECK(!audio_recorder_stop_gate_try_enter(&gate));
    CHECK(audio_recorder_stop_gate_open(&gate));
    CHECK(audio_recorder_stop_gate_try_enter(&gate));
    CHECK(audio_recorder_stop_gate_active(&gate) == 1u);

    /* STOP closes admission before waiting for the producer already in flight. */
    audio_recorder_stop_gate_close(&gate);
    CHECK(!audio_recorder_stop_gate_try_enter(&gate));
    CHECK(!audio_recorder_stop_gate_is_quiescent(&gate));
    audio_recorder_stop_gate_leave(&gate);
    CHECK(audio_recorder_stop_gate_is_quiescent(&gate));

    CHECK(audio_recorder_stop_gate_open(&gate));
    CHECK(audio_recorder_stop_gate_try_enter(&gate));
    audio_recorder_stop_gate_leave(&gate);
    audio_recorder_stop_gate_close(&gate);
    CHECK(audio_recorder_stop_gate_is_quiescent(&gate));

    /* An unpaired leave must not wrap the producer count. If it did, the gate
     * would report ~4 billion active producers and STOP would wait forever for a
     * drain that can never happen. */
    audio_recorder_stop_gate_leave(&gate);
    CHECK(audio_recorder_stop_gate_active(&gate) == 0u);
    CHECK(audio_recorder_stop_gate_is_quiescent(&gate));

    /* And the gate is still usable afterwards. */
    CHECK(audio_recorder_stop_gate_open(&gate));
    CHECK(audio_recorder_stop_gate_try_enter(&gate));
    CHECK(audio_recorder_stop_gate_active(&gate) == 1u);
    audio_recorder_stop_gate_leave(&gate);
    CHECK(audio_recorder_stop_gate_is_quiescent(&gate));

    printf("TESTS_RUN=%u\n", s_checks);
    if (failures == 0) puts("audio_recorder_stop_gate tests passed");
    return failures ? 1 : 0;
}
