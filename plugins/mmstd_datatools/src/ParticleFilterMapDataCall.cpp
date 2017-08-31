#include "stdafx.h"
#include "mmstd_datatools/ParticleFilterMapDataCall.h"

using namespace megamol;
using namespace megamol::stdplugin;
using namespace megamol::stdplugin::datatools;

ParticleFilterMapDataCall::ParticleFilterMapDataCall()
        : idx(nullptr), idx_len(0), frameCnt(1), frameID(0) {
    // intentionally empty
}

ParticleFilterMapDataCall::~ParticleFilterMapDataCall() {
    idx = nullptr; // paranoia clean-up
    idx_len = 0;
    frameCnt = 0;
    frameID = 0;
}
