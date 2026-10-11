# Leitz Icon — Independent verification suite

This branch contains **only verification code and fixtures**.
The product implementation lives separately on the `driver` branch.

## Contents

- `unit/`: low-level framing, job, probe and status checks.
- `integration/`: raster, media, CUPS and protocol availability checks.
- `mocks/`: synthetic CUPS raster interface.
- `Makefile`: standalone verification targets.
- `.github/workflows/ci.yml`: checks out the `driver` branch and this
  suite separately, staging them together **only in a temporary CI workspace**.

No production implementation files are tracked on this branch.
The `driver` branch does not read this suite, know its file layout, or
depend on its existence for building, printing, or packaging.

## Local verification

Check out the `driver` branch into a separate directory `product`
and this `tests` branch into `suite`. Copy only the suite's `integration/`, `unit/`, and `mocks/`
folders temporarily into `product/`, build the driver in `product/`,
and run the suite's separate Makefile:

```sh
mkdir -p product/tests
cp -a suite/integration suite/unit suite/mocks product/tests/
cd product
make clean all cups probe ppd
LEITZ_PROTOCOL_DIR="$PWD/driver/modules" make -f ../suite/Makefile verify
```

The temporary copy is not committed to the driver branch.

## Archived verification evidence

`reference/` retains the original, unchanged protocol research,
status investigations, and historical development records that were
available before splitting the two branches. These are reference materials
for checking device behavior, not production dependencies.
