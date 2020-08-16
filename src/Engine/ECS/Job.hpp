#pragma once

const size_t g_PhaseCount = 2;
const size_t g_LastPhase = g_PhaseCount - 1;

struct Job {
    std::function<void()> Function;
    std::vector<ComponentAccess> Components;
};
