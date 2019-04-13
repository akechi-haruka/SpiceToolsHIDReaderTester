#ifndef SPICETOOLS_MISC_EAMUSE_H
#define SPICETOOLS_MISC_EAMUSE_H

#include <cstdint>
#include <string>

/* From BT5 API for compatibility reasons.
   Scan codes for the so-called "10 key" button panel on each card reader. Each
   scan code corresponds to a bit position within the 16-bit bitfield that you
   return from eam_io_get_keypad_state(). */
enum eam_io_keypad_scan_code {
    EAM_IO_KEYPAD_0        = 0,
    EAM_IO_KEYPAD_1        = 1,
    EAM_IO_KEYPAD_4        = 2,
    EAM_IO_KEYPAD_7        = 3,
    EAM_IO_KEYPAD_00       = 4,
    EAM_IO_KEYPAD_2        = 5,
    EAM_IO_KEYPAD_5        = 6,
    EAM_IO_KEYPAD_8        = 7,
    EAM_IO_KEYPAD_DECIMAL  = 8,
    EAM_IO_KEYPAD_3        = 9,
    EAM_IO_KEYPAD_6        = 10,
    EAM_IO_KEYPAD_9        = 11,

    EAM_IO_KEYPAD_COUNT    = 12, /* Not an actual scan code */

    EAM_IO_INSERT = 13, /* SpiceTools Extension */
};

typedef struct {
	int card_type = 0;
	uint8_t uid[8];
} cardinfo_t;

bool eamuse_get_card(int active_count, int unit_id, char *card);

void eamuse_card_insert(int unit);

void eamuse_card_insert(int unit, const char *card);

bool eamuse_card_insert_consume(int active_count, int unit_id);

void eamuse_coin_set_block(bool block);

int eamuse_coin_get_stock();

int eamuse_coin_add();

int eamuse_coin_consume_stock();

void eamuse_coin_start_thread();

void eamuse_coin_stop_thread();

void eamuse_set_keypad_state(int unit, uint16_t keypad_state);

uint16_t eamuse_get_keypad_state(int unit);

bool eamuse_keypad_state_naive();

void eamuse_set_game(std::string game);

std::string eamuse_get_game();

void eamuse_autodetect_game();

void eamuse_scard_callback(uint8_t slot_no, cardinfo_t *cardinfo);

#endif //SPICETOOLS_MISC_EAMUSE_H
