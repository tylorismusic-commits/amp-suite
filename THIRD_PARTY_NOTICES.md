# Third-party notices

Amp Suite is GPL-3.0-or-later. The following components are either included or
planned for the DSP/cab stages. When a component is vendored, its original
copyright and license header is preserved and this file is updated.

## Included in the repository

| Component | License | Use |
|---|---|---|
| **CLAP** headers (`third_party/clap`) | MIT | Plugin API |
| **Jost** ExtraBold (`assets/fonts/Jost-ExtraBold.ttf`) | SIL Open Font License 1.1 | Panel typography; `assets/fonts/OFL.txt` |

## Included from M1

| Component | License | Use |
|---|---|---|
| **Guitarix** | GPL-2.0-or-later; some files GPL-3.0-or-later; whole work GPL-3.0-or-later | `third_party/guitarix`: tube tables, `valve.h`, `tubestageF/P`, tone-stack values, TS9/chorus/delay/Zita references |
| **CAPS** (Tim Goetze) | GPL-2.0-or-later | Tone-stack component values, via Guitarix; credit D. T. Yeh's tone-stack work |
| **HIIR** (`unevens/hiir`) | WTFPL-2 | 2×/4× halfband oversampling |
| **FFTConvolver** | MIT | Cab IR convolution |
| **chowdsp_wdf** | BSD-3-Clause | Optional WDF escape hatch |
| **NeuralAmpModelerCore** | MIT | Offline reference rendering only |
| **clap-validator** | MIT | CI validation |

## Reference captures (not included)

TONE3000 IR captures used as analysis targets are **T3K-licensed** and are not
included in the repository, tarball or binary. They are downloaded manually by
the developer/user into `refs/irs/` (gitignored) and used only to fit the
parametric cab model. The generated factory IRs are original CC0 works.

TONE3000 and the capture creators are not affiliated with Amp Suite and do not
endorse it. Real amp/cab names are trademarks of their respective owners and
appear only in documentation as descriptive references.
