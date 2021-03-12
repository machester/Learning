# *---- encoding uft-8 ------------*
import os
import time
import threading
import inspect
import ctypes
import random
import pymysql
import json
import demjson
import paho.mqtt.client as mqtt

MQTT_SERVER_IP = "59.47.233.179"
MQTT_SERVER_TOPIC = "temp"


def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    print("waitting for message:")


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))
    # demjson.encode(msg)

client_id = f'python-mqtt-{random.randint(0, 100)}'
print("client_id: " + client_id)
client = mqtt.Client(client_id)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_SERVER_IP, 1883, 60)  # 60s keepalive
# qos=0
client.subscribe(MQTT_SERVER_TOPIC, qos=0)
# keep connect
client.loop_forever()
