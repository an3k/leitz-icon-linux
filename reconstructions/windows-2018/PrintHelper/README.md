# Windows 2018 PrintHelper — DEVMODE patch reconstruction

Original source: user-supplied Windows ZIP, binary
\`LeitzIconLabelSetup/SourceDir/Leitz Icon Software/PrintHelper.dll\`
SHA-256: \`0b57ad5584fabe69610158347bace1647935e85d014d775753d0aa436734871a\`,
PE32 i386, image base \`0x10000000\`.

The original DLL exports \`SetPaper\` at RVA 0x1b30 and \`SetCustomPaper\`
at RVA 0x1930. Both open the Windows printer, load a DEVMODEW through
\`DocumentPropertiesW\`, update DEVMODEW and manufacturer-private bytes,
then write the changes through Windows \`DocumentPropertiesW\` and
\`SetPrinterW\` (level 9). These are **Windows printer configuration
operations**, not evidence of additional direct device wire commands.

Reconstructed C module \`devmode_patch.[ch]\` covers ONLY the defined
DEVMODEW byte modifications:
- offsets +0x44/+0x46 determine the full DEVMODE/private-data span;
- +0x48 gets \`dmFields=0x2402\`;
- +0x4e/+0x5a/+0x60 set paper size, print quality, Y resolution;
- OEM-private fields are located at the negative offsets -0x10,
  -0x0e, -0x0c and -0x0a from \`dmSize+dmDriverExtra\`;
- \`SetPaper\` checks the *existing* \`DM_PAPERSIZE\` bit before any
  changes, then sets private -0x10, fixed 1 at -0x0c, -0x0a;
- \`SetCustomPaper\` does not require that flag and sets private
  -0x10, -0x0e, -0x0a, leaving -0x0c unchanged.

The new API has explicit bounds validation and never calls Windows
spooler or hardware. It does not claim OEM x86 C++ ABI, all
exception paths, or Windows dynamic runtime parity. It is NOT
integrated into the native Linux CUPS driver, which already has
independent job-dependent custom-size handling.

Run \`sh test.sh\` for GCC/Clang fixed-vector checks. No OEM binary,
firmware, driver changes, or hardware commands are committed.
