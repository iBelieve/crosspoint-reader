# Fork Scope

This is a personal fork of [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader).
Upstream aims to be a narrow, reading-only firmware for many devices. This fork
keeps that reading core and adds features that the owner wants on their own
device. Upstream's scope rules (no interactive apps, closed areas, the PR scope
checklist) do not apply here.

## 1. What the fork is for

A dedicated e-ink device for reading **and studying**: books, scripture, and
spaced-repetition review. Features earn their place by being something the
owner will use on the device regularly.

## 2. In scope

* **Everything upstream treats as core:** EPUB rendering, typography, e-ink
  refresh, reading UX, library management, and memory/flash/code quality.
* **Study and reading apps.** Planned examples:
  * **Flashcards:** Anki-style spaced repetition, with decks loaded from the
    SD card and review state saved there.
  * **Bible reader:** structured scripture text with book/chapter/verse
    navigation, references, and search.
* **Other interactive apps** when they suit an e-ink screen and the device's
  buttons: short, mostly static screens and little animation.
* **Sync or import for those apps** (for example deck or progress import) when
  the app needs it, following the network rules below.

## 3. Out of scope, or ask first

* **Ask first:** web browsers, RSS/news readers, audio or video playback, PDF
  rendering, and anything that keeps Wi-Fi or a task running in the
  background. These can drain the battery or exceed the hardware. Describe
  the cost and ask the owner before building one.
* **Out:** features that only make sense on a phone or a full-color screen,
  such as live animation, games that need fast refresh, or touch-heavy
  editing.

## 4. Constraints that still apply

The hardware is the same as upstream's, so the technical rules in
[`.agents/rules/`](.agents/rules/) still apply in full:

* **Memory:** the ESP32-C3 has about 380 KB usable RAM and no PSRAM. A new app
  must fit next to the reader core and must release its memory when it exits.
  See [hardware-resources.md](.agents/rules/hardware-resources.md).
* **Flash:** an optional feature must not grow builds, screens, or callers
  that do not use it. See
  [flash size](.agents/rules/hardware-resources.md#flash-size-unused-features-must-cost-nothing).
* **Power:** no background tasks or Wi-Fi that outlive the screen that needs
  them.
* **Data on the SD card:** decks, texts, and app state live on the SD card in
  documented, versioned formats, not in firmware. See
  [cache versioning](.agents/rules/generated-files-cache.md#cache-file-format-versioning).
* **Testable logic:** app logic (scheduling, parsing, indexing, storage) lives
  in `lib/` with host tests. The activity is a thin UI layer on top.

## 5. Staying close to upstream

The fork regularly merges upstream `develop`. To keep those merges easy:

* Put new apps in their own files and directories instead of changing shared
  upstream code where possible.
* Make the smallest change needed in shared code (for example a menu entry or
  a hook), and keep it separate from the app's own logic.
* A general fix or improvement to shared code that upstream would want belongs
  in its own small branch, so it can also be offered upstream. Upstream's
  [SCOPE.md](https://github.com/crosspoint-reader/crosspoint-reader/blob/develop/SCOPE.md)
  decides what it accepts.
