#pragma once

#include <elc/pipeline/artifact.h>
#include <elc/pipeline/context.h>

typedef enum ElcObserverEvent {
    ELC_OBS_START,  ///< Triggered before the stage begins
    ELC_OBS_FINISH, ///< Triggered right after the stage execution ends; for time-sensitive observers (timers, metrics, etc.)
    ELC_OBS_END,    ///< Triggered after the FINISH event; for non-time-critical observers (dumps, validators, etc.)
} ElcObserverEvent;

typedef void ElcObserverFn(
    void* user_data,
    const ElcPipelineContext* ctx,
    ElcObserverEvent event,
    ElStringView stage_name,
    const ElcArtifact* artifact
);

typedef struct ElcObserver {
    ElcObserverFn* callback;
    void* user_data;
} ElcObserver;
