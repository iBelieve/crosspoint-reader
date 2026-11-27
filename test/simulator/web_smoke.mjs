#!/usr/bin/env node
// Boot the browser simulator in headless Chromium and read a book.
//
// Serves a site from test/simulator/web_site.py with the COOP/COEP headers
// threads need, presses real keys, and fails unless the page shows Home, the
// file browser, page 1 of the sample book, then page 2 after a page turn.
// Then it checks that the book resumes on page 2 after a reload (IndexedDB SD
// card), and that sleep followed by a key press wakes the firmware through a
// page reload.
//
//   node test/simulator/web_smoke.mjs sim-site --out sim-web-artifacts
//
// Needs the playwright package and its Chromium.

import { createServer } from 'node:http';
import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import { extname, join, resolve, sep } from 'node:path';
import { parseArgs } from 'node:util';
import { chromium } from 'playwright';

const PAGE_ONE_WORD = 'Alphasmoke';
const PAGE_TWO_WORD = 'Betasmoke';
const STEP_TIMEOUT_MS = 30000;
const TYPES = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm', '.json': 'application/json' };

const { values, positionals } = parseArgs({ allowPositionals: true, options: { out: { type: 'string', default: 'sim-web-artifacts' } } });
if (positionals.length !== 1) {
  console.error('usage: web_smoke.mjs <site folder> [--out folder]');
  process.exit(2);
}
const site = resolve(positionals[0]);
const out = resolve(values.out);
mkdirSync(out, { recursive: true });

const server = createServer((req, res) => {
  const path = decodeURIComponent(new URL(req.url, 'http://x').pathname);
  const file = join(site, path.endsWith('/') ? path + 'index.html' : path);
  if (!file.startsWith(site + sep) || !existsSync(file)) {
    res.writeHead(404).end();
    return;
  }
  res.writeHead(200, {
    'Content-Type': TYPES[extname(file)] || 'application/octet-stream',
    'Cross-Origin-Opener-Policy': 'same-origin',
    'Cross-Origin-Embedder-Policy': 'require-corp',
  });
  res.end(readFileSync(file));
});
await new Promise((r) => server.listen(0, '127.0.0.1', r));
const url = `http://127.0.0.1:${server.address().port}/?CROSSPOINT_SIM_SCREEN_TEXT=1`;

const logs = [];
const browser = await chromium.launch();
let failure = null;
try {
  const page = await browser.newPage({ viewport: { width: 900, height: 1000 } });
  page.on('console', (m) => logs.push(m.text()));
  page.on('pageerror', (e) => logs.push(`[pageerror] ${e.message}`));
  const sleep = (ms) => page.waitForTimeout(ms);

  // Waits for a log line matching re that arrived after index `from`.
  const waitLog = async (re, from = 0) => {
    const end = Date.now() + STEP_TIMEOUT_MS;
    while (Date.now() < end) {
      if (logs.slice(from).some((l) => re.test(l))) return;
      await sleep(100);
    }
    throw new Error(`timed out waiting for ${re}`);
  };
  const press = async (key) => {
    await page.keyboard.down(key);
    await sleep(100);
    await page.keyboard.up(key);
  };
  // A reloaded page must still own the saved SD card, or nothing it does is kept.
  const expectSaving = async () => {
    const notice = (await page.textContent('#notice')) || '';
    if (notice.trim()) throw new Error(`SD card not being saved: ${notice}`);
  };
  const shot = (name) => page.locator('canvas').screenshot({ path: join(out, `${name}.png`) });
  const step = async (key, re, name) => {
    const from = logs.length;
    await press(key);
    await waitLog(re, from);
    await sleep(300);
    await shot(name);
  };

  await page.goto(url);
  await waitLog(/SCREEN:.*Browse Files/);
  await sleep(300);
  await shot('1-home');
  await step('Enter', /SCREEN:.*books/, '2-files');
  await step('Enter', /SCREEN:.*sample/, '3-books');
  await step('Enter', new RegExp(`SCREEN:.*${PAGE_ONE_WORD}`), '4-page1');
  await step('ArrowDown', new RegExp(`SCREEN:.*${PAGE_TWO_WORD}`), '5-page2');

  // The SD card is saved every second; give it time, then reload.
  await sleep(2500);
  let from = logs.length;
  await page.reload();
  await waitLog(/SCREEN:.*Simulator Smoke/, from);
  await sleep(300);
  await shot('6-reloaded');
  await expectSaving();
  // Home selects the recent book: Confirm resumes it where it was left.
  await step('Enter', new RegExp(`SCREEN:.*${PAGE_TWO_WORD}`), '7-resumed');

  // S sleeps; any key wakes, which reloads the page as a power-button wake.
  from = logs.length;
  await press('s');
  await waitLog(/Entering deep sleep/, from);
  await sleep(300);
  await shot('8-sleep');
  from = logs.length;
  await press('Enter');
  await waitLog(/Starting CrossPoint/, from);
  await waitLog(/Entering activity: (?!Boot|Sleep)/, from);
  await sleep(500);
  await shot('9-woken');
  await expectSaving();
  if (page.url().includes('CROSSPOINT_SIM_WAKE_REASON')) {
    throw new Error(`wake reason left in the address bar: ${page.url()}`);
  }
} catch (e) {
  failure = e;
} finally {
  await browser.close();
  server.close();
  writeFileSync(join(out, 'console.log'), logs.join('\n') + '\n');
}

if (failure) {
  console.error(`browser simulator smoke test FAILED: ${failure.message}`);
  console.error('  last console lines:');
  for (const line of logs.slice(-40)) console.error(`    ${line}`);
  process.exit(1);
}
console.log(`browser simulator smoke test passed; artifacts in ${out}`);
