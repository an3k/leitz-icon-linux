# Outstanding work

- [ ] Add multilingual Leitz CUPS PPD/UI localization from macOS after exact
      source translations, locale coverage, and override behavior are verified.
      Do not copy vendor language claims without translating the added options.
- [ ] Obtain the original macOS raster filter implementation or reliable
      outputs before claiming that all 15 PrintQuality algorithms are
      byte-for-byte equivalent to the Apple/Leitz filters.
- [ ] Obtain the complete Windows driver package INF, StdNames.gpd reference,
      vendor LeitzUSRes.dll resources and related language resources to
      document exact UI strings and installation paths.
- [ ] Validate orientation and 88x55mm raster geometry on hardware; the
      Original PPD's Plus90 does not by itself fix A/B/G print rotation.
- [ ] Qualify imported options, quality modes and the explicit ESC A bypass
      on real hardware before promoting the development branch to main.
