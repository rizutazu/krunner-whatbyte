# KRunner WhatByte

Simple byte utility (KRunner plugin)

## Usage

#### `print <value>`: construct content by given hexadecimal value array

Syntax of `<value>`:
  - `\x11\xab\xcD \x12`: `\x` + one or two hex digits, space is allowed between each escape character
  - `{12, 0x34, 0xEf}`: C-style array, decimal or hexadecimal value, an extra comma is allowed after last element

Action
  - Press enter to copy constructed text
  - But we cannot accurately copy arbitrary bytes as text if they are malformed Unicode, they will be replaced by garbage
  - So there is an action that copy them as file, you can paste it somewhere


#### `what input`: display byte representation of input

Example: 
  - Two matches will be given
  - Press enter to copy (has line break for each character, doesn't show in KRunner)

`what こんにちは` =>
  - `'こ' => {0xe3, 0x81, 0x93}, 
'ん' => {0xe3, 0x82, 0x93}, 
'に' => {0xe3, 0x81, 0xab}, 
'ち' => {0xe3, 0x81, 0xa1}, 
'は' => {0xe3, 0x81, 0xaf}`
  - `5 character(s), 15 byte(s)`

`what hi` => 
  - `'h' => {0x68}, 
'i' => {0x69}`
  - `2 character(s), 2 byte(s)`

## Install & Uninstall

Install:

1. make sure `cmake` / `KDE6` / `Qt6` developing environment is prepared.
2. Run `./install.sh`

Uninstall:

Run `./uninstall.sh`