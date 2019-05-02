#!/usr/bin/env python
import pika
import time

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host='localhost'))
channel = connection.channel()

channel.queue_declare(queue='hello_s', durable=True)

cycles = 10000
start = time.clock()
while cycles > 0:
    channel.basic_publish(exchange='',
                      routing_key='hello_s',
                      body='Hello World!',
                      properties=pika.BasicProperties(
                         delivery_mode = 2, # make message persistent
                      ))
    cycles = cycles - 1
now = time.clock()
print "Time required = " + str(now - start)
connection.close()