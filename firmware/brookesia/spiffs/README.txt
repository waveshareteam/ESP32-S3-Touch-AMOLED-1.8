Bundled media for the default factory firmware.

music/    MP3 tracks played by the MusicPlayer app from /spiffs/music.
xiaozhi/  OGG voice prompts (activation, success, digits) played by the
          Xiaozhi app from /spiffs/xiaozhi. xiaozhi/font.bin is generated
          at build time from the xiaozhi-fonts managed component and is
          not stored here.

PROVENANCE: these files were copied from the sibling Waveshare
ESP32-P4-WIFI6-Touch-LCD-4.3 Brookesia firmware (firmware/brookesia/spiffs),
which ships the same asset set. Redistribution terms were NOT verified when
they were copied: the earlier revision of this file deliberately left the tree
empty for that reason. Confirm the rights to the music recordings (and the TTS
prompt clips) before distributing a firmware image that contains them.
Removing a file here removes it from the built SPIFFS image.

Capacity: the storage partition is 6 MiB, and this tree plus the generated
font fills about 98% of the usable SPIFFS space. There is no room for
additional assets; drop a track or re-encode at a lower bitrate if more room
is needed.
