#pragma once

enum class POLYGON_MODE {
    FILL,
    LINE
};

enum class CULL_MODE {
    NONE,
    FRONT,
    BACK
};

enum class FRONT_FACE_ORIENTATION {
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

struct RasterizerStateInfo {
    POLYGON_MODE PolygonMode;
    CULL_MODE CullMode;
    FRONT_FACE_ORIENTATION FrontFaceOrientation;
    bool DepthBiasEnable;
    float DepthBiasConstantFactor;
    float DepthBiasClamp;
    float DepthBiasSlopeFactor;
    float LineWidth;
};
