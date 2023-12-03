/* #ifndef EKONYV_STATE_H
#define EKONYV_STATE_H

#include "../storage/database.h"
#include "../types/fixedbuffer.h"

struct State {
    FixedBuffer<16> id;
    uint64_t timestamp;
};

class SessionDatabase {
public:
    Database<Session, 4> db;

    struct SessionInfo {
        bool valid;
        bool expired;
        uint32_t index;
        uint16_t user_id;
    };

public:
    SessionDatabase();
    void load();
    void save();

    Session start(uint16_t user_id);
    Session extend(uint32_t n, uint16_t user_id);
    void discard(uint32_t id);
    void discardAllForUser(uint16_t user_id);

    void update();

    SessionInfo check(const FixedBuffer<16> &token);
    SessionInfo checkRefresh(const FixedBuffer<16> &token);
};

#endif // !defined(EKONYV_STATE_H) */