#include "enki/rendering/runtime_shader.hpp"

#include <include/core/SkData.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkShader.h>
#include <include/core/SkString.h>
#include <include/effects/SkRuntimeEffect.h>

#include <string>
#include <utility>

namespace enki {

namespace {

class SkiaRuntimeShaderWrapper : public Shader {
public:
    explicit SkiaRuntimeShaderWrapper(sk_sp<SkShader> shader) : shader_(std::move(shader)) {}
    void* getNativeHandle() const override { return shader_.get(); }

private:
    sk_sp<SkShader> shader_;
};

std::shared_ptr<Shader> wrapShader(sk_sp<SkShader> shader) {
    if (!shader) {
        return nullptr;
    }
    return std::make_shared<SkiaRuntimeShaderWrapper>(std::move(shader));
}

} // namespace

struct RuntimeEffect::Impl {
    sk_sp<SkRuntimeEffect> effect;
};

RuntimeEffect::RuntimeEffect(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
RuntimeEffect::~RuntimeEffect() = default;

std::shared_ptr<RuntimeEffect> RuntimeEffect::compile(std::string_view code, std::string* error_out) {
    SkString sk_code(code.data(), code.size());
    auto [eff, err] = SkRuntimeEffect::MakeForShader(sk_code);
    if (!eff) {
        if (error_out) {
            *error_out = err.c_str();
        }
        return nullptr;
    }
    auto impl = std::make_unique<Impl>();
    impl->effect = std::move(eff);
    return std::shared_ptr<RuntimeEffect>(new RuntimeEffect(std::move(impl)));
}

bool RuntimeEffect::hasUniform(std::string_view name) const {
    if (!impl_ || !impl_->effect) {
        return false;
    }
    std::string key(name);
    return impl_->effect->findUniform(key.c_str()) != nullptr;
}

std::shared_ptr<Shader> RuntimeEffect::makeShader(const void* uniform_data, size_t data_size) const {
    if (!impl_ || !impl_->effect) {
        return nullptr;
    }
    sk_sp<SkData> data = uniform_data && data_size > 0
        ? SkData::MakeWithCopy(uniform_data, data_size)
        : SkData::MakeEmpty();
    return wrapShader(impl_->effect->makeShader(std::move(data), nullptr, 0));
}

std::shared_ptr<Shader> RuntimeEffect::makeShader(const void* uniform_data, size_t data_size, float tx, float ty) const {
    if (!impl_ || !impl_->effect) {
        return nullptr;
    }
    sk_sp<SkData> data = uniform_data && data_size > 0
        ? SkData::MakeWithCopy(uniform_data, data_size)
        : SkData::MakeEmpty();
    sk_sp<SkShader> base = impl_->effect->makeShader(std::move(data), nullptr, 0);
    if (!base) {
        return nullptr;
    }
    if (tx != 0.0f || ty != 0.0f) {
        SkMatrix local_matrix = SkMatrix::Translate(tx, ty);
        base = base->makeWithLocalMatrix(local_matrix);
    }
    return wrapShader(std::move(base));
}

void* RuntimeEffect::getNativeHandle() const {
    return impl_ && impl_->effect ? impl_->effect.get() : nullptr;
}

struct RuntimeShaderBuilder::Impl {
    std::shared_ptr<RuntimeEffect> effect;
    std::unique_ptr<SkRuntimeShaderBuilder> builder;
};

RuntimeShaderBuilder::RuntimeShaderBuilder(const std::shared_ptr<RuntimeEffect>& effect)
    : impl_(std::make_unique<Impl>()) {
    impl_->effect = effect;
    if (effect && effect->getNativeHandle()) {
        auto* raw = static_cast<SkRuntimeEffect*>(effect->getNativeHandle());
        impl_->builder = std::make_unique<SkRuntimeShaderBuilder>(sk_ref_sp(raw));
    }
}

RuntimeShaderBuilder::~RuntimeShaderBuilder() = default;

bool RuntimeShaderBuilder::setUniform(std::string_view name, float value) {
    if (!impl_ || !impl_->builder || !impl_->effect || !impl_->effect->hasUniform(name)) {
        return false;
    }
    std::string key(name);
    impl_->builder->uniform(key.c_str()) = value;
    return true;
}

bool RuntimeShaderBuilder::setUniform(std::string_view name, float v0, float v1) {
    float values[2] = {v0, v1};
    return setUniform(name, values, 2);
}

bool RuntimeShaderBuilder::setUniform(std::string_view name, const float* values, size_t count) {
    if (!impl_ || !impl_->builder || !impl_->effect || !values || count == 0 || !impl_->effect->hasUniform(name)) {
        return false;
    }
    std::string key(name);
    impl_->builder->uniform(key.c_str()).set(values, static_cast<int>(count));
    return true;
}

std::shared_ptr<Shader> RuntimeShaderBuilder::makeShader() const {
    if (!impl_ || !impl_->builder) {
        return nullptr;
    }
    return wrapShader(impl_->builder->makeShader(nullptr));
}

std::shared_ptr<Shader> RuntimeShaderBuilder::makeShader(float tx, float ty) const {
    if (!impl_ || !impl_->builder) {
        return nullptr;
    }
    SkMatrix local_matrix = SkMatrix::Translate(tx, ty);
    return wrapShader(impl_->builder->makeShader(&local_matrix));
}

} // namespace enki

