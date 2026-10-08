#pragma once

#include "Aqsh.h"

#include <string>

struct AqEnvKeys {
    std::string dev;
    std::string usr;
};

AqEnvKeys env_keys(AqVersion version);

void set_env_keys(AqVersion version, std::string dev_key, std::string usr_key);
void clear_env_keys();
