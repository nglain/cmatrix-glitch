#!/bin/bash
# Matrix "glitch" mode - colorful waterfall effect
# Uses Japanese characters that may not render properly, creating abstract color patterns

# Force a locale that breaks katakana rendering for the glitch effect
LC_ALL=C cmatrix -c "$@"
