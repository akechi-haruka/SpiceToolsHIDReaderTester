#include "eamuse.h"
#include <fstream>
#include <thread>
#include "logging.h"
#include "utils.h"


// state
static bool CARD_INSERT[2] = {false, false};
static char CARD_INSERT_UID[2][8];
static char CARD_INSERT_UID_ENABLE[2] = {false, false};
static int COIN_STOCK = 0;
static bool COIN_BLOCK = false;
static std::thread *COIN_INPUT_THREAD;
static bool COIN_INPUT_THREAD_ACTIVE = false;
static uint16_t KEYPAD_STATE[] = {0, 0};
static std::string EAMUSE_GAME_NAME;

bool eamuse_get_card(int active_count, int unit_id, char *card) {

    // get unit index
    int index = unit_id > 0 && active_count > 1 ? 1 : 0;

    // reader card input
    if (CARD_INSERT_UID_ENABLE[index]) {
        CARD_INSERT_UID_ENABLE[index] = false;
        memcpy(card, CARD_INSERT_UID[index], 8);
        log_info("eamuse", "Inserted card from reader " + to_string(index) + ": " + bin2hex(card, 8));
        return true;
    }

    // get file path
    std::string path;
    path = index > 0 ? "card1.txt" : "card0.txt";

    // open file
    std::ifstream f(path);
    if (!f) {
        log_warning("eamuse", path + " can not be opened!");
        return false;
    }

    // get size
    f.seekg(0, f.end);
    auto length = (size_t) f.tellg();
    f.seekg(0, f.beg);

    // check size
    if (length < 16) {
        log_warning("eamuse", path + " is too small (must be at least 16 characters)");
        return false;
    }

    // read file
    char buffer[17];
    f.read(buffer, 16);
    buffer[16] = 0;

    // verify card
    for (int n = 0; n < 16; n++) {
        char c = buffer[n];
        bool digit = c >= '0' && c <= '9';
        bool character_big = c >= 'A' && c <= 'F';
        bool character_small = c >= 'a' && c <= 'f';
        if (!digit && !character_big && !character_small) {
            log_warning("eamuse", path + " contains an invalid character sequence at byte " +
                        to_string(n) + " (16 characters, 0-9/A-F only)");
            return false;
        }
    }

    // info
    log_info("eamuse", "Inserted " + path + ": " + to_string(buffer));

    // convert hex to bytes
    hex2bin(buffer, card);

    return true;
}

void eamuse_card_insert(int unit) {
    CARD_INSERT[unit] = true;
}

void eamuse_card_insert(int unit, const char *card) {
	log_info("eamuse", "Inserted a card into unit " + to_string(unit));
	memcpy(CARD_INSERT_UID[unit], card, 8);
    CARD_INSERT[unit] = true;
    CARD_INSERT_UID_ENABLE[unit] = true;
}

bool eamuse_card_insert_consume(int active_count, int unit_id) {

    // get unit index
    int index = unit_id > 0 && active_count > 1 ? 1 : 0;

    // check for card insert
    if (!CARD_INSERT[index])
        return false;
	log_info("eamuse", "Card insert on " + to_string(unit_id) + "/" + to_string(active_count));
	CARD_INSERT[index] = false;
    return true;
}

void eamuse_coin_set_block(bool block) {
    COIN_BLOCK = block;
}

int eamuse_coin_get_stock() {
    return COIN_STOCK;
}

int eamuse_coin_consume_stock() {
    int stock = COIN_STOCK;
    COIN_STOCK = 0;
    return stock;
}

int eamuse_coin_add() {
    return ++COIN_STOCK;
}

void eamuse_coin_start_thread() {

    // set active
    COIN_INPUT_THREAD_ACTIVE = true;

    // create thread
    COIN_INPUT_THREAD = new std::thread([]() {
        static int COIN_INPUT_KEY = VK_F1;
        static bool COIN_INPUT_KEY_STATE = false;
        while (COIN_INPUT_THREAD_ACTIVE) {

            // check input key
            if (GetAsyncKeyState(COIN_INPUT_KEY)) {
                if (!COIN_INPUT_KEY_STATE) {
                    if (COIN_BLOCK)
                        log_info("eamuse", "Coin inserted while blocked.");
                    else {
                        log_info("eamuse", "Coin insert.");
                        COIN_STOCK++;
                    }
                }
                COIN_INPUT_KEY_STATE = true;
            } else
                COIN_INPUT_KEY_STATE = false;

            // once every frame
            Sleep(1000 / 60);
        }
    });
}

void eamuse_coin_stop_thread() {
    COIN_INPUT_THREAD_ACTIVE = false;
    COIN_INPUT_THREAD->join();
    delete COIN_INPUT_THREAD;
    COIN_INPUT_THREAD = nullptr;
}

void eamuse_set_keypad_state(int unit, uint16_t keypad_state) {
    KEYPAD_STATE[unit] = keypad_state;
}

uint16_t eamuse_get_keypad_state(int unit) {

    // reset
    eamuse_set_keypad_state(unit, 0);

    // return state
    return KEYPAD_STATE[unit];
}

bool eamuse_keypad_state_naive() {
    return false;
}

void eamuse_set_game(std::string game) {
}

std::string eamuse_get_game() {
    return EAMUSE_GAME_NAME;
}

void eamuse_autodetect_game() {
        log_warning("eamuse", "Unknown game model: ");
}

void eamuse_scard_callback(uint8_t slot_no, cardinfo_t *cardinfo) {
    if (cardinfo->card_type > 0)
        eamuse_card_insert(slot_no, (const char*) cardinfo->uid);
}
