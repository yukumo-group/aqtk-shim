#include "wav_track.h"

#include <mutex>
#include <unordered_map>

namespace {
    struct WavTable {
        std::mutex mu;
        std::unordered_map<unsigned char*, WavFreeFn> owners;
    };

    WavTable& wav_table() {
        static WavTable table;
        return table;
    }
}

void wav_track(unsigned char* wav, const WavFreeFn free_fn) {
    std::lock_guard lock(wav_table().mu);
    wav_table().owners.emplace(wav, free_fn);
}

void wav_release(unsigned char* wav) {
    if (!wav) {
        return;
    }

    WavFreeFn free_fn = nullptr;
    {
        std::lock_guard lock(wav_table().mu);
        const auto it = wav_table().owners.find(wav);
        if (it == wav_table().owners.end()) {
            return;
        }
        free_fn = it->second;
        wav_table().owners.erase(it);
    }
    free_fn(wav);
}

void wav_release_all() {
    std::unordered_map<unsigned char*, WavFreeFn> pending;
    {
        std::lock_guard lock(wav_table().mu);
        pending.swap(wav_table().owners);
    }
    for (const auto& [wav, free_fn] : pending) {
        free_fn(wav);
    }
}
