default: proj4
keygen:  keygen.o keygen.c
	gcc -Wall -std=c99   -o keygen keygen.c
otpencd: otp_enc_d.h otp_enc_d.o otp_enc_d.c
	gcc -Wall -std=c99  -o otp_enc_d otp_enc_d.c

otpenc: otp_enc.h otp_enc.o otp_enc.c
	gcc -Wall -std=c99  -o otp_enc otp_enc.c

otpdecd: otp_dec_d.h otp_enc_d.o otp_dec_d.c
	gcc -Wall -std=c99  -o otp_dec_d otp_dec_d.c

otpdec: otp_dec.h otp_dec.o otp_dec.c
	gcc -Wall -std=c99  -o otp_dec otp_dec.c


proj4: otpenc keygen otpencd otpdec otpdecd

clean:
	rm *.o

cleanall: clean
	 rm *.stackdump