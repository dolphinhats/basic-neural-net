CC=g++ -pthread

main:
	$(CC) main.cpp

test:
	$(CC) test.cpp

readme:
	python createReadMe.py
