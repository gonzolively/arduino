# Programming AVR Chips

To upload code to bare AVR chips (e.g. ATtiny85, 1634, 828, x313, x4, x41, x5, x61, x7, x8), you’ll need a way to flash them.

There are many methods, but one simple approach I recommend is [this Instructables guide](https://www.instructables.com/The-Idiots-Guide-to-Programming-AVRs-on-the-Chea/), which shows how to build a low-cost programming cradle and set up the Arduino IDE.

⚠️ **Note:** That guide references the old *arduino-tiny* core, which is no longer maintained. Use **[SpenceKonde/ATTinyCore](https://github.com/SpenceKonde/ATTinyCore)** instead — it supports a wide range of AVR chips and works with current Arduino IDE versions.

---

**Steps**
1. Build a programming cradle (see guide) or use another programmer.
2. Install Arduino IDE.
3. Add and use **ATTinyCore** via Boards Manager.
4. Select your chip/clock settings.
5. Upload your sketch.
