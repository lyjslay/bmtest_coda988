dd if=/dev/urandom of=ctrl0B bs=1 count=0
dd if=/dev/urandom of=ctrl1B bs=1 count=1
dd if=/dev/urandom of=ctrl64B bs=1 count=64
dd if=/dev/urandom of=ctrl64_0B bs=1 count=255
dd if=/dev/urandom of=ctrl67B bs=1 count=67
dd if=/dev/urandom of=ctrl131B bs=1 count=131
dd if=/dev/urandom of=ctrlInStall bs=1 count=137
dd if=/dev/urandom of=ctrlOutStall bs=1 count=136
dd if=/dev/urandom of=ctrlStsStall bs=1 count=138

dd if=/dev/urandom of=bulk00 bs=1 count=0
dd if=/dev/urandom of=bulk01 bs=1 count=1
dd if=/dev/urandom of=bulk02 bs=1 count=2561
dd if=/dev/urandom of=bulk03 bs=1 count=3072
dd if=/dev/urandom of=bulk04 bs=1 count=3073
dd if=/dev/urandom of=bulk05 bs=1 count=65536
dd if=/dev/urandom of=bulk06 bs=1 count=66048
dd if=/dev/urandom of=bulk07 bs=1 count=66049
dd if=/dev/urandom of=bulk08 bs=1 count=8388608
dd if=/dev/urandom of=BulkInStall bs=1 count=48879
dd if=/dev/urandom of=BulkOutStall bs=1 count=57005

