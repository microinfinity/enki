# AudioWaveform

> A high-performance, real-time audio spectrum analyzer and microphone visualizer for ENKI, featuring dual modes (Wave Music for system audio output monitor and Wave Mic for live microphone input), powered by a dedicated `audio/` subsystem with native C++20 FFT DSP and GPU Skia rendering.

- **Header File**: `#include "enki/widgets/audio_waveform.hpp"`
- **C++ Class**: `enki::AudioWaveformWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::AudioWaveformProps`, `enki::AudioWaveform` (converts implicitly to `WidgetPtr`)
- **Render Object**: `enki::RenderAudioWaveform` (inherits from `enki::RenderBox`)
- **Audio Subsystem**: `audio/` (`<audio/audio_controller.hpp>`, `<audio/audio_analyzer.hpp>`, `<audio/audio_capture.hpp>`)
- **Factory Helper**: `enki::audioWaveform()`

---

## Architectural Overview

The audio engine and spectrum analyzer are cleanly isolated in the root `audio/` subsystem, keeping the core ENKI UI framework lightweight, modular, and 100% independent of any desktop environment.

```
┌─────────────────────────────────────────────────────────────┐
│                    audio/ Subsystem                         │
│  ┌─────────────────────────┐   ┌─────────────────────────┐  │
│  │   AudioCaptureEngine    │   │      AudioAnalyzer      │  │
│  │ (libpulse-simple / PW)  │──►│ (Pure C++20 Radix-2 FFT)│  │
│  │ - Sink Monitor (Music)  │   │ - Hann Windowing        │  │
│  │ - Source In (Mic)       │   │ - Gravity Peak Decay    │  │
│  └─────────────────────────┘   └─────────────────────────┘  │
└──────────────────────────────┬──────────────────────────────┘
                               │ Thread-safe snapshot
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                  AudioWaveform Widget                       │
│  • SingleChildRenderObjectWidget with Ticker (60+ FPS)       │
│  • Skia GPU Vector Drawing (Bars, Splines, Waves)            │
│  • Zero tree rebuilds during playback                        │
└─────────────────────────────────────────────────────────────┘
```

### 1. Wave Music (System Audio Output Visualizer)
- **Mode**: `AudioWaveformType::Music`
- **Signal Source**: Live desktop output monitor (`default_sink.monitor` via PulseAudio / PipeWire).
- **Behavior**: Reacts in real-time to any audio playing on the system (Spotify, YouTube in a browser, media players, system alerts, games).
- **DSP Analysis**: Real-time Fast Fourier Transform (FFT) dividing audio frequencies into logarithmic musical bands (Sub-Bass, Bass, Mid, High-Mid, Treble) with physics-based gravity peak decay.

### 2. Wave Mic (Live Microphone Waveform)
- **Mode**: `AudioWaveformType::Mic`
- **Signal Source**: System default microphone input (`default_source`).
- **Behavior**: Reacts to human speech and ambient sounds in real-time.
- **Visuals**: Symmetrical dancing voice bars centered around the baseline, continuous scrolling time-domain speech tapes, and high-frequency oscilloscopes.

---

## C++ Declarative Definition

```cpp
namespace enki {

enum class AudioWaveformType {
    Music,   ///< System Output Monitor (Spotify, YouTube, Desktop Sound)
    Mic,     ///< Microphone Input (Voice activity, speech recording)
};

enum class WaveformStyle {
    SegmentedLed,     ///< Studio rack segmented LED ladder with floor reflection
    RadialArc,        ///< 360-degree circular pulsing reactor with bass beat core
    LayeredAurora,    ///< Triple-layer translucent neon spline waves with ambient glow
    SymmetricWings,   ///< Center-outward mirrored cyber wings (butterfly spectrum)
    SpectrumBars,     ///< Vertical frequency equalizer bars with peak caps
    FluidWave,        ///< Smooth cubic bezier wave with glowing gradient fill
    StereoBands,      ///< Split Left & Right stereo spectrum bars
    VoiceBars,        ///< Symmetrical dancing voice bars around center line
    ScrollingLive,    ///< Time-domain tape scrolling from right to left
    LiveOscilloscope, ///< Real-time oscillating waveform
};

struct AudioWaveformProps {
    AudioWaveformType                       type            = AudioWaveformType::Music;
    WaveformStyle                           style           = WaveformStyle::SegmentedLed;

    std::shared_ptr<audio::AudioController> controller      = nullptr;
    std::vector<float>                      amplitudes      = {};

    size_t                                  bands           = 32;
    float                                   bar_width       = 5.0f;
    float                                   bar_gap         = 3.0f;
    BorderRadius                            bar_radius      = BorderRadius::circular(2.5f);
    Color                                   primary_color   = 0xFF00E5FF;
    std::optional<Color>                    secondary_color = 0xFFA855F7;
    std::optional<Color>                    accent_color    = 0xFFF43F5E;
    bool                                    show_peaks      = true;
    bool                                    show_reflection = true;
    float                                   min_bar_height  = 4.0f;

    float                                   sensitivity     = 1.0f;
    float                                   decay_rate      = 0.05f;
    bool                                    auto_start      = true;

    std::optional<StyleValue>               width           = std::nullopt;
    std::optional<StyleValue>               height          = std::nullopt;
    std::optional<StyleValue>               min_width       = std::nullopt;
    std::optional<StyleValue>               min_height      = std::nullopt;
    std::optional<StyleValue>               max_width       = std::nullopt;
    std::optional<StyleValue>               max_height      = std::nullopt;

    Clip                                    clip_behavior   = Clip::AntiAlias;
    BorderRadius                            clip_radius     = BorderRadius::circular(16.0f);
    Key                                     key             = Key::none();

    operator WidgetPtr() const;
};

struct AudioWaveform : public AudioWaveformProps {
    using AudioWaveformProps::AudioWaveformProps;
};

inline WidgetPtr audioWaveform(const AudioWaveformProps& props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Widget key identifier. |
| `type` | `AudioWaveformType` | `Music` | Operation mode (`Music` for system audio output, `Mic` for live microphone). |
| `style` | `WaveformStyle` | `SegmentedLed` | Visual theme (`SegmentedLed`, `RadialArc`, `LayeredAurora`, `SymmetricWings`, `SpectrumBars`, `FluidWave`, `StereoBands`, `VoiceBars`, `ScrollingLive`, `LiveOscilloscope`). |
| `controller` | `std::shared_ptr<audio::AudioController>` | `nullptr` | Optional audio controller managing stream lifecycle. |
| `amplitudes` | `std::vector<float>` | `{}` | Static or custom programmatic amplitude samples. |
| `bands` | `size_t` | `32` | Number of frequency bands / bars across the visualizer width. |
| `bar_width` | `float` | `5.0f` | Width of each individual spectrum bar in points. |
| `bar_gap` | `float` | `3.0f` | Spacing between adjacent spectrum bars. |
| `bar_radius` | `BorderRadius` | `2.5f` | Corner rounding radius of individual bars. |
| `primary_color` | `Color` | `0xFF00E5FF` | Base color (Cyan). |
| `secondary_color` | `std::optional<Color>` | `0xFFA855F7` | Mid-frequency / peak gradient accent color. |
| `accent_color` | `std::optional<Color>` | `0xFFF43F5E` | High-frequency neon accent color. |
| `show_peaks` | `bool` | `true` | Renders floating peak caps that drop with gravitational decay. |
| `show_reflection` | `bool` | `true` | Renders mirrored glass floor reflection under visualizer bars. |
| `min_bar_height` | `float` | `4.0f` | Minimum resting height of bars when silent. |
| `sensitivity` | `float` | `1.0f` | Gain multiplier amplifying audio input. |
| `decay_rate` | `float` | `0.05f` | Gravity falloff decay rate per frame. |
| `auto_start` | `bool` | `true` | Starts audio capture stream automatically when mounted. |
| `width`, `height` | `std::optional<StyleValue>` | `nullopt` | Explicit dimensional constraints. |
| `min_width`, `min_height` | `std::optional<StyleValue>` | `nullopt` | Minimum dimensional constraints. |
| `max_width`, `max_height` | `std::optional<StyleValue>` | `nullopt` | Maximum dimensional constraints. |
| `clip_behavior` | `Clip` | `Clip::AntiAlias` | Hardware clipping mode. |
| `clip_radius` | `BorderRadius` | `16.0f` | Outer container corner clip radius. |

---

## Usage Examples

### 1. Real-Time Spotify / YouTube Equalizer (Wave Music)

```cpp
#include "enki/widgets/audio_waveform.hpp"

using namespace enki;

WidgetPtr buildMusicEqualizer() {
    return audioWaveform({
        .type = AudioWaveformType::Music,
        .style = WaveformStyle::SpectrumBars,
        .bands = 48,
        .bar_width = 6.0f,
        .bar_gap = 2.5f,
        .primary_color = 0xFF06B6D4,     // Cyan base
        .secondary_color = 0xFFA855F7,   // Electric purple peak gradient
        .show_peaks = true,
        .width = 480.0f,
        .height = 160.0f,
    });
}
```

### 2. Siri / Assistant Voice Activity Bars (Wave Mic)

```cpp
WidgetPtr buildVoiceVisualizer() {
    return audioWaveform({
        .type = AudioWaveformType::Mic,
        .style = WaveformStyle::VoiceBars,
        .bands = 28,
        .bar_width = 6.0f,
        .bar_gap = 3.0f,
        .primary_color = 0xFF10B981,     // Emerald voice glow
        .sensitivity = 1.6f,
        .width = 360.0f,
        .height = 120.0f,
    });
}
```

### 3. Continuous Fluid Rhythm Wave (Music Spline)

```cpp
WidgetPtr buildFluidRhythm() {
    return audioWaveform({
        .type = AudioWaveformType::Music,
        .style = WaveformStyle::FluidWave,
        .bands = 32,
        .primary_color = 0xFFA855F7,
        .secondary_color = 0xFFEC4899,
        .width = StyleValue::percent(100.0f),
        .height = 140.0f,
    });
}
```

---

## See Also
- [**VideoPlayer**](./video_player.md) — Hardware-accelerated media and video playback.
- [**ParticleSystem**](../Animation%20&%20Motion/particle_system.md) — 2D particle simulation.
