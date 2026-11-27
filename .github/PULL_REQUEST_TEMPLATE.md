## Summary

* **Goal:**
* **Changes:**

## Verification

Checks run, with results. Name any check that could not run and why.

- [ ] `./bin/clang-format-fix` (new files added to git first)
- [ ] Host tests (`ctest`, plus the Python and Node tests in CI)
- [ ] Any other job in `.github/workflows/ci.yml`
- [ ] `xhigh` code review on the final head
- [ ] `pio check`
- [ ] `pio run -e <env>` for affected boards
- [ ] New or changed logic has host tests

## Device test plan

Steps for the owner to run on hardware, with expected results. Write "none" for
changes that cannot affect firmware.

## Risky areas

Does this change partitions, the bootloader, OTA or recovery code, the HAL, or
the SDK pin? If yes, list what changed and the update and rollback risk.
Otherwise write "none".

## Notes

Memory and flash impact, risks, and anything the reviewer should look at first.
