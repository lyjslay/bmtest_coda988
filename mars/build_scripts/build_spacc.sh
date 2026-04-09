cat ./test/spacc/bl2.bin > bl2
xxd -i bl2 > ./test/spacc/bl2.h

openssl enc -aes-128-cbc -nosalt -in ./test/spacc/bl2.bin -out ./test/spacc/bl2_aes128_enc.bin -K 00000000000000000000000000000000 -iv 00000000000000000000000000000000
cat ./test/spacc/bl2_aes128_enc.bin > bl2_aes128_enc
xxd -i bl2_aes128_enc > ./test/spacc/bl2_aes128_enc.h

openssl enc -des-cbc -nosalt -in ./test/spacc/bl2.bin -out ./test/spacc/bl2_des_enc.bin -K 00000000000000000000000000000000 -iv 00000000000000000000000000000000
cat ./test/spacc/bl2_des_enc.bin > bl2_des_enc
xxd -i bl2_des_enc > ./test/spacc/bl2_des_enc.h

openssl enc -sm4-cbc -nosalt -in ./test/spacc/bl2.bin -out ./test/spacc/bl2_sm4_enc.bin -K 00000000000000000000000000000000 -iv 00000000000000000000000000000000
cat ./test/spacc/bl2_sm4_enc.bin > bl2_sm4_enc
xxd -i bl2_sm4_enc > ./test/spacc/bl2_sm4_enc.h


make clean; make TEST_CASE=spacc RUN_ENV=DDR BOARD=FPGA DEBUG=1
	
