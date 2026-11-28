// Lint settings for the repository's JavaScript. CI runs `npx eslint .`.
import js from '@eslint/js';
import globals from 'globals';

export default [
  // Lint only these paths, so gitignored trees (.venv, fs_, .pio) are skipped.
  // The device web UI (src/network/html) is built into the firmware and is not
  // linted yet.
  {ignores: ['**/*', '!.github/', '!.github/scripts/', '!.github/scripts/**', '!test/', '!test/**', '!eslint.config.mjs']},
  js.configs.recommended,
  {
    files: ['**/*.js', '**/*.cjs'],
    languageOptions: {sourceType: 'commonjs', globals: globals.node},
  },
  {
    files: ['**/*.mjs'],
    languageOptions: {sourceType: 'module', globals: globals.node},
  },
];
