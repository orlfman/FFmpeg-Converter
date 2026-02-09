# FFmpeg Converter

My own pet project. FFmpeg-Converter a simple Qt6 frontend for FFmpeg. Currently supports encoding with AV1, x265, and VP9. Slowly but surely adding more features and refinement.

### Feature List
- **Codec Tabs**: Dedicated supporet for AV1 (utilizing SVT-AV1), x265, and VP9 with codec specific options
- **General Video Tools**: Scale, auto detect cropping, rotate/flip, deinterlace, deblock, denoise, HDR to SDR tone mapping, sharpening, grain synthesis, and more
- **Audio Options**: Normalize loudness, speed change  with pitch preserve, and Opus, AAC, MP3, Vorbis, and FLAC support
- **Speed Controls**: Independent video/audio speed adjustment with proper resync
- **Combine Videos Tab**: Add multiple files and concat them into one
- **Trim Tab**: Built in player utilizing Qt6 multimedia for scrubbing, mark multiple in/out time stamps, reorder/remove segments, and output a single combined clip
- **Information Tab**: Detailed input/output file info (resolution, bitrate, codec, color depth, etc.)
- **Console Tab**: Verbos output and bonus able to run FFmpeg commands directly for quick testing
- **Presets**: One click quality presets for the AV1, x265, and VP9 tabs
- **Settings**: Custom FFmpeg path, custom SVT-AV1 library path (setting a custom path is broken atm), default output folder, notifications, and default codec tab

### Installation (Arch Linux / Qt6)
```bash
git clone https://github.com/orlfman/FFmpeg-Converter
cd FFmpeg-Converter
chmod +x build.sh
./build.sh
