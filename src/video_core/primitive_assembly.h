// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include "video_core/regs_pipeline.h"

namespace Pica {

/*
 * Utility class to build triangles from a series of vertices,
 * according to a given triangle topology.
 */
template <typename VertexType>
struct PrimitiveAssembler {
    PrimitiveAssembler(
        PipelineRegs::TriangleTopology topology = PipelineRegs::TriangleTopology::List);

    /*
     * Queues a vertex, builds primitives from the vertex queue according to the given
     * triangle topology.
     */
    void SubmitVertex(const VertexType& vtx);

    /**
     * Invert the vertex order of the next triangle. Called by geometry shader emitter.
     * This only takes effect for TriangleTopology::Shader.
     */
    void SetWinding();

    /**
     * Resets the internal state of the PrimitiveAssembler.
     */
    void Reset();

    /**
     * Reconfigures the PrimitiveAssembler to use a different triangle topology.
     */
    void Reconfigure(PipelineRegs::TriangleTopology topology);

    /**
     * Returns whether the PrimitiveAssembler has an empty internal buffer.
     */
    bool IsEmpty() const;

    /**
     * Returns the current topology.
     */
    PipelineRegs::TriangleTopology GetTopology() const;

private:
    PipelineRegs::TriangleTopology topology;

    int buffer_index{};
    VertexType buffer[2];
    bool strip_ready{};
    bool winding{};
};

} // namespace Pica
