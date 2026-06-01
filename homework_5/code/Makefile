CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -pthread

all: bank_sim producer_consumer_sim dining_sim

bank_sim: bank/bank.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

producer_consumer_sim: producer_consumer/producer_consumer.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

dining_sim: dining_philosophers/dining_philosophers.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f bank_sim producer_consumer_sim dining_sim

.PHONY: all clean
