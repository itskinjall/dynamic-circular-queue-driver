savedcmd_vicharak_driver.mod := printf '%s\n'   vicharak_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > vicharak_driver.mod
