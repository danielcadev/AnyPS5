#include "prx/libSceAgcDriver/Execution/include/VulkanDevice.hpp"
#include "prx/libSceAgcDriver/Graphics/include/Draw.hpp"
#include "Recompiler.hpp"
#include "VulkanTestDevice.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdio>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace {

using AgcDriver::Graphics::Require;
using ShaderRecompiler::ShaderStage;

constexpr std::uint32_t Vertices = 64;
constexpr std::uint32_t Words = 8;
constexpr std::uint32_t Sentinel = 0xdeadbeefu;
alignas(256) std::array<std::array<float, 4>, Vertices> Positions{};
alignas(256) std::array<std::uint32_t, Vertices * Words> Output{};
alignas(256) std::array<std::byte, 64 * 16 * 4> Pixels{};

// s[0:3] is the output descriptor; s[4:7] is the position descriptor.
// Snapshot entry EXEC, overwrite EXEC with -1, then snapshot again. A separate
// literal -1 in an ordinary SGPR must retain every bit. Finally snapshot a
// sparse guest EXEC, restore EXEC, and store all four pairs for each vertex.
alignas(256) constexpr std::uint32_t VertexCode[]{
    0xbe88047e,                         // s_mov_b64 s[8:9], exec
    0xbefe04c1,                         // s_mov_b64 exec, -1
    0xbe8a047e,                         // s_mov_b64 s[10:11], exec
    0xbe8c04c1,                         // s_mov_b64 s[12:13], -1
    0xbe9003ff, 0xaaaaaaaa,             // s_mov_b32 s16, 0xaaaaaaaa
    0xbe9103ff, 0xaaaaaaaa,             // s_mov_b32 s17, 0xaaaaaaaa
    0xbefe0410,                         // s_mov_b64 exec, s[16:17]
    0xbe8e047e,                         // s_mov_b64 s[14:15], exec
    0xbefe04c1,                         // s_mov_b64 exec, -1
    0x7e140208, 0x7e160209,             // v_mov_b32 v10/v11, s8/s9
    0x7e18020a, 0x7e1a020b,             // v_mov_b32 v12/v13, s10/s11
    0x7e1c020c, 0x7e1e020d,             // v_mov_b32 v14/v15, s12/s13
    0x7e20020e, 0x7e22020f,             // v_mov_b32 v16/v17, s14/s15
    0xe0702000, 0x80000a05,             // buffer_store_dword v10, v5, s[0:3], 0
    0xe0702004, 0x80000b05,
    0xe0702008, 0x80000c05,
    0xe070200c, 0x80000d05,
    0xe0702010, 0x80000e05,
    0xe0702014, 0x80000f05,
    0xe0702018, 0x80001005,
    0xe070201c, 0x80001105,
    0xe0382000, 0x80010005,             // buffer_load_format_xyzw v[0:3], v5, s[4:7], 0
    0xbf8c3f70,
    0xf80008cf, 0x03020100,             // exp pos0 v0,v1,v2,v3 done
    0xbf810000,
};

alignas(256) constexpr std::uint32_t PixelCode[]{0x7e0e02f2, 0xf800180f, 0x07070707, 0xbf810000};

std::array<std::uint32_t, 4> Descriptor(const void* data, std::uint32_t stride, std::uint32_t count) {
    const auto address = reinterpret_cast<std::uintptr_t>(data);
    return {static_cast<std::uint32_t>(address), static_cast<std::uint32_t>((address >> 32u) & 0xffffu) | (stride << 16u), count, 0x01016facu};
}

void Run(AgcDriver::VulkanDevice& device, std::uint32_t count) {
    Output.fill(Sentinel);
    Pixels.fill(std::byte{0});
    for (std::uint32_t i = 0; i < Vertices; ++i) {
        Positions[i] = {i % 3u == 1u ? 0.75f : -0.75f, i % 3u == 2u ? 0.75f : -0.75f, 0.5f, 1.0f};
    }
    std::vector<std::uint32_t> userData(8);
    const auto output = Descriptor(Output.data(), Words * 4u, Vertices);
    const auto positions = Descriptor(Positions.data(), 16u, Vertices);
    std::copy(output.begin(), output.end(), userData.begin());
    std::copy(positions.begin(), positions.end(), userData.begin() + 4);
    const std::array<ShaderRecompiler::MemoryRegion, 1> memory{{{reinterpret_cast<std::uintptr_t>(VertexCode), std::as_bytes(std::span(VertexCode))}}};
    ShaderRecompiler::RecompileRequest vertex{
        {ShaderStage::Vertex, reinterpret_cast<std::uintptr_t>(VertexCode), VertexCode, 0, {}},
        {64u, 0, userData, std::nullopt, std::nullopt, ShaderRecompiler::ShaderVertexStageInfo{}, memory},
        device.Target(), {0, 0, 0, 64}
    };
    vertex.useCache = false;
    const auto vs = ShaderRecompiler::Recompile(vertex);
    const auto offset = static_cast<std::uint32_t>(vs.pushConstants.size());
    ShaderRecompiler::ShaderPixelStageInfo pixel{};
    pixel.targetOutputMode[0] = 9;
    pixel.targetExportMapping.fill(0xe4u);
    const std::vector<std::uint32_t> pixelData;
    const std::array<ShaderRecompiler::MemoryRegion, 1> pixelMemory{{{reinterpret_cast<std::uintptr_t>(PixelCode), std::as_bytes(std::span(PixelCode))}}};
    ShaderRecompiler::RecompileRequest fragment{
        {ShaderStage::Fragment, reinterpret_cast<std::uintptr_t>(PixelCode), PixelCode, 0, {}},
        {64u, 0, pixelData, std::nullopt, pixel, std::nullopt, pixelMemory},
        device.Target(), {0, 0, offset, 128u - offset}
    };
    fragment.useCache = false;
    const auto fs = ShaderRecompiler::Recompile(fragment);
    const std::array<AgcDriver::Graphics::CompiledShader, 2> shaders{{{ShaderStage::Vertex, &vs, 0}, {ShaderStage::Fragment, &fs, offset}}};
    AgcDriver::Graphics::State state{};
    state.stages = {AgcDriver::Graphics::ShaderPath::Vertex, 0u, 64u, 64u, std::nullopt, std::nullopt};
    state.color = {reinterpret_cast<std::uintptr_t>(Pixels.data()), {64, 16}, VK_FORMAT_R8G8B8A8_UNORM, Pixels.size(), 0xe4u};
    state.colors = {state.color};
    state.hasColorTarget = true;
    state.renderExtent = {64, 16};
    state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    state.viewport = {0, 16, 64, -16, 0, 1};
    state.scissor = {{0, 0}, {64, 16}};
    state.cullMode = VK_CULL_MODE_NONE;
    state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    state.blend.colorWriteMask = 15;
    state.blends = {state.blend};
    const AgcDriver::Pm4::DrawParameters draw{0, count, 0, 1, 0, false};
    device.Draw(state, draw, shaders);
    device.WaitIdle();

    for (std::uint32_t i = 0; i < Vertices; ++i) {
        const auto* row = Output.data() + i * Words;
        const auto label = "vertex EXEC copy, draw=" + std::to_string(count) + " vertex=" + std::to_string(i);
        if (i >= count) {
            Require(std::all_of(row, row + Words, [](auto value) { return value == Sentinel; }), label + ": an unissued vertex wrote output");
            continue;
        }
        const std::uint64_t initial = row[0] | (static_cast<std::uint64_t>(row[1]) << 32u);
        const std::uint64_t copied = row[2] | (static_cast<std::uint64_t>(row[3]) << 32u);
        const std::uint64_t sparse = row[6] | (static_cast<std::uint64_t>(row[7]) << 32u);
        Require(initial != 0 && std::popcount(initial) <= count, label + ": missing or invalid entry ballot");
        std::printf("draw=%u vertex=%u entry=%016llx copied=%016llx sparse=%016llx literal=%08x%08x\n", count, i, static_cast<unsigned long long>(initial), static_cast<unsigned long long>(copied), static_cast<unsigned long long>(sparse), row[5], row[4]);
        Require(row[4] == 0xffffffffu && row[5] == 0xffffffffu, label + ": ordinary SGPR literal -1 changed");
        Require(copied == initial, label + ": EXEC copied after -1 includes inactive invocations");
        Require(sparse == (initial & 0xaaaaaaaaaaaaaaaaull), label + ": sparse guest EXEC includes inactive invocations");
    }
}

}

int main() {
    try {
        const auto device = OpenVulkanTestDevice();
        if (!device) return VulkanTestSkipped;
        if (device->Target().subgroupSize != 64u) {
            std::printf("skipped, the vertex EXEC fixture requires a 64-lane subgroup, got %u\n", device->Target().subgroupSize);
            return VulkanTestSkipped;
        }
        Run(*device, 3);
        Run(*device, 64);
        std::puts("vertex EXEC copy tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
