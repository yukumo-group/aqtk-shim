#include "env_keys.h"

#include <mutex>
#include <utility>

namespace {
    enum {
        SLOT_AQ1 = 0,
        SLOT_AQ10,
        SLOT_COUNT
    };

    struct Store {
        std::mutex mu;
        AqEnvKeys keys[SLOT_COUNT];
    };

    Store& store() {
        static Store s;
        return s;
    }

    // Only AquesTalk1 and AquesTalk10 export SetDevKey / SetUsrKey.
    int slot(AqVersion version) {
        switch (version) {
        case AQ_VER_1: return SLOT_AQ1;
        case AQ_VER_10: return SLOT_AQ10;
        default: return -1;
        }
    }
}

AqEnvKeys env_keys(AqVersion version) {
    Store& s = store();
    std::lock_guard lock(s.mu);
    const int i = slot(version);
    return i < 0 ? AqEnvKeys{} : s.keys[i];
}

void set_env_keys(AqVersion version, std::string dev_key, std::string usr_key) {
    Store& s = store();
    std::lock_guard lock(s.mu);
    const int i = slot(version);
    if (i < 0) {
        return;
    }
    s.keys[i].dev = std::move(dev_key);
    s.keys[i].usr = std::move(usr_key);
}

void clear_env_keys() {
    Store& s = store();
    std::lock_guard lock(s.mu);
    for (AqEnvKeys& keys : s.keys) {
        keys = AqEnvKeys{};
    }
}
