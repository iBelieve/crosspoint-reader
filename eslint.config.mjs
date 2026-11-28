// Lint settings for the repository's JavaScript. CI runs `npx eslint .`.
import js from '@eslint/js';
import globals from 'globals';

export default [
  // The device web UI (src/network/html) is built into the firmware and is not
  // linted yet.
  {ignores: ['node_modules/', 'build/', '.pio/', 'freeink-sdk/', 'src/']},
  js.configs.recommended,
  {
    files: ['**/*.js', '**/*.cjs'],
    languageOptions: {sourceType: 'commonjs', globals: globals.node},
  },
];
