#pragma once

#include "enki/rendering/paint.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace enki {

class RuntimeEffect {
public:
    ~RuntimeEffect();
    RuntimeEffect(const RuntimeEffect&) = delete;
    RuntimeEffect& operator=(const RuntimeEffect&) = delete;

    static std::shared_ptr<RuntimeEffect> compile(std::string_view code, std::string* error_out = nullptr);

    bool hasUniform(std::string_view name) const;

    std::shared_ptr<Shader> makeShader(const void* uniform_data, size_t data_size) const;
    std::shared_ptr<Shader> makeShader(const void* uniform_data, size_t data_size, float tx, float ty) const;

    void* getNativeHandle() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit RuntimeEffect(std::unique_ptr<Impl> impl);
};

class RuntimeShaderBuilder {
public:
    explicit RuntimeShaderBuilder(const std::shared_ptr<RuntimeEffect>& effect);
    ~RuntimeShaderBuilder();

    RuntimeShaderBuilder(const RuntimeShaderBuilder&) = delete;
    RuntimeShaderBuilder& operator=(const RuntimeShaderBuilder&) = delete;

    bool setUniform(std::string_view name, float value);
    bool setUniform(std::string_view name, float v0, float v1);
    bool setUniform(std::string_view name, const float* values, size_t count);

    std::shared_ptr<Shader> makeShader() const;
    std::shared_ptr<Shader> makeShader(float tx, float ty) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace enki

