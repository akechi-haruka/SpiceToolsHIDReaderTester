#include "cardio_runner.h"
#include <thread>
#include <mutex>
#include "eamuse.h"
#include "logging.h"
#include "cardio_hid.h"
#include "cardio_window.h"

bool CARDIO_RUNNER_FLIP = false;
static bool CARDIO_RUNNER_INITIALIZED = false;
static std::thread* CARDIO_RUNNER_THREAD = nullptr;
static HWND CARDIO_RUNNER_HWND = NULL;

void cardio_runner_start(bool scan_hid) {

    // initialize
    if (!CARDIO_RUNNER_INITIALIZED) {
        CARDIO_RUNNER_INITIALIZED = true;
        log_info("cardio", "Initializing CARDIO");

        // initialize
        if (!cardio_window_init()) {
            log_warning("cardio", "Couldn't init CARDIO window");
            return;
        }
        if (!cardio_hid_init()) {
            log_warning("cardio", "Couldn't init CARDIO HID");
            return;
        }

		log_info("cardio", "Starting scan");

        // scan HID devices
        if (scan_hid) {
            if (!cardio_hid_scan()) {
                log_warning("cardio", "Couldn't scan for CARDIO devices");
                return;
            }
        }
    }

	log_info("cardio", "Waiting a sec...");
	Sleep(1000);

    // create thread
    if (CARDIO_RUNNER_THREAD == nullptr) {
		log_info("cardio", "Reader thread starting up");
        CARDIO_RUNNER_THREAD = new std::thread([] {

            // create window
            if (CARDIO_RUNNER_HWND == NULL) {
                if ((CARDIO_RUNNER_HWND = cardio_window_create(GetModuleHandle(NULL))) == NULL) {
                    log_warning("cardio", "Couldn't create CARDIO window");
                    return;
                }
            }
			log_info("cardio_run", "CARDIO window open");

            // main loop
            while (CARDIO_RUNNER_HWND != NULL) {

				log_info("cardio_run", "reading...");

                // update window
                cardio_window_update(CARDIO_RUNNER_HWND);

                // update HID devices
                EnterCriticalSection(&CARDIO_HID_CRIT_SECTION);
                for (size_t device_no = 0; device_no < CARDIO_HID_CONTEXTS_LENGTH; device_no++) {
                    auto device = &CARDIO_HID_CONTEXTS[device_no];

                    // get status
					log_info("cardio_run", "polling device "+to_string(device)+"...");
                    auto status = cardio_hid_device_poll(device);
                    if (status == HID_POLL_CARD_READY) {
						log_info("cardio_run", "read ok");

                        // read card
						if (cardio_hid_device_read(device) == HID_CARD_NONE) {
							log_info("cardio_run", "no card found");
							continue;
						}

                        // if card not empty
                        if (*((uint64_t*) &device->usage_value[0]) > 0) {
							
							log_info("cardio_run", "Look at this, we got a card!!");

                            // insert card
                            if (CARDIO_RUNNER_FLIP)
                                eamuse_card_insert((int) (device_no + 1) & 1, (const char*) &device->usage_value[0]);
                            else
                                eamuse_card_insert((int) device_no & 1, (const char*) &device->usage_value[0]);
						} else {
							log_warning("cardio_run", "card data was empty");
						}
                    }
                }
                LeaveCriticalSection(&CARDIO_HID_CRIT_SECTION);

                // slow down
                Sleep(1000);
            }
        });
    }
}

void cardio_runner_stop() {

    // destroy window
    cardio_window_close(CARDIO_RUNNER_HWND);
    CARDIO_RUNNER_HWND = NULL;
    cardio_window_shutdown();

    // destroy thread
    delete CARDIO_RUNNER_THREAD;
    CARDIO_RUNNER_THREAD = nullptr;

    // shutdown HID
    cardio_hid_close();

    // set initialized to false
    CARDIO_RUNNER_INITIALIZED = false;
}
