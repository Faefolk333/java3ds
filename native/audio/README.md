# Reusable MIDI / SF2 engine 0.2

`midi_engine.c` and `midi_engine.h` are independent of Java, the game and libctru.
They parse Standard MIDI Files with TinyMidiLoader and synthesize them using
TinySoundFont. The 3DS adapter streams PCM16 mono at 22,050 Hz through NDSP on a
separate thread. A lock serializes VM commands and rendering; the audio thread
never touches Java objects. The native bridge copies input bytes before allowing GC.

Source: [TinySoundFont / TinyMidiLoader](https://github.com/schellingb/TinySoundFont),
revision in `third-party/TinySoundFont/REVISION.txt`. The bank is a lossless PCM
subset of GeneralUser GS 2.0.3 by S. Christian Collins; its license and derivation
hashes are in `third-party/GeneralUser-GS/`. Only this game's instruments and the
standard drum kit are included. For another game's instruments, supply a suitable
GM SF2 or regenerate the selection; the C engine needs no changes.

## Minimal use

```c
MidiEngine *engine = midi_engine_open("bank.sf2", 22050);
int handle = midi_engine_load(engine, midi_bytes, midi_size);
midi_engine_control(engine, handle, MIDI_START, -1); // Repeat forever
midi_engine_render(engine, mono_pcm16_buffer, frame_count);
midi_engine_control(engine, handle, MIDI_STOP, 0);
midi_engine_control(engine, handle, MIDI_CLOSE, 0);
midi_engine_close(engine);
```

Check load/init return values. Compile the C file once and link the C math library
where needed. Keep `third-party/TinySoundFont` in the same relative location or
update the includes. The standalone ZIP retains that layout. Caller owns
synchronization. SEEK/POSITION use milliseconds; the J2ME bridge translates
microseconds. Seeking replays synthesis to preserve note phase and can take time
for long seeks. VOLUME accepts 0..100. States: 400 playing, 300 ready/stopped/end,
0 invalid/closed. START accepts a positive repeat count or -1. STOP preserves
position. CLOSE releases sequence storage.

The engine has 32 loaded slots, one active player (matching this game's own sound
arbitration), and at most 32 synth voices. It handles tempo, note on/off, programs,
controllers, sustain, pitch bend and percussion. PCM WAV supports mono/stereo
8/16-bit integer input resampled to mono. Unsupported formats return an error.
GM instrument timbres may differ from the original phone's bank. TinySoundFont
does not implement reverb/chorus.

## Shared bank and background layer

`midi_engine_clone(source)` creates another independent player sharing the decoded
SF2 samples and presets through TinySoundFont reference counting. Either player
can outlive the other. `midi_engine_current()` returns the currently playing slot,
while `MIDI_KIND` distinguishes MIDI (1) from PCM (2). Both instances require the
caller's synchronization.

The 3DS device now owns a foreground instance for original game sounds and a
background instance for the original theme. WAV effects mix with the theme;
original MIDI jingles temporarily replace it. Scene notifications carry actual
state, level and the saved SOUND setting. Pause gates rendering without replaying
past notes, avoiding a long seek under the VM lock when returning from a menu.
The sample bank is shared; only voices, channels and sequence state are separate.
The theme is an original game asset in the private bundle, not a free demo MIDI.

## 3DS integration and tests

`device_3ds.cpp` owns three 1,024-frame linear buffers and NDSP channel 0. Start it
before the VM; join the VM before shutting audio down; retain RomFS until shutdown
finishes. Missing DSP or font disables sound and logs the reason. NDSP needs the
console's own `sdmc:/3ds/dspfirm.cdc`; proprietary DSP firmware is not bundled.

`tests/audio_probe.c` renders six original MIDI sequences and five effects to
nonzero PCM, checking finite/infinite loops, stop, mute and close.
`tests/audio_device_probe.cpp` tests the real streaming adapter with explicit DSP
stubs, including MIDI PCM output, background/effects mixing, pause/resume, SOUND OFF,
init/allocation/thread/cache failure and cleanup.
`tests/GameAudioDirectProbe.java` starts all six MIDI sequences through the
original game sound methods in native miniJVM. `tests/NativeAudioProbe.java` exercises the native miniJVM bridge and MIDP Player.
The Java screenshot harness uses a duration clock, not an audio device.
Console playback and performance still need a physical New 3DS test.
