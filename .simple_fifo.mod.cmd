savedcmd_simple_fifo.mod := printf '%s\n'   simple_fifo.o | awk '!x[$$0]++ { print("./"$$0) }' > simple_fifo.mod
