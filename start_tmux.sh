#!/bin/sh

tmux new-session \; \
    split-window -v -p 80 \; \
    send-keys "minicom -w -D /dev/ttyUL1" C-m \; \
    split-window -h -p 67 \; \
    send-keys "minicom -w -D /dev/ttyUL2" C-m \; \
    split-window -h -p 50 \; \
    send-keys "minicom -w -D /dev/ttyUL3" C-m \; \
    select-pane -U \;
