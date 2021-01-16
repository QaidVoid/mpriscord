### ***Discord MPRIS Integration***
**Get Discord Rich Presence for media playing in your favorite media player.** \
*NOTE: As Discord Rich Presence has limitation of 1 update every 15 seconds, it's not possible to handle Playback rate. So, the time elapsed will be out of sync if playback rate isn't 1.*

```
git clone https://github.com/QaidVoid/mpriscord.git
make
./build/mpriscord
```

Media player priority can be changed in `include/mpris.h`.

Supported media players:
- [CMUS](https://cmus.github.io)
- [Plasma Browser Integration](https://community.kde.org/Plasma/Browser_Integration) for Firefox
- [VLC Media Player](https://www.videolan.org/vlc)
- Other media players implementing MPRIS interface (without their icon)