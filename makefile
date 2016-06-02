default: proj4

otpencd: otp_enc_d.o otp_enc_d.c
	gcc -Wall -std=c99  -o otp_enc_d otp_enc_d.c

keygen:  keygen.o keygen.c
	gcc -Wall -std=c99   -o keygen keygen.c

otpenc: otp_enc.o otp_enc.c
	gcc -Wall -std=c99  -o otp_enc otp_enc.c

proj4: otpenc keygen otpencd

clean:
	rm *.o

cleanall: clean
	rm *.o rm *.exe