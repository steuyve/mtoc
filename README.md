# Table of Contents
1. [About](#about)
    1. [Options](#options)

# About

Read in a markdown file (including this one) and generate its table of contents. Process multiple files in series.

## Options

Usage: `mtoc [lwd] filename ...`

- `-l` option specifies whether to make each item in the TOC into a link to its corresponding header.
- `-w` writes the generated TOC and contents of the original file to `tmp`. If unset, just write the generated TOC to `stdout`.
- `-d` followed by a number specifies the depth of the TOC. The default value is 6.

