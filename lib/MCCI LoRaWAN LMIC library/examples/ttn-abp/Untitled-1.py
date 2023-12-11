import paho.mqtt.client as mqtt
import json
import base64

# ChirpStack MQTT broker details
broker_address = "192.168.1.50"
port = 1883
username = "rung"
password = "1142"
topic = "application/0d7eca21-680c-472b-bfed-a9337c966a7a/#"


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(topic)
    else:
        print(f"Connection failed with code {rc}")


def on_message(client, userdata, msg):
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")

    # Parse JSON data from the received message payload
    try:
        data = json.loads(msg.payload.decode())
        print("------------------------------------------------------------")
        print(data)
        decode_data_string = data.get("object", {}).get("Data", "")
        print("------------------------------------------------------------")
        # Decode base64-encoded data
        try:
            decoded_data = base64.b64decode(decode_data_string).decode("utf-8")
            print(f"Decoded Data: {decoded_data}")
        except (base64.binascii.Error, UnicodeDecodeError) as e:
            print(f"Error decoding base64 data: {e}")

    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Error processing message: {e}")


client = mqtt.Client()
client.username_pw_set(username, password)
client.on_connect = on_connect
client.on_message = on_message

client.connect(broker_address, port, keepalive=60)

try:
    client.loop_forever()
except KeyboardInterrupt:
    print("Disconnecting from MQTT broker")
    client.disconnect()
