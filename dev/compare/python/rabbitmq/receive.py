#!/usr/bin/env python
import pika

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host='localhost'))
channel = connection.channel()

channel.queue_declare(queue='hello_s', durable=True)

print ' [*] Waiting for messages. To exit press CTRL+C'

def callback(ch, method, properties, body):
    pass
    #print " [x] Received %r" % (body,)

channel.basic_consume(callback,
                      queue='hello_s')

channel.start_consuming()