import time
import threading
import inspect
import ctypes
import random
import pymysql
import paho.mqtt.client as mqtt

MQTT_SERVER_IP="59.47.233.179"
MQTT_SERVER_TOPIC="temp"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_SERVER_IP, 1883, 600) # 600为keepalive的时间间隔
client.subscribe(MQTT_SERVER_TOPIC, qos=0)
print("waitting for message:")
client.loop_forever() # 保持连接, qos=0)
