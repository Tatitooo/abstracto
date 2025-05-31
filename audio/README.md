# Audio Files for Abstracto Game

This directory contains the audio files used by the game:

## Music Files
- `musica_menu.wav` - Background music for the main menu
- `musica_juego.wav` - Background music during gameplay

## Sound Effects
- `carta.wav` - Sound played when a card is played by any player
- `victoria.wav` - Sound played when the human player wins
- `derrota.wav` - Sound played when the human player loses

## Supported Audio Formats

The audio system supports **WAV files** through SDL2's built-in audio support.

### For best compatibility:
- Use WAV files with 44.1 kHz sample rate, 16-bit, stereo format
- Files must be in WAV format with `.wav` extension
- The system will first try `.wav` extension, then fall back to the original filename

### If you have MP3 files:
1. **Convert your MP3 files to WAV format** using any audio converter (recommended)
2. **Name them with .wav extension**: `musica_menu.wav`, `musica_juego.wav`, etc.
3. **Replace the placeholder files** in this directory

## Installation Instructions

1. **Convert your audio files to WAV format**
2. **Name them exactly as:**
   - `musica_menu.wav`
   - `musica_juego.wav` 
   - `carta.wav`
   - `victoria.wav`
   - `derrota.wav`
3. **Place them in this `audio/` directory**
4. **Restart the game** - it will automatically load and play them

## Expected Console Output

When working correctly, you should see:
```
Trying WAV filename: audio/musica_menu.wav
Successfully loaded audio file: audio/musica_menu.wav (XXXXX bytes, X.X seconds)
```

## Audio Integration Points
- Menu music starts when `mostrarMenuPrincipal()` is called
- Game music starts when `jugarPartida()` begins
- Card sound plays after each card is played (both human and AI)
- Victory/defeat sounds play based on game outcome before showing results

## Troubleshooting
- **No sound playing**: Ensure files are in WAV format and not empty (0 bytes)
- **Console shows "Successfully loaded" but no sound**: Check system volume and that files have actual audio content
- **Loading errors**: Check file format and ensure files are not corrupted
- The system will continue to work without audio if files cannot be loaded