CC=arm-linux-gcc
client: adcTest.o
	$(CC) adcTest.o -s -o adcTest
client.o: adcTest.c
	$(CC) -c adcTest.c
clean:
	rm -rf *.o adcTest
