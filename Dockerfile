FROM ubuntu:latest

RUN apt-get update && apt-get install -y g++

WORKDIR /app

COPY shell.cpp .

RUN g++ -Wall -O2 shell.cpp -o myshell

CMD ["./myshell"]